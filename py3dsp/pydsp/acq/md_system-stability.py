"""
md_system-stability.py - a script to run in pydsp.
    
    This is a script written to mimic the stability program for an H2RG det, 
    but run when the detector is not connected. Monitoring the stability of
    the system.
 
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
rd.object = 'drain-stability'

# Set integration time and image mode
rd.itime = 22000
rd.nsamp = 2

# Set global comment
rd.gc = 'System Stability, 2kOhm resistors connected to 4 outputs'

# Take approx same number of images as were obtained with md_stability.py for
# H2RG-18235
for i in range(409):
    # Get the room temperature
    roomtemp = ls332.readTempB()
    # Update the local comment
    rd.lc = 'Room temperature at start of image is %f' % roomtemp
    # Take one image
    srun()
    # Take ten images between the saved images
    for blah in range(10):
        sscan()
    # Take temp again - not used, but this should stay to mimic overhead time
    temp = ls332.readTemp()

rd.lc = ''
rd.gc = ''
rd.nsamp = 1
rd.itime = 11000
