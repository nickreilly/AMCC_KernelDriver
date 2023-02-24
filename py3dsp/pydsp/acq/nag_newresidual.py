"""
cmc_residual.py - a script to run in pydsp.
    
  At the moment this is setup for testing residual/persistent images 
  for the NEOCam mission.  
  
  Change as needed for your mission requirements.
 
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
    fullcol = 1024
    #subarray = 512
    #subskip = 256
    #ncols = 512

# HAWAII-2RG
if whichSCA == 'H2RG' :
    fullitime = 11000 # full array read time
    longitime = 25000
    fullrow = 2048
    fullcol = 2048
    subarray = 256
    subskip = 896


print 'What is the starting detector bias you want to do?'
startbias = int(raw_input())

print 'What is the ending detector bias you want to do?'
endbias = int(raw_input())

print 'In what increments do you want to change the detector bias?'
stepbias = int(raw_input())

OrigOffset = dd.voffset

print 'How long do you want to wait between tests due to previous bright source residual decay before starting?'
print '    (enter in seconds)'
waittime = int(raw_input()) 

def WaitAndBurst():
    time.sleep(waittime) # wait for temperature to stabilize
    burst()  # stop CRUN mode
    time.sleep(5)
    burst()  # sometimes, once is not enough
    time.sleep(5)
    burst()


def ResidualVsBias():
    '''
    # Set up subarray reads so that we can read out fast and meet NEOCam reqt
    rd.nrow = subarray
    rd.nrowskip = subskip
    ## Added ncols to make array read faster
    rd.ncol = ncols
    '''
    # Set up subarray reads so that we can read out fast and meet NEOCam reqt
    rd.nrow = 10
    rd.nrowskip = 1020
    ## Added ncols to make array read faster
    rd.ncol = 2048
    rd.ncolskip = 0

    # What is our current temperature?
    tmps()
    CurrTemp = int(round(readTemp())) # round and return as integer -- no decimal 

    # Loop through a number of applied biases.
    for detbias in range(startbias, endbias+1, stepbias):
        # The applied reverse detector bias = (dsub-vreset), invert to get dsub
        detsub = dd.vreset + detbias
        dd.dsub = detsub

        # Set the object name for this data with the bias in the name.
        rd.object = "residual_%dK_%dmV_Test1"%(CurrTemp, detbias)
        rd.gc = 'Taking residual image data.'
        
        
        # Take some dark images first
        print ' Taking 100 dark images'
        rd.itime = 550
        for blah in range(100):
            srun()

        # Repeat test 1 16 times
        for tests in range(0, 16, 1):
            rd.lc = 'This is the light image data for test 1 (1 s).'
            filterBase.set("7.1")
            rd.nsamp = 1
            rd.itime = 1000
            srun()
            rd.lc = 'This is the dark image data for test 1 (1 s).'
            filterBase.set("cds")
            rd.itime = 550
            for blah in range(200):
                srun()
            print '  Now waiting between testings...'
            rd.itime = 3000
            crun()
            WaitAndBurst()

        # Set the object name for this data with the bias in the name.
        rd.object = "residual_%dK_%dmV_Test2"%(CurrTemp, detbias)
        rd.gc = 'Taking residual image data.'
        
        
        # Take some dark images first
        print ' Taking 100 dark images'
        rd.itime = 550
        for blah in range(100):
            srun()

        # Repeat test 2 16 times
        for tests in range(0, 16, 1):
            rd.lc = 'This is the light image data for test 2 (3 s).'
            filterBase.set("7.1")
            rd.nsamp = 1
            rd.itime = 3000
            srun()
            rd.lc = 'This is the dark image data for test 2 (3 s).'
            filterBase.set("cds")
            rd.itime = 550
            for blah in range(200):
                srun()
            print '  Now waiting between testings...'
            rd.itime = 3000
            crun()
            WaitAndBurst()

        
        # Set the object name for this data with the bias in the name.
        rd.object = "residual_%dK_%dmV_Test3"%(CurrTemp, detbias)
        rd.gc = 'Taking residual image data.'
        
        
        # Take some dark images first
        print ' Taking 100 dark images'
        rd.itime = 550
        for blah in range(100):
            srun()

        # Repeat test 3 16 times
        for tests in range(0, 16, 1):
            rd.lc = 'This is the light image data for test 3 (28 s).'
            filterBase.set("7.1")
            rd.nsamp = 1
            rd.itime = 28000
            srun()
            rd.lc = 'This is the dark image data for test 3 (28 s).'
            filterBase.set("cds")
            rd.itime = 550
            for blah in range(200):
                srun()
            print '  Now waiting between testings...'
            rd.itime = 3000
            crun()
            WaitAndBurst()        


        # Set the object name for this data with the bias in the name.
        rd.object = "residual_%dK_%dmV_Test4"%(CurrTemp, detbias)
        rd.gc = 'Taking residual image data.'
        
        
        # Take some dark images first
        print ' Taking 100 dark images'
        rd.itime = 550
        for blah in range(100):
            srun()

        # Repeat test 4 16 times
        for tests in range(0, 16, 1):
            rd.lc = 'This is the light image data for test 4 (60 s).'
            filterBase.set("7.1")
            rd.nsamp = 1
            rd.itime = 60000
            srun()
            rd.lc = 'This is the dark image data for test 4 (60 s).'
            filterBase.set("cds")
            rd.itime = 550
            for blah in range(200):
                srun()
            print '  Now waiting between testings...'
            rd.itime = 3000
            crun()
            WaitAndBurst()

        # Set the object name for this data with the bias in the name.
        rd.object = "residual_%dK_%dmV_Test5"%(CurrTemp, detbias)
        rd.gc = 'Taking residual image data.'
        
        
        # Take some dark images first
        print ' Taking 100 dark images'
        rd.itime = 550
        for blah in range(100):
            srun()

        # Repeat test 5 16 times
        for tests in range(0, 16, 1):
            rd.lc = 'This is the light image data for test 5 (5 minutes).'
            filterBase.set("7.1")
            rd.nsamp = 1
            rd.itime = 300000
            srun()
            rd.lc = 'This is the dark image data for test 5 (5 minutes).'
            filterBase.set("cds")
            rd.itime = 550
            for blah in range(200):
                srun()
            print '  Now waiting between testings...'
            rd.itime = 3000
            crun()
            WaitAndBurst()


WaitAndBurst()  # wait some time, then stop crun
ResidualVsBias()  # just taking darks at this one temp

print 'Finished taking your data'
dd.voffset = OrigOffset
rd.lc = ''
rd.gc = ''
rd.nrow = fullrow
rd.ncol = fullcol
rd.nrowskip = 0
rd.ncolskip = 0
rd.nsamp = 1
rd.itime = fullitime
sscan()

crun()
