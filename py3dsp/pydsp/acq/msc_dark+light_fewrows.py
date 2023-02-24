"""
cmc_dark+light.py - a script to run in pydsp.
    
  1) aquires images in SUTR mode with the first set in the dark, 
  2) moves the filter wheel to filter with high background, 
  3) sets the reset clock to zero (off),
  4) takes more SUTR images in the light
  5) moves the filter wheel back to cds (dark)
  6) takes more SUTR data in the dark to allow diodes to debias to zero.
  7) sets the reset clock back to default.

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

# Ask user for SCA type
print 'H1RG or H2RG?'
whichSCA = str(raw_input())

# HAWAII-1RG
if whichSCA == 'H1RG' :
    fullitime = 167
    longitime = 25000
    fullrow = 32
    skiprows = 1024/2

# HAWAII-2RG
if whichSCA == 'H2RG' :
    fullitime = 11000 # full array read time
    longitime = 25000
    fullrow = 2048

print 'What is the starting detector bias you want to do?'
startbias = int(raw_input())

print 'What is the ending detector bias you want to do?'
endbias = int(raw_input())

print 'In what increments do you want to change the detector bias?'
stepbias = int(raw_input())


#print 'What amount do you want to shift the Voffset voltage?'
#extraoffset = int(raw_input())
extraoffset = 0

OrigOffset = dd.voffset

print 'After taking data at the current temperature, do you want to change the temperature and take dark+light at the new temperature? (y/n)'
changeTemp = str(raw_input())
if (changeTemp == 'y' or changeTemp == 'Y' or changeTemp == 'yes'):
    print 'What is the FINAL temperature at which we are measuring dark current? NOTE: we can only do whole integer temperatures and steps. '
    FinalTemp = int(raw_input())  # integer because of range in for-loop
    print 'In what increments do you want to change the temperature?'
    StepTemp = int(raw_input())
    print 'How long do you want to wait for temperature stability before starting after temp change?'
    print '    (enter in seconds)'
    waittime_tempchange = int(raw_input())


print 'How long do you want to wait for temperature stability before starting?'
print '    (enter in seconds)'
waittime = int(raw_input()) 


def WaitAndBurst():
    time.sleep(waittime) # wait for temperature to stabilize
    burst()  # stop CRUN mode
    time.sleep(5)
    burst()  # sometimes, once is not enough
    time.sleep(5)


def WaitAndBurst_tempchange():
    time.sleep(waittime_tempchange) # wait for temperature to stabilize
    burst()  # stop CRUN mode
    time.sleep(5)
    burst()  # sometimes, once is not enough
    time.sleep(5)


def DarkVsBias():
    # What is our current temperature?
    tmps()
    CurrTemp = int(round(ls332.readTemp())) # round and return as integer -- no decimal. Changed it to match the command from temp modulation code
    #CurrTemp = round(readTemp(),1)
 
    # Loop through a number of applied biases.
    for detbias in range(startbias, endbias+1, stepbias):
        # The applied reverse detector bias = (dsub-vreset), invert to get dsub
        detsub = dd.vreset + detbias
        dd.dsub = detsub

        #change voffset for 350mV to -2230
        if detbias == 350:
            dd.voffset = -2230
        else:
            dd.voffset = -2135

        # Set the object name for this data with the bias in the name.
        rd.object = "dk_%dK_%dmV_%dmsitime"%(CurrTemp, detbias, fullitime)
        rd.gc = 'Taking dark+light data. No reset between SUTR sets.'
        rd.lc = 'This is the start of the dark data.'

        rd.nsamp = 1
        rd.itime = fullitime
        rd.nrow = fullrow
        rd.nrowskip = skiprows
        for blah in range(700):
            sscan() # just take some throw-away data to help new bias settle.
    
        # Take some samples in the dark
        rd.nsamp = 7000   # This is for a much shorter itime of 167 msec for subarray
        rd.itime = fullitime
        sutr()
        # Turn off the reset clock so that we still do non-destructive reads.
        dd.resetnhi = 0
        # Also, move the Voffset voltage to keep things on scale with A/D converters.
        dd.voffset = OrigOffset - extraoffset
        # Take some samples in the light
        filterBase.set("8.6")
        rd.nsamp = 50    # was 100
        rd.itime = longitime # was 60 sec with ND filter.
        rd.gc = 'Taking dark+light data. No reset between SUTR sets.'
        rd.lc = 'This is the light exposed data.'
        sutr()
        # Move filter back to dark and take more samples
        filterBase.set("cds")
        rd.nsamp = 7000
        rd.itime = fullitime
        rd.gc = 'Taking dark+light data. No reset between SUTR sets.'
        rd.lc = 'This is the data back in the dark after light exposure.'
        sutr()
        # Turn on the upper rail of reset clock (allows clocking reset, not continuous)
        dd.resetnhi = 3300


if (changeTemp == 'y' or changeTemp == 'Y' or changeTemp == 'yes'):
    # What is our current temperature?
    tmps()
    StartTemp = int(round(ls332.readTemp())) # round and return as integer - no decimal
    for NextTemp in range(StartTemp, FinalTemp+1, StepTemp):
        if NextTemp != StartTemp:
            time.sleep(1)
            ls332.setSetpointTemp(NextTemp) # Tell controller to change temperature
            time.sleep(1)
            crun()  # continuously read the array, for temp stability
            WaitAndBurst_tempchange()  # wait some time, then stop crun
            DarkVsBias() # Take data at the current temperature
        else:
            WaitAndBurst()
            DarkVsBias()
else :
    WaitAndBurst()  # wait some time, then stop crun
    DarkVsBias()  # just taking darks at this one temp

print 'Finished taking your data'
dd.voffset = OrigOffset
rd.lc = ''
rd.gc = ''
rd.nrow = 1024
#rd.ncol = 1024
rd.nrowskip = 0
rd.ncolskip = 0
rd.nsamp = 1
rd.itime = 5800
sscan()

time.sleep(1)
dd.dsub = 150
time.sleep(1)
dd.voffset = -2130
time.sleep(1)

crun()
