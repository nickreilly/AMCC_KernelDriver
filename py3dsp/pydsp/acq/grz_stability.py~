from run import rd
import numpy
import xdir
import pyfits
import filterBase
import time
import ls332

print 'What detector bias do you want to use?'
detbias = int(raw_input())

print 'How many images do you want to take?'
numframes=int(raw_input())

print 'How long do you want to wait for temperature stability before starting?'
print '    (enter in seconds)'
waittime = int(raw_input())

time.sleep(waittime)
burst()
time.sleep(5)
burst()
time.sleep(5)
burst()
tmps()
CurrTemp = int(round(ls332.readTemp())) # round and return as integer -- no decimal. Changed it to match the command from temp modulation code

 
# Loop through a number of applied biases.


detsub = dd.vreset + detbias
dd.dsub = detsub

    # Set the object name for this data with the bias in the name.
rd.object = "stability_dark_%dK_%dmV"%(CurrTemp, detbias)
rd.lc = ''
rd.gc = ''  
rd.nsamp = 1
rd.itime = 11000
rd.nrow = 2048
rd.nrowskip = 0    
for blah in range(100):
	sscan() # just take some throw-away data to help new bias settle.    

    
for blah in range(numframes):
	srun()



print 'Finished taking your data'

crun()
