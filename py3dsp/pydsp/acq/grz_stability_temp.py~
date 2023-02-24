from run import rd
import numpy
import xdir
import pyfits
import filterBase
import time
import ls332
from numpy import pi, sin

print 'What detector bias do you want to use?'
detbias = int(raw_input())

print 'How many images do you want to take?'
numframes=int(raw_input())

print 'What do you want the base temperature (K) to be?'
basetemp=int(raw_input())

print 'What do you want the amplitude of oscillation (mK) to be?'
amplitude=float(raw_input())/1000.0

print 'What do you want the average rate (mK/min) to be?'
rate=float(raw_input())/60000.0
quarter=float(amplitude)/float(rate)

print 'How long do you want to wait for temperature stability before starting?'
print '    (enter in seconds)'
waittime = int(raw_input())

ls332.setSetpointTemp(basetemp)
time.sleep(waittime)
burst()
time.sleep(5)
burst()
time.sleep(5)
tmps()
CurrTemp = int(round(ls332.readTemp())) # round and return as integer -- no decimal. Changed it to match the command from temp modulation code



detsub = dd.vreset + detbias
dd.dsub = detsub

    # Set the object name for this data with the bias in the name.
rd.object = "stability_darktemp_%dK_%dmV"%(CurrTemp, detbias)
rd.lc = ''
rd.gc = ''  
rd.nsamp = 1
rd.itime = 11000
rd.nrow = 2048
rd.nrowskip = 0    
for blah in range(100):
	sscan() # just take some throw-away data to help new bias settle.
	time.sleep(.5) #To match the timing below where a delay is needed to communicate with the temperature controller    


starttime=time.time() 
for blah in range(numframes):
	srun()
	currtime=time.time()
	newtemp=round(amplitude*sin(pi*(currtime-starttime)/(2.0*quarter)) + basetemp,3)
	time.sleep(.25)
	ls332.setSetpointTemp(newtemp)
	time.sleep(.25)

ls332.setSetpointTemp(basetemp)
print 'Finished taking your data'

crun()
