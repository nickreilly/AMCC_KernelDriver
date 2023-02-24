from run import rd
from pydsp import cloop
import xdir
import pyfits
import time
import numpy as np

rd.object = 'IPC_vreset'

# print 'What detsub do you want to start at (largest value)?'
detstart= 450 # int(raw_input())


# print 'What detsub step size would you like to use (absolute value)?'
detstep = 100 # int(raw_input())

#print 'What step size would you like to use for Vreset?'
resetstep = 100 # int(raw_input())

# print 'How many rows in the full array?'
fullrow= 2048 # int(raw_input())

# print 'How many different row measurements would you like to take?'
numrows= 8 #int(raw_input())
rowstep=int(fullrow/(numrows+2))

row_starts = np.arange(100, 2048 - 100, 256)

counter = 0
for detsub in range(detstart,100,-detstep):
    for resetv in range(100+resetstep,detsub+1,resetstep):
        print('Working on ' + str(detsub - resetv) + ' which is counter ' + str(counter))
        counter = counter + 1


for detsub in range(detstart,100,-detstep):
    dd.dsub=detsub
    for resetv in range(100+resetstep,detsub+1,resetstep):
        print('Working on ' + str(detsub - resetv))
        for i_row, row_start in enumerate(np.arange(128, 2048, 256)):
            sscan()
            sscan()
            
		    
            dd.resetnhi=3300
            rd.nrow=2048
            rd.nrowskip=0
            rd._vreset = 100
            pedrun()
            
            dd._vreset=resetv
            rd.nrow = 256
            rd.nrowskip = row_start
            pedrun()
            
            dd.resetnhi=0
            rd.nrow=2048
            rd.nrowskip = 0
            pedrun()

            dd._vreset=100
            dd.resetnhi=3300


    '''
    print('Working on ' + str(detsub - resetv))
    for i_row, row_start in enumerate(np.arange(128, 2048, 256)):
        sscan()
        sscan()
        
	    
        dd.resetnhi=3300
        rd.nrow=2048
        rd.nrowskip=0
        rd._vreset = detsub
        pedrun()
        
        dd._vreset = resetv
        rd.nrow = 256
        rd.nrowskip = row_start
        pedrun()
        
        dd.resetnhi=0
        rd.nrow=2048
        rd.nrowskip = 0
        pedrun()

        dd._vreset=detsub
        dd.resetnhi=3300
    '''


			
'''
for detsub in range(detstart,100,-detstep):
	dd.dsub=detsub
	for resetv in range(100+resetstep,detsub+1,resetstep):
		for ipcrow in range(rowstep,fullrow-rowstep,rowstep):
			dd.resetnhi=3300
			rd.nrow=2048
			rd.nrowskip=0

			sscan()
			sscan()

			pedrun()
			dd.resetnhi=0
			dd._vreset=resetv
			rd.nrow=ipcrow
			pedscan()
			rd.nrow=2048
			pedrun()
			dd._vreset=100
			dd.resetnhi=3300

			sscan()
			sscan()

			pedrun()
			dd._vreset=resetv
			rd.nrow=ipcrow
			pedscan()
			dd.resetnhi=0
			rd.nrow=2048
			pedrun()

			dd._vreset=100
			dd.resetnhi=3300
'''


print('There were ' + str(counter) + ' tests')
dd.vreset=100
dd.dsub=350
rd.object='test'
#crun2()
