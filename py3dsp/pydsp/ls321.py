"""
ls321.py..  allows access to the lakeshore '321' temperature sensor / controller.

This was used at RIT for Zoran's setup.
"""

__version__ = """$Id: ls321.py 399 2006-06-04 20:02:17Z drew $ """

__author__ = "$Author: drew $"

__URL__ = "$URL: http://astro.pas.rochester.edu/svn/pydsp/trunk/pydsp/ls321.py $"

try :
    import serial
except:
    print("no serial module!")
    serial = None

lakeshore321 = None

#open the temperature sensor
def openTempSensor():
    global lakeshore321 
    if serial :
      try:
        lakeshore321 = serial.Serial('/dev/ttyS1', baudrate=1200,
            bytesize=serial.SEVENBITS,
            parity=serial.PARITY_ODD,
            stopbits=serial.STOPBITS_ONE,
            timeout=10) #open read/write
      except :
        print("access violation")
    if (lakeshore321):
        print("Lakeshore321 sensor opened")
        print(lakeshore321.portstr)
    else:
        print("Problem opening Lakeshore321 Sensor")
    
#close the temperature sensor
def closeTempSensor():
    if (lakeshore321):
        lakeshore321.close()
    else:
        print('Device not open')

#set the temperature (set point)
def setTemp(temp):
   if lakeshore321:
      lakeshore321.write("SETP " + repr(temp))

#get the set point
def readSetpoint():
   if lakeshore321:
    lakeshore321.write("SETP?")
    return lakeshore321.readline()

#get the actual temperature
def readTemp():
   if lakeshore321:
    lakeshore321.write("CDAT?")
    return lakeshore321.readline()
   else :
    return "xxxxx\n\r"

#set the units in Kelvin or Celsius
def setUnits(unit):
   if lakeshore321:
    lakeshore321.write("CUNI " + unit)

#get the units of measure
def readUnits():
   if lakeshore321:
    lakeshore321.write("CUNI?")
    return lakeshore321.readline()
   else:
    return "xxxxx\n\r"

#set the rate
def setRate(rate): #ramp rate
   if lakeshore321:
    lakeshore321.write("RATE " + repr(rate))

#get the rate
def readRate(): #ramp rate
   if lakeshore321:
    lakeshore321.write("RATE?")
    return lakeshore321.readline()

"""
# alternate approach that does not use the serial module..

import termios
import time

atts = [0, 0, 2985, 0, 9, 9, ['\x00', '\x00', '\x00', '\x00', '\x00', 0, 0, '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00']]


lso = None
lsi = None
def openls(name = "/dev/ttyS1") : 
    global lso, lsi
    lso = open(name,"w")
    lsi = open(name,"r")
    termios.tcsetattr(lso, termios.TCSANOW, atts)
    termios.tcsetattr(lsi, termios.TCSANOW, atts)

def rdtemp() :
    lso.write("SETP?")
    lso.flush()
    time.sleep(1)
    return lsi.readline()
"""
