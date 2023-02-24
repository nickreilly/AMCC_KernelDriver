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


"""
print 'Which method do you want to use to get changing fluence:\
       \n  1) Changing wavelength (preferred method) \
       \n  2) Changing integration time'
"""
method = 1 # int(raw_input())

print 'What is the starting detector bias you want to do?'
startbias = int(raw_input())

print 'What is the ending detector bias you want to do?'
endbias = int(int(raw_input()) + 1)

# print 'In what increments do you want to change the detector bias?'
stepbias = 100 # int(raw_input())

print 'What do you want the integration time of the full frame to be (enter in milliseconds)?'
int_time = int(raw_input())

print 'How long do you want to wait for temperature stability before starting?'
print '    (enter in seconds)'
waittime = int(raw_input())

#print 'What temp are you taking this at?'
#desired_temp = float(raw_input())

fullarrayitime=int_time
rowstart=0
rowend=2048

def nap(waittime):
    previous_object = rd.object
    rd.object = 'test'
    print 'Napping!'
    initial_time = time.time()
    while True:
        sscan()
        time.sleep(0.001)
        now_time = time.time()
        if (now_time - initial_time) > waittime:
            break
    rd.object = previous_object

# A small number (2 or 4) is needed for doing box-mean version of capacitance 
# calculation. But a larger number (50) is needed to calculate per-pixel 
# capacitance.  Always do this in pairs for compatibility with the 
# difference-box-mean method.  Also, the calculation of stats within this
# program requires PAIRS of images!

# print "How many PAIRS of images do you want to take at each signal level? (try 25)"
NumImgPairs = 25 # int(raw_input())

nap(waittime)

##########################
#Filename, itime, temperature desired, threshold temperature, how long you want to wait at that temperature before starting
# kjl375.pre_sleep('capacitance', 11000, desired_temp, 1, waittime)

##################

print 'taking noise squared vs. signal data...'

rd.lc = 'taking noise squared vs. signal data for capacitance'
rd.nsamp = 1

# What is our current temperature?
tmps()
time.sleep(1)
CurrTemp = int(round(readTemp())) # round and return as integer -- no decimal

print 'Taking Garbage Frames!'
filterBase.set(("cds"))
for garbage in range(10):
    sscan()
    print str(garbage+1) + ' out of 20 garbage frames'

# Loop through a number of applied biases.
for detbias in range(startbias, endbias+1, stepbias):
    # The applied reverse detector bias = (dsub-vreset), invert to get dsub
    detsub = dd.vreset + detbias
    dd.dsub = detsub
    # Set the object name for this data with the bias in the name.
    #rd.object = "cap_perpix_%dK_%dmV"%(CurrTemp, detbias)
    rd.object = "capacitance_%dK_%dmV"%(CurrTemp, detbias)
    rd.itime = int_time
    nap(300) # Take a nap to let the new bias settle

    if method == 1 :
        start_wave = 3000
        end_wave = 4450
        colors = 21
        step_size = int((end_wave - start_wave) / colors)
        # for wavelen in range(3000,4550 + 1, 50):
        # for wavelen in range(start_wave, end_wave, step_size):  # Filter Wheel 5
            # filterBase.set(("cvfL", wavelen)) 
        for wavelen in range(4300, 5801, 100):  # Filter Wheel 2
            filterBase.set(("cvfII", wavelen)) 
            for zzzz in range(0, NumImgPairs):
                # Need to setup file name before doing [ped,s,b]run
                runfilename1 = xdir.get_nextobjfilename() + ".fits"
                srun() # Take our images. 
                runfilename2 = xdir.get_nextobjfilename() + ".fits"
                srun()


print 'Finished taking Capacitance data'
rd.lc = ''
filterBase.set("cds")  # put detector back in dark
rd.itime = int_time
sscan() # take an image to reset array -- helps clear residual images.


#kjl375.post_sleep('capacitance', int_time)
rd.object = 'test'
crun2()
