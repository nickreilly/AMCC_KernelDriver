"""
md_stability.py - a script to run in pydsp.
    
    This is a script to measure the radiometric stability of an H2RG det.
 
to use, type: execuser filename
where filename is this file's name, but WITHOUT the .py
"""

from run import rd
#import numpy
#import xdir
#import pyfits
import time
import filterBase
import ls332

# Set directory
rd.object = 'stability40K_2'

# Set wait time
print 'How long do you want to wait for temperature stability or previous bright source residual decay before starting?'
print '    (enter in seconds)'
waittime = int(raw_input()) 

# Wait then exit crun mode
time.sleep(waittime)
burst()  
time.sleep(5)
burst()  
time.sleep(5)

# Move to appropriate filter
filterBase.set("3.3")

# Set integration time and image mode
rd.itime = 22000
rd.nsamp = 2

# Set global comment
rd.gc = 'Looking at Blackbody T=100 C'

# Get the temperature
temp = ls332.readTemp()

while temp <= 44:
    # Get the room temperature
    roomtemp = ls332.readTempB()
    # Update the local comment
    rd.lc = 'Room temperature at start of image is %f' % roomtemp
    # Take one image
    srun()
    # Take ten images between the saved images
    for blah in range(10):
        sscan()
    # Take temp again to stop taking data when it starts to warm up
    temp = ls332.readTemp()

# Return the filter wheel to the dark slide
rd.lc = ''
rd.gc = ''
rd.nsamp = 1
rd.itime = 11000
filterBase.set("cds")
