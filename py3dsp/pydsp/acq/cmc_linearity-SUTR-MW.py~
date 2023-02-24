"""
cmc_linearity-SUTR.py - a script to run in pydsp.
    
Aquires images in SUTR mode.  The user should find a suitable filter that gives
a flux which is not too high. Then the user should determine how many samples to
do in the SUTR in order to reach saturation for the majority of the pixels.  
You should determine this for the highest bias that you will measure.  Thus at
lower biases, the number of samples will still be sufficient to saturate the pixels.

to use, type: execuser cmc_linearity-SUTR
"""

from run import rd
#from pydsp import cloop
import filterBase
#import xdir


#print "Which filter? (j, h, or k) h may be best choice."
#filterchoice = str(raw_input())
#filterBase.set(filterchoice)


print 'What is the starting detector bias you want to do?'
startbias = int(raw_input())

print 'What is the ending detector bias you want to do?'
endbias = int(raw_input())

print 'In what increments do you want to change the detector bias?'
stepbias = int(raw_input())

print 'How many samples do you want for your SUTR data?'
numsamples = int(raw_input())

print 'How many RAMPS do you want for each bias?'
numramps = int(raw_input())

print 'How long do you want to wait for temperature stability before starting?'
print '    (enter in seconds)'
waittime = int(raw_input())

time.sleep(22)
# What is our current temperature?

time.sleep(waittime)
burst()
time.sleep(5)
burst()
time.sleep(5)
tmps()
CurrTemp = int(round(readTemp())) # round and return as integer -- no decimal
filterBase.set(('cvfL', 3100)) #For if you want a spot on the CVF
for i_garbage in range(20):
    sscan()

print 'Hey, we are taking linearity data.'

rd.lc = 'taking linearity: mean signal vs time'

# When taking linearity data, the important stuff happens at starvation and 
# saturation, i.e. at the beginning and end of the data set. 
# Loop through a number of applied biases.

for detbias in range(startbias, endbias+1, stepbias):
  for ramps in range(0, numramps):
    # The applied reverse detector bias = (dsub-vreset), invert to get dsub
    detsub = dd.vreset + detbias
    dd.dsub = detsub
    rd.nsamp = 1
    
    # Set the object name for this data with the bias in the name.
    rd.object = "linearity%dK_%dmV"%(CurrTemp, detbias)
    sscan() # just take some throw-away data to help new bias settle.
    sscan()
    # We are going to take SUTR data. 
    rd.nsamp = numsamples
    sutr()



print 'Finished taking linearity data'

filterBase.set('cds')
rd.nsamp = 1
crun2()

