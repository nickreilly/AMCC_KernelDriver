"""
md_residualvsflux.py - a script to run in pydsp.
    
    This is a script to take residual image data with fast sub-array reads.
 
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


filterBase.set("cds")
rd.object = 'residualvsflux'

# Ask user for SCA type
print 'H1RG or H2RG?'
whichSCA = str(raw_input())

# HAWAII-1RG
if whichSCA == 'H1RG' :
    fullitime = 5800 # full array read time
    fullrow = 1024
    fullcol = 1024

# HAWAII-2RG
if whichSCA == 'H2RG' :
    fullitime = 11000 # full array read time
    fullrow = 2048
    fullcol = 2048

# Set wait time
print 'How long do you want to wait between tests due to previous bright source residual decay before starting?'
print '    (enter in seconds)'
waittime = int(raw_input()) 

def WaitAndBurst():
    time.sleep(waittime) # wait for temperature to stabilize
    burst()  # stop CRUN mode
    time.sleep(5)
    burst()  # sometimes, once is not enough
    time.sleep(5)
    burst()

WaitAndBurst()

# Set up subarray reads so that we can read out fast and meet NEOCam reqt
rd.nrow = 10
rd.nrowskip = 1020
rd.ncol = fullcol

rd.gc = 'Taking Residual vs. Flux data'

# Take some dark images first
print ' Taking 100 dark images'
rd.itime = 60
for blah in range(100):
    srun()

# Start a loop to increase itime with 8.8 um filter
for t in range(60, 1800, 30):

    # Initialize this loop to take the residual image test 16 times
    for test in range(0, 16, 1):
        # Take image with filter 8.8 um 
        rd.lc = 'This is the light image data with itime '+str(t)+' ms'
        filterBase.set("8.8")
        rd.nsamp = 1
        rd.itime = t
        srun()
        rd.lc = 'This is the dark data after 8.8 um filter '+str(t)+' ms'
        filterBase.set("cds")
        rd.itime = 60
        for blah in range(100):
            srun()
        # Wait for a bit before repeating the same test (and in between tests)
        rd.itime = 3000
        crun()
        WaitAndBurst()

print ' Finished Residual vs. Flux'
# Return the original full integration time and frame size
rd.lc = ''
rd.gc = ''
rd.nrow = fullrow
rd.ncol = fullcol
rd.nrowskip = 0
rd.ncolskip = 0
rd.nsamp = 1
rd.itime = fullitime
# Take an image and then start crun mode
sscan()
crun()











