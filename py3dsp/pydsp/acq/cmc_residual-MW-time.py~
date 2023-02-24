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

print 'How long do you want to wait to start the test for temperature stability?'
sleepytime = int(raw_input())

print 'How many iterations of each test would you like to do?'
print 'Each tests will run about ~4 hours?'
test_iters = int(raw_input())


crun()
time.sleep(sleepytime)

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
        rd.object = "residualTime_%dK_%dmV"%(CurrTemp, detbias)
        rd.gc = 'Taking residual image data.'
        
        
        # Take some dark images first
        print ' Taking 100 dark images'
        rd.itime = 550
        for blah in range(100):
            srun()
        

        tests_required= test_iters

        # Repeat test- increasing by 15 minutes 
        for i_test in range(0, tests_required, 1):
          for i_time in range(2, 5, 1):
            
            naptime = 900 * i_time #this needs to be in seconds
            print 'This is the light image data for itime='+str(naptime)+'s'
            rd.itime = 550 #900000 * i_time
            rd.lc = 'This is the light image data for itime='+str(naptime)+'s'
            filterBase.set("l'") #brighter than l', still kind of close to CDS
            rd.nsamp = 2
            sutr()
            dd.resetnhi = 0
            time.sleep(naptime) 
            sutr()
            rd.lc = 'This is the dark image data for itime='+str(naptime)+'s'
            dd.resetnhi = 3300
            nsamp = 1
            filterBase.set("cds") 
            #rd.itime = 550
            
            for blah in range(200):
                srun()
            print '  Now waiting between testings...'
            rd.itime = 3000
            crun()
            WaitAndBurst()
        
        # Repeat test- increasing by 5 minutes 
        for i_test in range(0, tests_required, 1):
          for i_time in range(1, 6, 1):
            
            naptime = 300 * i_time #this needs to be in seconds
            print 'This is the light image data for itime='+str(naptime)+'s'
            rd.itime = 550 #900000 * i_time
            rd.lc = 'This is the light image data for itime='+str(naptime)+'s'
            filterBase.set("l'") #brighter than l', still kind of close to CDS
            rd.nsamp = 2
            sutr()
            dd.resetnhi = 0
            time.sleep(naptime) 
            sutr()
            rd.lc = 'This is the dark image data for itime='+str(naptime)+'s'
            dd.resetnhi = 3300
            nsamp = 1
            filterBase.set("cds") 
            #rd.itime = 550
            
            for blah in range(200):
                srun()
            print '  Now waiting between testings...'
            rd.itime = 3000
            crun()
            WaitAndBurst()



        # Repeat test- increasing by half second 
        # Do the shortest exposures with a close time interval. 
        for i_test in range(0, tests_required, 1):
           for i_time in range(1, 20, 1):    
            rd.nsamp = 1
            rd.itime = 500 * i_time
            print 'This is the light image data for itime='+str(int((500*i_time)/1000))+'s'
            rd.lc = 'This is the light image data for itime='+str(int((500*i_time)/1000))+'s'
            filterBase.set("l'") #Closest to CDS that gives decent signal

            srun()
            rd.lc = 'This is the dark image data for itime='+str(int((500*i_time)/1000))+'s'
            filterBase.set("cds") #cd2 is much closer than cds
            rd.itime = 550
            for blah in range(200):
                srun()
            print '  Now waiting between testings...'
            rd.itime = 3000
            crun()
            WaitAndBurst()

        # Repeat test- increasing by one second 
        # Do the shortest exposures with a close time interval. 
        for i_test in range(0, tests_required, 1):
           for i_time in range(1, 21, 1):    
            print 'This is the light image data for itime='+str(int((1000*i_time+9000)/1000))+'s'
            rd.nsamp = 1
            rd.itime = 1000 * i_time + 9000
            rd.lc = 'This is the light image data for itime='+str(int((1000*i_time+9000)/1000))+'s'
            filterBase.set("l'") #Closest to CDS that gives decent signal

            srun()
            rd.lc = 'This is the dark image data for itime='+str(int((500*i_time)/1000))+'s'
            filterBase.set("cds") #cd2 is much closer than cds
            rd.itime = 550
            for blah in range(200):
                srun()
            print '  Now waiting between testings...'
            rd.itime = 3000
            crun()
            WaitAndBurst()

        
        # Repeat test- increasing by 30 second 
        for i_test in range(0, tests_required, 1):
          for i_time in range(1, 10, 1):
            print 'This is the light image data for itime='+str(int((30000*i_time)/1000))+'s'
            rd.lc = 'This is the light image data for itime='+str(int((30000*i_time)/1000))+'s'
            filterBase.set("l'") #brighter than l', still kind of close to CDS
            rd.nsamp = 1
            rd.itime = 30000 * i_time
            srun()
            rd.lc = 'This is the dark image data for itime='+str(int((30000*i_time)/1000))+'s'
            filterBase.set("cds") 
            rd.itime = 550
            for blah in range(200):
                srun()
            print '  Now waiting between testings...'
            rd.itime = 3000
            crun()
            WaitAndBurst()

        
        # Repeat test- increasing by five minutes 
        for i_test in range(0, tests_required, 1):
          for i_time in range(1, 6, 1):
            print 'This is the light image data for itime='+str(int((300000*i_time)/1000))+'s'
            rd.lc = 'This is the light image data for itime='+str(int((300000*i_time)/1000))+'s'
            filterBase.set("l'") #brighter than l', still kind of close to CDS
            rd.nsamp = 1
            rd.itime = 30000        # Repeat test- increasing by 15 minutes 
        for i_test in range(0, tests_required, 1):
          for i_time in range(2, 5, 1):
            print 'This is the light image data for itime='+str(int((300000*i_time)/1000))+'s'
            rd.lc = 'This is the light image data for itime='+str(int((300000*i_time)/1000))+'s'
            filterBase.set("l'") #brighter than l', still kind of close to CDS
            rd.nsamp = 1
            rd.itime = 900000 * i_time 
            srun()
            rd.lc = 'This is the dark image data for itime='+str(int((300000*i_time)/1000))+'s'
            filterBase.set("cds") 
            rd.itime = 550
            for blah in range(200):
                srun()
            print '  Now waiting between test[5,ings...'
            rd.itime = 3000
            crun()
            WaitAndBurst() * i_time
            srun()
            rd.lc = 'This is the dark image data for itime='+str(int((300000*i_time)/1000))+'s'
            
            





        
         

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
