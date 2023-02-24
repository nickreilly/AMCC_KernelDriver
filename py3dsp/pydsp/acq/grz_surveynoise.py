from run import rd
import numpy
import xdir
import pyfits
import filterBase
import time
import ls332

print 'What is the detector bias you want to do?'
detbias = int(raw_input())

print 'How many rows do you want to read for each sub-array?'
skip = int(raw_input())
rd.nrowskip = 0
rowstart = 0

print 'How many rows in the full image?'
rowend = int(raw_input())
colstart = 0
colend = rd.ncol # full row of pixels (assuming user set it to full row)

print 'What integration time would you like for each sub-array read (enter in ms)?'
subarrayitime = int(raw_input())
rd.nsamp = 1

print 'How many survey cycles would you like to perform?'
numcycles=int(raw_input())

print 'What do you want the cycle time to be (enter in s)?'
cycletime=int(raw_input())
pedtime=5.17*skip
totaltime=(subarrayitime+pedtime)/1000.0
maxframes=int(round(cycletime/totaltime -0.49))

print 'How many frames do you want per cycle? For your chosen cycle time, subarray size, and integration time this MUST be ' + str(maxframes) + ' frames or fewer!'
numframes=int(raw_input())

print 'How long do you want to wait for temperature stability before starting?'
print '    (enter in seconds)'
waittime = int(raw_input())

time.sleep(waittime)
burst()
time.sleep(5)
burst()
time.sleep(5)
tmps()
CurrTemp = int(round(ls332.readTemp())) # round and return as integer -- no decimal. Changed it to match the command from temp modulation code

 
# Loop through a number of applied biases.

# The applied reverse detector bias = (dsub-vreset), invert to get dsub
detsub = dd.vreset + detbias
dd.dsub = detsub

    # Set the object name for this data with the bias in the name.
rd.object = "surveynoise_%dK_%dmV"%(CurrTemp, detbias)
    
filterBase.set("cds")
rd.itime=subarrayitime
rd.nrow = skip
for rowskip in range(rowstart, rowend, skip):
    rd.nrowskip=rowskip
    for cycles in range(numcycles):
        timestart=time.time()
        for blah in range(numframes):
            srun()
        timestop=time.time()
        time.sleep(cycletime-timestop+timestart)      
 
print 'Finished taking your data'
rd.lc = ''
rd.gc = ''
rd.nrowskip = 0
rd.ncolskip = 0
rd.nrow=2048
rd.nsamp = 1
rd.itime = 11000
sscan()

crun()
