"""
ls332.py..  allows access to the lakeshore '332' temperature sensor / controller.

This is used at UR.
"""

__version__ = """$Id: ls332.py 406 2006-09-22 16:36:54Z drew $ """

__author__ = "$Author: drew $"

__URL__ = "$URL: http://astro.pas.rochester.edu/svn/pydsp/trunk/pydsp/ls332.py $"

"""
Be sure to set the proper rw permissions on the serial port.
chmod o+rw /dev/ttyS0
or modify the /etc/udev/permissions.d/50-udev.permissions file that is 
used to create the devices at boot time.  Set ttyS*:root:uucp:0666

added a file at /etc/udev/rules.d/50-serial-perm.conf with the line:
SUBSYSTEM=="tty", MODE="0666"
"""



try :
    import serial
    import csv as CSV
    import time
    import xdir
except:
    print("no serial module!")
    serial = None

lakeshore332 = None
useTempControl = "hardwareTempControl"
from util_funcs import info_from_string

def find_serial_port():
  for i in range(20):
    temp_loc = f'/dev/ttyS{i}'
    try:
      lakeshore332 = serial.Serial(temp_loc, baudrate=9600,
              bytesize=serial.SEVENBITS,
              parity=serial.PARITY_ODD,
              stopbits=serial.STOPBITS_ONE,
              timeout=3) #open read/write
      # print(f'Got a hit on {i}!')
      # try:
      lakeshore332.write(("*IDN?  \r \n").encode('utf-8'))
      result_string = lakeshore332.readline().decode('utf-8')
      if len(result_string) > 0:
        
        if result_string.split(',')[0] == 'LSCI':
          print(f'{temp_loc} gives the string: {result_string}- using it as your temp controller!')
          return temp_loc
      # current_temp = float(info_from_string(lakeshore332.readline().decode('utf-8'), 'ls332_output'))
      
      # if current_temp is None:
      #   print(f'{i} isnt it tho...')
      return  temp_loc
      # except TypeError:
      lakeshore332.close()
      # time.sleep(2)

    except:
      continue

try:
  with open(f'{xdir.dsphome}/pydsp/last_temp_controller_port.txt', mode='r') as temp_file:
    lakeshore_loc = temp_file.readlines()[0]
except FileNotFoundError:
  lakeshore_loc = find_serial_port()
  print(f'Saving location to file! {lakeshore_loc}')
  with open(f'{xdir.dsphome}/pydsp/last_temp_controller_port.txt', mode='x') as temp_file:
    temp_file.write(lakeshore_loc)

# open the temperature sensor
def openTempSensor():
    global lakeshore332 
    # lakeshore_loc = ''
    if serial :
      try:
        lakeshore332 = serial.Serial(lakeshore_loc, baudrate=9600,
            bytesize=serial.SEVENBITS,
            parity=serial.PARITY_ODD,
            stopbits=serial.STOPBITS_ONE,
            timeout=3) #open read/write
      except :
        print("access violation")
    if (lakeshore332):
        print("LakeShore332 Temperature Controller opened")
        print(lakeshore332.portstr)
    else:
        print(f"Problem opening Lakeshore332 Temperature Controller. \n Be sure to set the proper permissions on the serial port. \n chmod a+rw f'{lakeshore_loc}")
    
# close the temperature sensor
def closeTempSensor():
    if (lakeshore332):
        lakeshore332.close()
    else:
        print('Device not open')

# get the actual temperature in Kelvin
def readTemp():
   if lakeshore332:
     lakeshore332.write(("KRDG? A \r \n").encode('utf-8'))
     # need to drop the "+" for float -- we know it is + since Kelvin=absolute
     currentTemp = info_from_string(lakeshore332.readline().decode('utf-8'), 'ls332_output')
     
     if len(currentTemp) == 0 or currentTemp is None:
        time.sleep(0.1)
        currentTemp = info_from_string(lakeshore332.readline().decode('utf-8'), 'ls332_output')
     # if not using lakeshore332, simplest way to bypass is just set temp like:
     #currentTemp = 0.0
     return float(currentTemp)
   else:
     print("LakeShore332 not opened?")
     return 0.0

# get the actual temperature in Kelvin
def readTempB():
   if lakeshore332:
     lakeshore332.write(("KRDG? B \r \n").encode('utf-8'))
     # need to drop the "+" for float -- we know it is + since Kelvin=absolute
     currentTemp = info_from_string(lakeshore332.readline().decode('utf-8'), 'ls332_output')
     # if not using lakeshore332, simplest way to bypass is just set temp like:
     #currentTemp = 0.0
     return float(currentTemp)
   else:
    #  print("LakeShore332 not opened?")
     return 0.0
     
# Change the Set Point Temperature of control loop 1
def setSetpointTemp(temp):
   if lakeshore332:
     lakeshore332.write(("SETP 1," + repr(temp) + " \r \n").encode('utf-8'))

# Get the set point temperature
def readSetpointTemp():
   if lakeshore332:
     lakeshore332.write(("SETP? 1 \r \n").encode('utf-8'))
     return float(info_from_string(lakeshore332.readline().decode('utf-8'), 'ls332_output'))

