"""
cmc_nvs-pixel.py - a script to run in pydsp.
    
    We are taking data to measure the capacitance of a detector array.
    This is the version to use when you want to measure the capacitance on
    a pixel-by-pixel basis, i.e. when you have a LWIR array that has many
    non-uniform pixels and there are many high dark current pixels.

    This aquires a series of images for either changing wavelength or
    changing integration times.  
    Later, the user will process these data on a pixel-by-pixel basis to 
    extract the noise squared and signal values, and then plot to get a slope. 

to use, type: execuser filename
where filename is this file's name, but WITHOUT the .py
"""

from run import rd
from pydsp import cloop
from numpy import *
import xdir
import pyfits
import time
import filterBase
import numpy
import kjl375



#print 'Which method do you want to use to get changing fluence:\
#       \n  1) Changing wavelength (preferred method) \
#       \n  2) Changing integration time'
method = 1#int(raw_input())

if method ==2:
    print 'What wavelength on CVF K would you like to use? 2700 is recommended.'
    wave2=int(raw_input())
    print 'What is the shortest integration time you want (enter in milliseconds)?'
    starttime=int(raw_input())
    print 'What is the longest integration time you want?'
    endtime=int(raw_input())
    print 'What step size do you want for integration time?'
    timestep=int(raw_input())

#print 'What is the starting detector bias you want to do?'
startbias = 0#int(raw_input())

#print 'What is the ending detector bias you want to do?'
endbias = 1#int(raw_input())

#print 'In what increments do you want to change the detector bias?'
stepbias = 100#int(raw_input())

#print 'What do you want the integration time of the full frame to be (enter in milliseconds)?'
int_time = 11000#int(raw_input())

print 'How long do you want to wait for temperature stability before starting?'
print '    (enter in seconds)'
waittime = int(raw_input())

print 'What temp are you taking this at?'
desired_temp = float(raw_input())

fullarrayitime=int_time
rowstart=0
rowend=2048

#print "How many PAIRS of images do you want to take at each signal level?"
#NumImgPairs = int(raw_input())


#time.sleep(waittime)
##########################
#Filename, itime, temperature desired, threshold temperature, how long you want to wait at that temperature before starting
kjl375.pre_sleep('capacitanceTEST', 11000, desired_temp, 1, waittime)
##################
rd.object = 'TestSleeps'


print 'Finished taking Capacitance data'
rd.lc = ''
filterBase.set("cds")  # put detector back in dark
rd.itime = int_time
sscan() # take an image to reset array -- helps clear residual images.


kjl375.post_sleep('capacitanceTEST', int_time, 3)

#crun()
