"""
  tempcontrol.py

  Control temperature by varying power to a heater resistor.
"""

__version__ = """$Id: tempcontrol.py 314 2004-12-07 15:25:51Z dsp $ """

__author__ = "$Author: dsp $"

__URL__ = "$URL: http://astro.pas.rochester.edu/svn/pydsp/trunk/pydsp/tempcontrol.py $"

import time
import sys

class tcontroller :
    """Generic temperature controller class.

    Ideally, the thermal modelling is accurate, and
    little extra is needed to close the loop.
    """
    def __init__(self, temp_reader = None, heat_writer = None, 
            K_per_min = 0.1, decay_minutes = 55, equilib_K = 29.6, 
            equilib_V = 3.0, gain = 10, wfile=sys.stdout):
        """create a new temp controller object. """

        if temp_reader :
            self.readTemp = temp_reader
            self.goal = self.carrot = self.current_temp = self.readTemp() 
        else :
            self.goal = self.carrot = self.current_temp = 30.0
        if heat_writer :
            self.setHeater = heat_writer
        # latest reading from system.
        self.degrees_K_per_minute = K_per_min # controller ramp rate, positive
        self.negative_rate_factor = 0.8 # to cool down slower than warmup.
        self.decay_minutes = decay_minutes# thermal time constant. (constant?)
        # the next 2 lines are probably better
        # expressed as a monotonic function.
        # given a certain heater voltage,
        # what T does the dewar settle out to?
        self.equilibrium = equilib_K # reference temp.
        self.equilib_V = equilib_V # reference voltage at ref temp..
        # got 28.2 K at a heater of 2800
        self.deg_K_per_watt = 1000.0
        self.gain = gain # how much faster we want the system to react.
        self.time = time.time() # time at last carrot.
        self.time_limit_minutes = 10.0 #
        self.error = 0.0 #
        self.errorpower = 0.0
        self.sleepfunc = time.sleep
        self.wfile = wfile
        self.max_volts = 8.0
    def watts(self, error = None) :
        "Given all else, what power should we put out?"
        if not error : error = self.carrot - self.current_temp
        self.error = error
        power = self.dc_watts() # open loop
        power += self.ac_watts() # ramping factor
        if power > 0 :
            self.errorpower = 0.9 *self.errorpower 
            self.errorpower += 0.1 * self.watts_per_K() * self.gain * self.error # KP * error
            if power+self.errorpower < 0 :
                self.errorpower = -power 
            power += self.errorpower 
        return power
    def ac_watts(self) : # VFF watts, for ramping
        # need to know the slope of the watts / K curve here.
        # also need to know the thermal time constant.
        # T(t) = e^-(eq_err/k)
        # dT/dt = 1/k * e^-(eq_err/k)
        # given carrot ramp velocity (degrees K per minute)
        # what extra power (in milliwatts) will slew
        # the (first order) system at that ramp rate?
        return self.watts_per_K() * self.K_per_min() * self.decay_minutes 
        
    def K_per_min(self) :
        " given the carrot, goal, and ramp rate, yield desired thermal velocity."
        if self.carrot == self.goal :
            return 0.0 # not changing? no VFF
        if self.carrot < self.goal :
            # we are heating up.
            return self.degrees_K_per_minute
        else :
            return -self.degrees_K_per_minute * self.negative_rate_factor
    def r_power(self, volts):
        """Return resistor power for a given voltage"""
        return volts**2/self.load_r()

    def dc_watts(self,temp=None) : # TFF watts, for open loop.
        "watts needed to sustain temp (defaults to current carrot)"
        if temp == None :
            temp = self.carrot 
        # the open loop dc power we need is the power at our known stable point
        dc_power = (self.equilib_V**2)/self.load_r()
        # plus (or minus) the power off of that known stable point, from the
        # slope of the curve
        dc_power += self.watts_per_K(temp) * (temp - self.equilibrium)
        return dc_power
    def watts_per_K(self,temp=None) :
        """Return slope of watts vs temp curve at the specified temp.
           thermal conductivity varies wildly with temperature..
           also, the array may self-heat differently at different temps."""
        #return 0.00111  # approximate over 28-46K.
        return 1/self.deg_K_per_watt
    def set_dc_calibration(self, reading1, reading2):
        """
        using two known control points, 
        where each reading is a tuple (voltage, temp),
        set the slope and equilibrium points.
        """
        v1, t1 = reading1
        v2, t2 = reading2
        self.deg_K_per_watt = ((t2 - t1) * self.load_r())/(v2**2 - v1**2)
        self.equilibrium = (t1 + t2) / 2.0
        self.equilib_V = ((v1**2 + v2**2)/2.0)**0.5
        
    def load_r(self) :
        """Return the load resistance. 
        Why a function? R may vary with temp. (property might be good here)
        the current monitor could actually help here, and measure the
        current, and we could compute the proper voltage to deliver 
        that power.
        """
        return 1000.0 # our curves assume 1000 ohm R.
    def volts(self, watts=None) :
        "power is V squared over R. V = sqrt(power * resistance)"
        if watts == None:
            watts = self.dc_watts()
        if watts <= 0.0 :
            return 0.0  # can't cool a resistor with imaginary current. 
        else :
            import math
            volts = math.sqrt(watts * self.load_r())
            if volts > self.max_volts:
                volts = self.max_volts
            return volts
    def readTemp(self):
        "return the current temperature in degrees Kelvin. MUST OVERRIDE!"
        return self.current_temp #
    def setHeater(self, voltage) :
        "set the heater voltage in volts. MUST OVERRIDE!"
        print("setting heater to %6.3f volts"% voltage, file=self.wfile)
    def adjustCarrot(self) :
        """Move the carrot. carrot == desired instantaneous temperature.
            (picture a horse with a carrot hanging in front of it)"""
        now = time.time() # time is in seconds.
        minutes_elapsed = (now - self.time) / 60 
        # skip it if not enough time has passed (near zero)
        if minutes_elapsed < 0.1  :
            return
        self.time = now
        if minutes_elapsed > self.time_limit_minutes :
            return
        # move the carrot towards the goal (but don't pass it)
        K_per_minute = self.K_per_min()
        if not K_per_minute : return
        newcarrot = self.carrot + K_per_minute * minutes_elapsed
        # did we overshoot?
        if ( newcarrot > self.goal > self.carrot ) or ( newcarrot < self.goal < self.carrot )  :
            self.carrot = self.goal
        else:
            self.carrot = newcarrot
    def do(self) :
        current_temp = self.readTemp() # read the current temp.
        if not current_temp : # XXX how would this happen?
            print("can't read temperature! aborting!", file=self.wfile)
            return
        self.current_temp = current_temp
        self.adjustCarrot() # move our instantaneous temp.
        watts = self.watts()
        heater_voltage = self.volts(watts)
        self.setHeater(heater_voltage)
    def goto(self, newgoal) :
        current_temp = self.readTemp() # read the current temp.
        if not current_temp :
            print("can't read temperature! aborting!", file=self.wfile)
            return
        self.current_temp = current_temp
        if not (current_temp - 0.2 < self.carrot < current_temp + 0.2) :
            self.carrot = current_temp ;
        self.goal = newgoal
        self.time = time.time() 
        # move the carrot until carrot is there
        while ( self.carrot != self.goal) :
            self.do()
            print("err %5.3f temp %5.3f"% (self.error, self.current_temp), file=self.wfile)
            time.sleep(10)
        # stabilize for 10 minutes afterwards.
        for i in range(60) :
            self.do()
            print("err %5.3f temp %5.3f"% (self.error, self.current_temp), file=self.wfile)
            time.sleep(10)
