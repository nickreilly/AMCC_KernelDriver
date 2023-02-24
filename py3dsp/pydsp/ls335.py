"""
ls335.py..  allows access to the lakeshore '335' temperature sensor / controller.

This is used at UR.
"""

__version__ = """$Id: ls335.py 406 2006-09-22 16:36:54Z drew $ """

__author__ = "$Author: drew $"

__URL__ = "$URL: http://astro.pas.rochester.edu/svn/pydsp/trunk/pydsp/ls335.py $"

"""
Be sure to set the proper rw permissions on the serial port.
chmod o+rw /dev/ttyUSB0
or modify the /etc/udev/permissions.d/50-udev.permissions file that is 
used to create the devices at boot time.  Set ttyUSB*:root:uucp:0666
"""

try :
    import serial
    import csv as CSV
    import time
except:
    print("no serial module!")
    serial = None

lakeshore335 = None
useTempControl = "hardwareTempControl"

# open the temperature sensor
def openTempSensor():
    global lakeshore335 
    if serial :
      try:
        lakeshore335 = serial.Serial('/dev/ttyUSB0', baudrate=57600,
            bytesize=serial.SEVENBITS,
            parity=serial.PARITY_ODD,
            stopbits=serial.STOPBITS_ONE,
            timeout=3) #open read/write
      except :
        print("access violation")
    if (lakeshore335):
        print("LakeShore335 Temperature Controller opened")
        print(lakeshore335.portstr)
    else:
        print("Problem opening Lakeshore335 Temperature Controller. \n Be sure to set the proper permissions on the serial port. \n chmod a+rw /dev/ttyUSB0")
    
# close the temperature sensor
def closeTempSensor():
    if (lakeshore335):
        lakeshore335.close()
    else:
        print('Device not open')

# get the actual temperature in Kelvin
def readTemp():
   # Read the temperature from probe "A"
   if lakeshore335:
     lakeshore335.write("KRDG? A \r \n")
     # need to drop the "+" for float -- we know it is + since Kelvin=absolute
     currentTemp = float(lakeshore335.readline()[1:])
     # if not using lakeshore335, simplest way to bypass is just set temp like:
     #currentTemp = 0.0  # comment out this when you want to use the real one above!
     return currentTemp
   else:
     print("LakeShore335 not opened?")
     return 0.0

def readTempB():
   # Read the temperature from probe "B"
   if lakeshore335:
     lakeshore335.write("KRDG? B \r \n")
     # need to drop the "+" for float -- we know it is + since Kelvin=absolute
     currentTemp = float(lakeshore335.readline()[1:])
     return currentTemp
   else:
     print("LakeShore335 not opened?")
     return 0.0
     
# Change the Set Point Temperature of control loop 1
def setSetpointTemp(temp):
   if lakeshore335:
     lakeshore335.write("SETP 1," + repr(temp) + " \r \n")

# Get the set point temperature
def readSetpointTemp():
   if lakeshore335:
     lakeshore335.write("SETP? 1 \r \n")
     return lakeshore335.readline()

# Set the temperature ramp rate (units: K/min) of control loop 1
def setRamp(rate):
   if lakeshore335:
     lakeshore335.write("RAMP 1,1," + repr(rate) + " \r \n")

# Get the temperature ramp rate of control loop 1
def readRamp():
   if lakeshore335:
     lakeshore335.write("RAMP? 1 \r \n")
     return lakeshore335.readline()

# Turn on the heater to a given power level. 0=off, 1=low, 2=med, 3=high
def heaterRange(powlev):
   if lakeshore335:
     lakeshore335.write("RANGE " + repr(powlev) + " \r \n")

# Input a calibration curve
def curveInput():
    f = open('tmptr_curve25.txt', 'r') # File is in Comma Separated Value (CSV) format
    curvenum=25
    # Write the serial number of this diode to the curve header (see manual for syntax)
    lakeshore335.write("CRVHDR 25,DT-470,D18755,2,325.0,1 \r \n")
    # Now check that it is correct
    lakeshore335.write("CRVHDR? 25 \r \n")
    query_hdr = lakeshore335.readline()
    print(query_hdr)
    try:
        filesreader = CSV.DictReader(f,skipinitialspace=True)
        for each in filesreader:
            if lakeshore335:
                lakeshore335.write("CRVPT " + repr(curvenum) + "," + repr(int(each['index'])) + "," + repr(float(each['voltage'])) + "," + repr(float(each['temp'])) + " \r \n")
                # The above write command works fine, however, the print statement 
                # is missing the first C on screen output.  Not sure why.
                print("CRVPT " + repr(curvenum) + "," + repr(int(each['index'])) + "," + repr(float(each['voltage'])) + "," + repr(float(each['temp'])) + " \r \n") 
                time.sleep(2) # Needed because LS-335 can't handle high data rate
    finally:
        f.close()


