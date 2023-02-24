from run import rd
import numpy
import xdir
import pyfits
import filterBase
import time
import ls332



print 'What is the starting detector bias you want to do?'
startbias = int(raw_input())

print 'What is the ending detector bias you want to do?'
endbias = int(raw_input())

print 'In what increments do you want to change the detector bias?'
stepbias = int(raw_input())

print 'How many frames do you want to take?'
numframes=int(raw_input())

print 'What do you want the integration time to be?'
inttime=int(raw_input())

print 'What do you want nrow to be?'
nrowinput=int(raw_input())

print 'What do you want nrowskip to be?'
nrowskipinput=int(raw_input())

print 'How long do you want to wait for temperature stability before starting ?'
print '    (enter in seconds)'
waittime = int(raw_input())


filterBase.set("cds")

time.sleep(waittime)
burst()
time.sleep(5)
burst()
time.sleep(5)
burst()

tmps()
CurrTemp = int(round(ls332.readTemp())) # 

for detbias in range(startbias, endbias+1, stepbias):
    # The applied reverse detector bias = (dsub-vreset), invert to get dsub
    detsub = dd.vreset + detbias
    dd.dsub = detsub

    # Set the object name for this data with the bias in the name.
    rd.object = "CDSImages_%dK_%dmV"%(CurrTemp, detbias)

    rd.nsamp = 1
    rd.itime = inttime
    rd.nrow = nrowinput
    rd.nrowskip=nrowskipinput
    for blah in range(10):
        sscan() # just take some throw-away data to help new bias sett

    for blah in range(numframes):
        srun()

rd.nsamp = 1
crun2()