# Set the temperature ramp rate (units: K/min) of control loop 1
def setRamp(rate):
   if lakeshore332:
     lakeshore332.write(("RAMP 1,1," + repr(rate) + " \r \n").encode('utf-8'))

# Get the temperature ramp rate of control loop 1
def readRamp():
   if lakeshore332:
     lakeshore332.write(("RAMP? 1 \r \n").encode('utf-8'))
     return lakeshore332.readline()

# Turn on the heater to a given power level. 0=off, 1=low, 2=med, 3=high
def heaterRange(powlev):
   if lakeshore332:
     lakeshore332.write(("RANGE " + repr(powlev) + " \r \n").encode('utf-8'))

# Input a calibration curve
def curveInput():
    f = open('tmptr_curve22.txt', 'r') # File is in Comma Separated Value (CSV) format
    curvenum=22
    # Write the serial number of this diode to the curve header (see manual for syntax)
    lakeshore332.write(("CRVHDR 22,DT-470,D18755,2,325.0,1 \r \n").encode('utf-8'))
    # Now check that it is correct
    lakeshore332.write(("CRVHDR? 22 \r \n").encode('utf-8'))
    query_hdr = lakeshore332.readline()
    print(query_hdr)
    try:
        filesreader = CSV.DictReader(f,skipinitialspace=True)
        for each in filesreader:
            if lakeshore332:
                lakeshore332.write(("CRVPT " + repr(curvenum) + "," + repr(int(each['index'])) + "," + repr(float(each['voltage'])) + "," + repr(float(each['temp'])) + " \r \n").encode('utf-8'))
                # The above write command works fine, however, the print statement 
                # is missing the first C on screen output.  Not sure why.
                print("CRVPT " + repr(curvenum) + "," + repr(int(each['index'])) + "," + repr(float(each['voltage'])) + "," + repr(float(each['temp'])) + " \r \n") 
                time.sleep(2) # Needed because LS-332 can't handle high data rate
    finally:
        f.close()

"""
Alternate approach that does not use the serial module.

This almost works.  Writing to the LakeShore is successful, i.e.
you can change the SetPoint temperature, turn heater power on, etc.
However, reading the LakeShore doesn't work: output from the LS is
garbled e.g. when reading the temperature you get \x8a\xab2\xb94\xae\xa31
instead of +294.31 (notice that the numbers are there, but you get extra
ASCII hex codes -- if you print that string with the hex codes, it
does NOT give you the right number, since the hex code needs to \xFF,
then you get something printed as 241 instead of +294.31).  
If you set all the serial port attributes with the python serial module 
(see above), then all of this code works properly.  But after reboot of 
the computer, the serial port is reset and this code will fail to read
properly until the attributes are again set via the serial module. 
Obviously, this code doesn't set the correct attributes for the serial
port, but finding those attributes and how to set them would take much 
more time than it would to simply download the pyserial module and use 
it.  Who wants to reinvent the wheel?

import time
import os

LSreadwrite = None
lakeshore332 = None


#open the temperature sensor
def openLakeShore(name = "/dev/ttyS0"):
    global LSreadwrite 
    if os :
      try:
        # lsw = os.open(name,os.O_WRONLY+os.O_NONBLOCK)
        # lsr = os.open(name,os.O_RDONLY+os.O_NONBLOCK)
        LSreadwrite = os.open(name,os.O_RDWR+os.O_NONBLOCK)
        # LSreadwrite = os.open("/dev/ttyS0",os.O_RDWR+os.O_NONBLOCK)
        print "Lakeshore332 sensor opened"
      except :
        print "Access violation."
        print "Problem opening Lakeshore321 Sensor"
    
#close the temperature sensor
def closeTempSensor(name = "/dev/ttyS0"):
    if (LSreadwrite):
        os.close(name)
    else:
        print 'Device not open'

#get the actual temperature in Kelvin
def readTemp():
    os.write(LSreadwrite,"KRDG? A \r \n")
    # os.write(LSreadwrite,"KRDG? A " + chr(13) + chr(10))
    # os.flush()
    time.sleep(1)
    return os.read(LSreadwrite,80)

#set the temperature (set point) of control loop 1
def setTemp(temp):
   if LSreadwrite:
     os.write(LSreadwrite,"SETP 1," + repr(temp) + " \r \n")

#get the set point
def readSetpoint():
   if LSreadwrite:
     os.write(LSreadwrite,"SETP? 1 \r \n")
     time.sleep(1)
     return os.read(LSreadwrite,80)

#set the rate of control loop 1
def setRate(rate): #ramp rate
   if LSreadwrite:
     os.write(LSreadwrite,"RAMP 1,1," + repr(rate) + " \r \n")

#get the rate
def readRate(): #ramp rate
   if LSreadwrite:
     os.write(LSreadwrite,"RAMP? 1 \r \n")
     time.sleep(1)
     return os.read(LSreadwrite,80)

# Turn on the heater to a given power level. 0=off, 1=low, 2=med, 3=high
def heaterRange(powlev):
   os.write(LSreadwrite,"RANGE " + repr(powlev) + " \r \n")
   
"""
