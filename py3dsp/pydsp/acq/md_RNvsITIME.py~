"""
md_RNvsITIME.py - a script to run in pydsp.
    
    script to acquire a sequence of SUTR images, which may be used to 
    calculate the read noise on a pixel-by-pixel basis.

    read noise sutr are obtained with different itimes - first full frames,
    then sub arrays with shorter itime.
    
to use, type: execuser md_RNvsITIME
"""
from run import rd
#import xdir
import pyfits
import filterBase
import time
import ls332

print 'How long do you want to wait for temperature stability before starting?'
print '    (enter in seconds)'
waittime = int(raw_input())  
time.sleep(waittime)

burst()
time.sleep(5)
burst()
time.sleep(5)

# Set up full rows
rd.nrow = 2048
rd.ncol = 2048
rd.itime = 11000

# Hardcode the bias that we will be using
detbias = 250

filterBase.set("cds")

# What is our current temperature?
tmps()
CurrTemp = int(round(readTemp())) # round and return as integer -- no decimal

# The applied reverse detector bias = (dsub-vreset), invert to get dsub
detsub = dd.vreset + detbias
dd.dsub = detsub
# Set the object name for this data with the temp & bias in the name.
rd.object = "RNvsITIME%dK_%dmV"%(CurrTemp, detbias)

# Settle detector with new bias by reseting+reading array.
rd.nsamp = 1
for blah in range(12):
    sscan()

# Take 64 SUTR-36 with full array reads
rd.nsamp = 36
for kimages in range(0, 64):
    sutr()

# Take another 12 throw away CDS images
rd.nsamp = 1
for blah in range(12):
    sscan()

# Take 64 SUTR-36 with 3 sec sub array reads
rd.nrow = 10
rd.nrowskip = 1020
rd.itime = 3000
rd.nsamp = 36
for kimages in range(0, 64):
    sutr()

# Take another 12 throw away CDS images - this will be fast
rd.nsamp = 1
for blah in range(12):
    sscan()

# Take 64 SUTR-36 with 1.5 sec sub array reads
rd.nrow = 10
rd.nrowskip = 1020
rd.itime = 1500
rd.nsamp = 36
for kimages in range(0, 64):
    sutr()

# Return the detector to original full frame reads
rd.nrow = 2048
rd.nrowskip = 0
rd.itime = 11000
rd.nsamp = 1
sscan()
sscan()
print 'Finished taking data'


crun()
