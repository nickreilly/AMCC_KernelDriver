from run import rd
#import numpy
#import xdir
#import pyfits
import time
import filterBase
import ls332

print('What do you want the new temperature to be? (K)')
newtemp=int(raw_input())

print('How many frames do you want to take in your sutr ramp?')
numframes=int(raw_input())

print('What bias do you want to use? (mV)')
bias=int(raw_input())

print('How long do you want to wait before starting? (s)')
sleeptime=int(raw_input())

time.sleep(sleeptime)
burst()
time.sleep(5)
burst()
time.sleep(5)
burst()
time.sleep(5)

tmps()
CurrTemp = int(round(ls332.readTemp()))
rd.object = 'tempchange_' + str(CurrTemp) + 'K_' + str(newtemp) + 'K_' + str(bias) + 'mV'
dd.dsub =dd.vreset + bias
for blah in range(0,20):
	sscan()


rd.nsamp=numframes
ls332.setSetpointTemp(newtemp)
time.sleep(5) #Need this because serial communication is stupid slow...
sutr()


rd.nsamp=1
print('Done taking your data!')
crun2()
