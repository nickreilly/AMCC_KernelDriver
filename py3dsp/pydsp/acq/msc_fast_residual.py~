"""
md_fast_residual.py - a script to run in pydsp.
    
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
rd.object = 'fast_residual'

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
print 'How long do you want to wait for temperature stability before starting?'
print '    (enter in seconds)'
waittime = int(raw_input()) 

# Wait then exit crun mode
time.sleep(waittime)
burst()  
time.sleep(5)
burst()  
time.sleep(5)

# Set up subarray reads so that we can read out fast
rd.nrow = 10
rd.nrowskip = 512
dark_itime = 55
## Added ncols to make array read faster
#rd.ncol = 1024

# Take some dark images first
print 'Taking 1100 dark images'
rd.itime = dark_itime
for blah in range(1100):
    srun()

#30 minutes is 1,800,000 milliseconds
#take 6 samples up the ramp with 30,000 millisecond itime
print ' Now starting the residual test 1'
# Initialize this loop to take the residual image test 16 times
for test in range(0, 16, 1):
    # Take image with filter 7.1 um and then slew to dark and take 1100 imgs
    rd.lc = 'This is the light image data for test 1'
    filterBase.set("7.1")
    rd.nsamp = 6
    #rd.itime = 300000
    rd.itime = 150000
    sutr()
    rd.nsamp=1
    rd.lc = 'This is the dark image data for test 1'
    filterBase.set("cds")
    rd.itime = dark_itime
    for blah in range(1100):
        srun()
    # Wait for a bit before repeating the same test (and in between tests)
    rd.itime = 3000
    crun()
    time.sleep(120)
    burst()
    burst()


print ' Now starting the residual test 2'
# Initialize this loop to take the residual image test 16 times
for test in range(0, 16, 1):
    # Take image with filter 7.1 um
    rd.lc = 'This is the light image data for test 2'
    filterBase.set("7.1")
    rd.nsamp = 1
    rd.itime = 30000
    srun()
    rd.nsamp=1
    rd.lc = 'This is the dark image data for test 2'
    filterBase.set("cds")
    rd.itime = dark_itime
    for blah in range(1100):
        srun()
    # Wait for a bit before repeating the same test (and in between tests)
    rd.itime = 3000
    crun()
    time.sleep(120)
    burst()
    burst()


print ' Now starting the residual test 3'
# Initialize this loop to take the residual image test 16 times
for test in range(0, 16, 1):
    # Take image with filter 8.8 um
    rd.lc = 'This is the light image data for test 3'
    filterBase.set("8.8")
    rd.nsamp = 1
    rd.itime = 30000
    srun()
    rd.nsamp=1
    rd.lc = 'This is the dark image data for test 3'
    filterBase.set("cds")
    rd.itime = dark_itime
    for blah in range(1100):
        srun()
    # Wait for a bit before repeating the same test (and in between tests)
    rd.itime = 3000
    crun()
    time.sleep(120)
    burst()
    burst()


'''
print ' Now starting the residual test 2'
for test in range(0, 16, 1):
    rd.lc = 'This is the light image data for test 2'
    filterBase.set(('cvfIII',13.0))
    rd.nsamp = 6
    rd.itime = 300000
    sutr()
    rd.nsamp=1
    rd.lc = 'This is the dark image data for test 2'
    filterBase.set("cds")
    rd.itime = dark_itime
    for blah in range(700):
        srun()
    rd.itime = 3000
    crun()
    time.sleep(120)
    burst()
    burst()


print ' Now starting the residual test 3'
# Initialize this loop to take the residual image test 16 times
for test in range(0, 16, 1):
    # Take image with filter 7.1 um
    rd.lc = 'This is the light image data for test 3'
    filterBase.set("7.1")
    rd.nsamp = 1
    rd.itime = 4000
    srun()
    rd.nsamp=1
    rd.lc = 'This is the dark image data for test 3'
    filterBase.set("cds")
    rd.itime = dark_itime
    for blah in range(1100):
        srun()
    # Wait for a bit before repeating the same test (and in between tests)
    rd.itime = 3000
    crun()
    time.sleep(120)
    burst()
    burst()

print ' Now starting the residual test 4'
for test in range(0, 16, 1):
    rd.lc = 'This is the light image data for test 4'
    filterBase.set(('cvfIII',13.0))
    rd.nsamp = 1
    rd.itime = 2000
    srun()
    rd.lc = 'This is the dark image data for test 4'
    filterBase.set("cds")
    rd.itime = dark_itime
    for blah in range(700):
        srun()
    rd.itime = 3000
    crun()
    time.sleep(120)
    burst()
    burst()
'''


print ' Finished Fast Residual'
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











