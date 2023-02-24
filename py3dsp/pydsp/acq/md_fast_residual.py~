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
    longitime = 25000
    fullrow = 1024
    fullcol = 1024

# HAWAII-2RG
if whichSCA == 'H2RG' :
    fullitime = 11000 # full array read time
    longitime = 25000
    fullrow = 2048
    fullcol = 2048

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

# Set up subarray reads so that we can read out fast and meet NEOCam reqt
rd.nrow = 10
rd.nrowskip = 1020
rd.ncol = fullcol
## Added ncols to make array read faster
#rd.ncol = 1024

# Take some dark images first
print ' Taking 100 dark images'
rd.itime = 60
for blah in range(100):
    srun()

print ' Now starting the residual test 1'
# Initialize this loop to take the residual image test 16 times
for test in range(0, 16, 1):
    # Take image with filter 8.8 um and then slew to dark and take 100 imgs
    #rd.lc = 'This is the light image data with 3 s'
    rd.lc = 'This is the light image data with 100 ms' #Delete me!
    filterBase.set("8.8")
    rd.nsamp = 1
    #rd.itime = 3000
    rd.itime = 100 #Delete me!
    srun()
    #rd.lc = 'This is the dark image data after 8.8 um filter 3 s'
    rd.lc= 'This is the dark image data after 8.8 um filter 100 ms'#Delete me!
    filterBase.set("cds")
    rd.itime = 60
    for blah in range(100):
        srun()
    # Wait for a bit before repeating the same test (and in between tests)
    rd.itime = 3000
    crun()
    time.sleep(120)
    burst()
    burst()

print ' Now starting the residual test 2'
# Repeat above for a different integration time (increase flux) 28 s
for test in range(0, 16, 1):
    #rd.lc = 'This is the light image data with 28 s'
    rd.lc = 'This is the light image data with 200 ms' #Delete me!
    filterBase.set("8.8")
    rd.nsamp = 1
    #rd.itime = 28000
    rd.itime = 200 #Delete me!
    srun()
    #rd.lc = 'This is the dark image data after 8.8 um filter 28 s'
    rd.lc= 'This is the dark image data after 8.8 um filter 200 ms'#Delete me!
    filterBase.set("cds")
    rd.itime = 60
    for blah in range(100):
        srun()
    rd.itime = 3000
    crun()
    time.sleep(120)
    burst()
    burst()

print ' Now starting the residual test 3'
# Repeat above for a different integration time (increase flux again) 1 min
for test in range(0, 16, 1):
    #rd.lc = 'This is the light image data with 1 min'
    rd.lc = 'This is the light image data with 400 ms' #Delete me!
    filterBase.set("8.8")
    rd.nsamp = 1
    #rd.itime = 60000
    rd.itime= 400 #Delete me!
    srun()
    #rd.lc = 'This is the dark image data after 8.8 um filter 1 min'
    rd.lc= 'This is the dark image data after 8.8 um filter 400 ms'#Delete me!
    filterBase.set("cds")
    rd.itime = 60
    for blah in range(100):
        srun()
    rd.itime = 3000
    crun()
    time.sleep(120)
    burst()
    burst()

print ' Now starting the residual test 4'
# Repeat above for a different integration time 10 min
for test in range(0, 16, 1):
    #rd.lc = 'This is the light image data with 10 min'
    rd.lc = 'This is the light image data with 800 ms' #Delete me!
    filterBase.set("8.8")
    rd.nsamp = 1
    #rd.itime = 600000
    rd.itime = 800 #Delete me!
    srun()
    #rd.lc = 'This is the dark image data after 8.8 um filter 10 min'
    rd.lc= 'This is the dark image data after 8.8 um filter 800 ms'#Delete me!
    filterBase.set("cds")
    rd.itime = 60
    for blah in range(100):
        srun()
    rd.itime = 3000
    crun()
    time.sleep(120)
    burst()
    burst()

print ' Now starting the residual test 5'
# Repeat above for a different integration time 1,000 s (max for dsp clk pgm)
for test in range(0, 16, 1):
    #rd.lc = 'This is the light image data with 1,000 s (16.6 min)'
    rd.lc = 'This is the light image data with 1.6 s' #Delete me!
    filterBase.set("8.8")
    rd.nsamp = 1
    #rd.itime = 1000000
    rd.itime = 1600 #Delete me!
    srun()
    #rd.lc = 'This is the dark image data after 8.8 um filter 1,000 s'
    rd.lc= 'This is the dark image data after 8.8 um filter 1.6 s'#Delete me!
    filterBase.set("cds")
    rd.itime = 60
    for blah in range(100):
        srun()
    rd.itime = 3000
    crun()
    time.sleep(120)
    burst()
    burst()

print ' Now starting the residual test 6'
# Repeat above for a different integration time 1 hr
for test in range(0, 8, 1):
    #rd.lc = 'This is the light image data with 1 hr'
    rd.lc = 'This is the light image data with 3.2 s' #Delete me!
    filterBase.set("8.8")
    rd.nsamp = 4
    #rd.itime = 1000000
    rd.itime = 3200 #Delete me!
    sutr()
    #rd.lc = 'This is the dark image data after 8.8 um filter 1 hr'
    rd.lc= 'This is the dark image data after 8.8 um filter 3.2 s'#Delete me!
    filterBase.set("cds")
    rd.nsamp = 1
    rd.itime = 60
    for blah in range(100):
        srun()
    rd.itime = 3000
    crun()
    time.sleep(120)
    burst()
    burst()

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











