"""
wheel.py  Filter Wheel Movement Routines & Parameters   Version 1.20

filter wheel movement, geg stepper motor
slo-syn m061-fd-301 model, 200 steps/360deg
10:1 anti-backlash worm reduction ==> 2000 steps/360deg of fw
0-9999 10-turn fwp indicator ==> 5 fw steps/motor step
0.18 deg/motor step, 0.036 deg/fw step

IM483 stepper driver takes 4 steps at its input
to make the dial indicator move one count.
(this is coarsest resolution of the IM483.)
"""

__version__ = """$Id: wheel.py 399 2006-06-04 20:02:17Z drew $ """

__author__ = "$Author: drew $"

__URL__ = "$URL: http://astro.pas.rochester.edu/svn/pydsp/trunk/pydsp/wheel.py $"

import time

curdial = None # dial position, from 0 to 1000.. is floating point.
dialzero = 0 # 

_stepfact = 4.0 # steps per dial position 

class Wheel:
    # UNUSED class.
    # left here as suggestion.
    # filter wheel is singleton, so we implemented it as a module.
    # that's the easy lazy, and more procedural way.
    # HOWEVER, some object purists would rather see it as a class.
    # the client instantiates only one instance per wheel.
    # hey, some dewars have more than one wheel, it's not a bad idea.
    # 
    # the abstract wheel maintains a dial position / angle, using a motor
    # the dial is an external arbitrary indicator of position. 
    # the motion controller has its own units, either a stepper motor
    # or some sort of encoder,
    # and of course wheels have 360 degrees of rotation -- 
    # which we assume is a 'stable abstraction.'
    # all of these things (dial position, angle, motorsteps) are really
    # different expressions of the same physical thing.. where the wheel is.
    # the wheel class/module encapsulates the translations.
    # semantically, a filterwheel is multiple inheritance,
    # deriving from both Filter(coordinating names, wavelengths, etc, with angles)
    # and Wheel (just described above.)

    def __init__(self,value = 0.0) :
        self.value = value
    def setdegrees(self, degrees) :
        self.value = degrees * 1000.0 / 360.0
    def getsteps(self) :
        return self.value * _stepfact
    def steps_to(self,other) :
        pass # how many steps?
    
def move(motorsteps) :
    """ default motor move method.
    override with a real motor move method."""
    print("moving motor %d steps"%motorsteps)

def _dialstep(dialsteps) :
    """Move dialsteps, as reflected at the dial of the counter (0-999)
    updates curdial."""
    global curdial 
    if curdial == None :
        raise RuntimeError("Must initialize curdial (0-999)!") 
    # round request to nearest steps. 
    motorsteps = int(dialsteps*_stepfact)
    curdial += (motorsteps / _stepfact)
    while curdial >= 1000 :
        curdial -= 1000
    while curdial < 0 :
        curdial += 1000 
    move(motorsteps)
        
def backlash():
    _dialstep(-10)
    _dialstep(10)
 
def getdial():
    """ Return the current position of the dial """
    return curdial

def presetdial(dialpos):
    global curdial
    curdial = dialpos

def getdialsteps(dialpos):
    """
    Return the number of motor steps it will take
    to get to dialpos by shortest path.
    """
    dialpos = float(dialpos)
    if curdial == None :
        raise RuntimeError
    dialsteps = dialpos - curdial
    while (dialsteps > 500) :
        dialsteps -= 1000
    while (dialsteps < -500) :
        dialsteps += 1000
    return dialsteps

def setdial(dialpos):
    """Move the filter wheel to make the dial read dialpos. Use backlash"""
    dialsteps = getdialsteps(dialpos)
    _dialstep(dialsteps)
    if dialsteps < 20 :
        backlash()

def setdialnb(dialpos):
    """Move the filter wheel to make the dial read dialpos. No backlash"""
    dialsteps = getdialsteps(dialpos)
    _dialstep(dialsteps)
    if dialsteps < 20 :
        backlash()

"""
def move_fwd(abspos)  :
    # shortest path move to fwp = abspos, without backlash
    # abspos is always between 0 and 1000.
    print " moving filter wheel..."
    delta = abspos - curdial 
    while delta > 500 :
        delta -= 1000
    while delta < -500 :
        delta += 2000
    _dialstep(delta)

def move_fw(abspos) :
    # shortest path move with backlash
    move_fwd(abspos) 
    backlash()
"""
    
def angle(degrees = None) :
    "if degrees passed in, set filter wheel to that angle. Return position."
    if (degrees != None) :
        # move to some absolute degrees location
        dialpos = (degrees * 1000.0 / 360.0) + dialzero
        setdial(dialpos)
        print("move to %d, %d degrees"%(dialpos,degrees))
    # return current angle
    if curdial == None :
        raise RuntimeError("Must initialize curdial (0-999)!")
    curangle = (curdial - dialzero) * 360.0 / 1000.0
    if curangle < 0 : curangle += 360.0
    return curangle
