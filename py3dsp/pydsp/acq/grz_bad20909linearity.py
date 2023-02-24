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


print "Which filter? (3.3 or l')"
filterchoice = str(raw_input())
filterBase.set(filterchoice)

print 'What is the starting detector bias you want to do?'
startbias = int(raw_input())

print 'What is the ending detector bias you want to do?'
endbias = int(raw_input())

print 'In what increments do you want to change the detector bias?'
stepbias = int(raw_input())

print 'How many samples do you want for your SUTR data?'
numsamples = int(raw_input())

print 'How long do you want to wait for temperature stability before starting?'
print '    (enter in seconds)'
waittime = int(raw_input())


# What is our current temperature?
tmps()
CurrTemp = int(round(readTemp())) # round and return as integer -- no decimal

time.sleep(waittime)
burst()
time.sleep(5)
burst()
time.sleep(5)
rd.nrow=100
rd.nrowskip=0
rd.itime=1000

print 'Hey, we are taking linearity data.'

rd.lc = 'taking linearity: mean signal vs time'

# When taking linearity data, the important stuff happens at starvation and 
# saturation, i.e. at the beginning and end of the data set. 
# Loop through a number of applied biases.

for detbias in range(startbias, endbias+1, stepbias):
    # The applied reverse detector bias = (dsub-vreset), invert to get dsub
    detsub = dd.vreset + detbias
    dd.dsub = detsub
    rd.nsamp = 1
    
    # Set the object name for this data with the bias in the name.
    rd.object = "badlinearity%dK_%dmV"%(CurrTemp, detbias)
    sscan() # just take some throw-away data to help new bias settle.
    sscan()
    # We are going to take SUTR data. 
    rd.nsamp = numsamples
    sutr()


for detbias in range(startbias, endbias+1, stepbias):
    # The applied reverse detector bias = (dsub-vreset), invert to get dsub
    detsub = dd.vreset + detbias
    dd.dsub = detsub
    rd.nsamp = 1
    
    # Set the object name for this data with the bias in the name.
    rd.object = "badlinearity%dK_%dmV"%(CurrTemp, detbias)
    sscan() # just take some throw-away data to help new bias settle.
    sscan()
    # We are going to take SUTR data. 
    rd.nsamp = numsamples
    sutr()


for detbias in range(startbias, endbias+1, stepbias):
    # The applied reverse detector bias = (dsub-vreset), invert to get dsub
    detsub = dd.vreset + detbias
    dd.dsub = detsub
    rd.nsamp = 1
    
    # Set the object name for this data with the bias in the name.
    rd.object = "badlinearity%dK_%dmV"%(CurrTemp, detbias)
    sscan() # just take some throw-away data to help new bias settle.
    sscan()
    # We are going to take SUTR data. 
    rd.nsamp = numsamples
    sutr()

print 'Finished taking linearity data'

rd.nrow=2048
rd.itime=11000
rd.nsamp = 1
crun()





'''CRUN mode. type 'burst' to abort
ur_ok fwp 542.5
ur_ok filter k
Moving cw
New Location is: 
919.5
ur_ok sscan
Read /data/H2RG-20909/center-FW/src.fits C
ur_ok filter h
Moving cw
New Location is: 
947.25
ur_ok sscan
Read /data/H2RG-20909/center-FW/src.fits D
ur_ok filter j
Moving cw
New Location is: 
975.0
ur_ok sscan
Read /data/H2RG-20909/center-FW/src.fits A
ur_ok filter l'
Traceback (most recent call last):
  File "/home/dsp/dsp/pydsp/pydsp.py", line 686, in cloop
    execcmd(s, raw_input=raw_input, wfile=wfile)
  File "/home/dsp/dsp/pydsp/pydsp.py", line 559, in execcmd
    rd[tok[0]] = arg # assign
  File "/home/dsp/dsp/pydsp/DataDict.py", line 138, in __setitem__
    self.setfunc[name](val,*self.args[name],**self.kwds[name]) # call it with args
  File "/home/dsp/dsp/pydsp/filter_wheel/filterBase.py", line 102, in set
    raise ValueError, "can't find that on this filter wheel!"
ValueError: can't find that on this filter wheel!
ur_ok filter l''
Moving ccw
New Location is: 
864.0
ur_ok sscan
Read /data/H2RG-20909/center-FW/src.fits B
ur_ok fwp 925.5
ur_ok filter h
Moving cw
New Location is: 
947.25
ur_ok sscan
Read /data/H2RG-20909/center-FW/src.fits C
ur_ok fwp 864.0
ur_ok filter h
Moving cw
New Location is: 
947.25
ur_ok sscan
Read /data/H2RG-20909/center-FW/src.fits D
ur_ok fwp 987
ur_ok filter l''
Moving ccw
New Location is: 
864.0
ur_ok sscan
Read /data/H2RG-20909/center-FW/src.fits A
ur_ok fwp 884
ur_ok filter 3.3
Moving cw
New Location is: 
891.75
ur_ok sscan
Read /data/H2RG-20909/center-FW/src.fits B
ur_ok filter l''
Moving ccw
New Location is: 
864.0
ur_ok fwp 844
ur_ok filter l''
Moving cw
New Location is: 
864.0
ur_ok sscan
Read /data/H2RG-20909/center-FW/src.fits C
ur_ok fwp 927.5
ur_ok filter l''
Moving ccw
New Location is: 
864.0
ur_ok sscan
Read /data/H2RG-20909/center-FW/src.fits D
ur_ok fwp 854
ur_ok filter l''
Moving cw
New Location is: 
864.0
ur_ok sscan
Read /data/H2RG-20909/center-FW/src.fits A
ur_ok fwp 834
ur_ok filter l''
Moving cw
New Location is: 
864.0
ur_ok sscan
Read /data/H2RG-20909/center-FW/src.fits B
ur_ok fwp 764
ur_ok filter l''
Moving cw
New Location is: 
864.0
ur_ok sscan
Read /data/H2RG-20909/center-FW/src.fits C
ur_ok filter 3.3
Moving cw
New Location is: 
891.75
ur_ok sscan
Read /data/H2RG-20909/center-FW/src.fits D
ur_ok filter l''
Moving ccw
New Location is: 
864.0
ur_ok sscan
Read /data/H2RG-20909/center-FW/src.fits A
ur_ok filter j
Moving cw
New Location is: 
975.0
ur_ok sscan
Read /data/H2RG-20909/center-FW/src.fits B
ur_ok filter l''
Moving ccw
New Location is: 
864.0
ur_ok sscan
Read /data/H2RG-20909/center-FW/src.fits C
ur_ok nrow 1000
ur_ok nrow nrow 100
too many words! I'm confused!
ur_ok nrow 100





^C
'''
