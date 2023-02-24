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
    fullitime = 5800 # full array read time
    longitime = 25000
    fullrow = 1024


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

print 'What amount do you want to shift the Voffset voltage (mV)?'
extraoffset = int(raw_input())
#extraoffset = 0

OrigOffset = dd.voffset


print 'How long do you want to wait for temperature stability before starting?'
print '    (enter in seconds)'
waittime = int(raw_input()) 
#waittime = 5

def WaitAndBurst():
    print 'Taking a nap before starting!'
    crun2()
    time.sleep(waittime) # wait for temperature to stabilize
    burst()  # stop CRUN mode
    time.sleep(5)
    burst()  # sometimes, once is not enough
    time.sleep(5)


def nap(waittime):
    print('Napping!')
    initial_time = time.time()
    counter = 0
    while True:
        if counter%5 == 0: 
            sigrun()
        else:
           sscan()
        time.sleep(0.001)
        now_time = time.time()
        counter+=1
        if (now_time - initial_time) > waittime:
            print 'Waking up!'
            return


# What is our current temperature?
tmps()
#CurrTemp = int(round(readTemp())) # round and return as integer -- no decimal
CurrTemp = int(round(ls332.readTemp())) # round and return as integer -- no decimal. Changed it to match the command from temp modulation code
#CurrTemp = round(readTemp(),1)


# The applied reverse detector bias = (dsub-vreset), invert to get dsub
detsub = dd.vreset + detbias
dd.dsub = detsub

# Set the object name for this data with the bias in the name.
rd.object = "welldepth_%dK_%dmV"%(CurrTemp, detbias)
#rd.object = "dk_%sK_%dmV"%(str(CurrTemp), detbias)
rd.gc = 'Taking well depth data.'
#rd.lc = 'Post proton-radiation exposure'
rd.lc = ''

rd.nsamp = 1
rd.itime = fullitime
rd.nrow = fullrow
WaitAndBurst()
for blah in range(20):
    srun() # just take some throw-away data to help new bias settle.
rd.lc = 'This is the pedestal frame.'
pedrun()

# Turn off the reset clock so that we still do non-destructive reads.
dd.resetnhi = 0
# Take some samples in the light
##filterBase.set("8.6")
#filterBase.set("m'") #Maybe try m' if this does not work well.
filterBase.set("5.8")

rd.lc = 'This is the light exposed data.'

nap(60*60)
sigrun()
# Move filter back to dark and take more samples
filterBase.set("cds")
rd.lc = 'This is the data back in the dark after light exposure.'
nap(60*60)
sigrun()
# Turn on the upper rail of reset clock (allows clocking reset, not continuous)
dd.resetnhi = 3300

rd.lc = 'This is a cds after exposure'
for frame in range(10):
    srun()

print 'Finished taking your data'
rd.lc = ''
rd.gc = ''
#rd.nrow = 1024
#rd.ncol = 1024
rd.nrowskip = 0
rd.ncolskip = 0
rd.nsamp = 1
rd.dsub = 350
rd.object = test
rd.itime = fullitime
sscan()


crun2()
