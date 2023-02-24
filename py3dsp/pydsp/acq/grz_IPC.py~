
from autouser import AutoUser
from run import rd
from pydsp import cloop
import xdir
import pyfits
import time
import ls332


print 'What is the starting detector bias you want to use?'
biasstart=int(raw_input())

print 'What is the ending detector bias you want to use?'
biasend = int(raw_input())

print 'What step size would you like to use for the detector bias?'
biasstep = int(raw_input())

print 'How many rows in the full array?'
fullrow=int(raw_input())

print 'How many different row measurements would you like to take?'
numrows=int(raw_input())
rowstep=int(fullrow/(numrows+1))

print 'How many times would you like to measure each row?'
numrepeats = int(raw_input())

print 'How long would you like to wait for temperature stability?'
sleeptime=int(raw_input())

time.sleep(sleeptime)
burst()
time.sleep(5)
burst()
time.sleep(5)
burst()

CurrTemp = int(round(ls332.readTemp()))

for biases in range(biasstart,biasend+1,biasstep):
	rd.object = "IPC_%dK_%dmV"%(CurrTemp, biases)
	dd.vreset=100	
	dd.dsub=biases+100
	dd.resetnhi=3300
	rd.nrow=2048
	rd.nrowskip=0
	for blah in range(10):
		sscan()
	for ipcrow in range(rowstep,fullrow,rowstep):
		for trial in range(numrepeats):			
			dd.resetnhi=3300
			rd.nrow=2048
			rd.nrowskip=0

			sscan()
			sscan()

			pedrun()
			dd.resetnhi=0
			dd._vreset=biases+100
			rd.nrow=ipcrow
			pedscan()
			rd.nrow=2048
			pedrun()
			dd._vreset=100
			dd.resetnhi=3300

			sscan()
			sscan()

			pedrun()
			rd.nrow=ipcrow
			dd._vreset=biases+100
			pedscan()
			dd.resetnhi=0
			rd.nrow=2048
			pedrun()

			dd._vreset=100
			dd.resetnhi=3300



dd.vreset=100
dd.dsub=biasstart + 100
dd.resetnhi=3300
crun2()
