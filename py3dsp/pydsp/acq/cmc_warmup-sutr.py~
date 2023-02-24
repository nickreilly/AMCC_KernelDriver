"""
cmc_warmup-sutr.py - a script to run in pydsp.
    
  1) aquires images in SUTR mode with the first set in the dark, 
  2) acquires a pedestal image
  3) adjusts voffset based upon pedestal frame if needed
  4) stops when we reach a certain temperature

to use, type: execuser cmc_warmup-sutr
"""

from run import rd
from numpy import *
import xdir
import pyfits
import time
import filterBase
import ls332

# Ask user for SCA type
print 'H1RG or H2RG?'
whichSCA = str(raw_input())

# Ask user for the temp to start warmup
print 'At what temperature (K) do you want to start taking data?'
mintemp = float(raw_input())

print 'How long do you want to wait before we start checking for the Temp trigger point (in seconds)? Useful if you just had to lower the temperature.'
waittime = int(raw_input())
time.sleep(waittime) # wait in seconds

# HAWAII-1RG
if whichSCA == 'H1RG' :
    fullitime = 5800
    subitime = 200
    fullrow = 1024

# HAWAII-2RG
if whichSCA == 'H2RG' :
    fullitime = 11000
    subitime = 200
    fullrow = 2048

# Issue burst command to end crun() sequence
burst()
time.sleep(5)
burst()
time.sleep(5)

# definitely want to take data in the dark!
filterBase.set("cds")

# Save voffset so we can reset after we are done.
OrigOffset = dd.voffset
rd.object = "warmup"
rd.lc = ''
rd.gc = '' 

def getdata():
    rd.itime = fullitime
    rd.nsamp = 1
    # recreate file name (duplicate code from runrun)
    pedfilename = xdir.get_nextobjfilename() + ".fits"
    pedrun()
    pedfile = pyfits.open(pedfilename)
    # These are FITS files. Get just the data and ignore header.
    peddata = pedfile[0].data
    # Check the pedestal level. Is it beginning to go off-scale? Adjust if needed. Prefer the values be towards the low end -32768 so that we have head room.
    oldoffset = dd.voffset
    pedmean = peddata[fullrow/2+100:fullrow/2+150, fullrow/2+100:fullrow/2+150].mean()  # added 100 so that it isn't in center gap of duplex 2Kx1Ks.
    if pedmean > 0 :
        dd.voffset = oldoffset - 50
    if pedmean < -25000 :
        dd.voffset = oldoffset + 50

    # Now take some full frame SUTR data
    rd.nsamp = 4
    rd.itime = fullitime
    sutr()

    # Take some samples for a sub-array read, but faster itime
    rd.nsamp = 4
    rd.itime = subitime
    rd.nrow = 32
    rd.nrowskip = fullrow/2
    sutr()
    # Put it back to full array
    rd.nrow = fullrow
    rd.nrowskip = 0
    rd.itime = fullitime
    rd.nsamp = 1
    # take a "throw away" frame to fix "rows not reset" effect.
    sscan()

# What is the temperature?
tempnow = ls332.readTemp()

# Take some data until we go above a certain temperature
while tempnow < 65.0 :
    #if tempnow > mintemp:
        #getdata()
    getdata()
        
    # else :
        # #time.sleep(10) # low temp data not useful, so just wait.
        # ssscan() # or just take scans to keep ROIC stable.

    tempnow = ls332.readTemp()


print 'Finished taking your data'
dd.voffset = OrigOffset
rd.lc = ''
rd.gc = ''
rd.nrow = fullrow
rd.ncol = fullrow
rd.nrowskip = 0
rd.ncolskip = 0
rd.nsamp = 1
rd.itime = fullitime
rd.dsub = 100
sscan()
