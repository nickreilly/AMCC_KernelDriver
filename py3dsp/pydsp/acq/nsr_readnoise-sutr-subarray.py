"""
cmc_readnoise-sutr.py - a script to run in pydsp.
    
    script to acquire a sequence of SUTR images, which may be used to 
    calculate the read noise on a pixel-by-pixel basis.
    
to use, type: execuser cmc_readnoise-sutr
"""
from run import rd
#import xdir
import pyfits
import filterBase
import time
import ls332

print 'MAKE SURE IT IS IN THE LOW NOISE STATE'
print 'Also, turn off CRUN/CRUN2 before running'

print 'H1RG or H2RG?'
whichSCA = str(raw_input())

print 'What is the starting detector bias you want to do?'
startbias = int(raw_input())

print 'What is the ending detector bias you want to do?'
endbias = int(raw_input())

print 'In what increments do you want to change the detector bias?'
stepbias = int(raw_input())

print 'How long do you want to wait for temperature stability before starting?'
print '    (enter in seconds)'
waittime = int(raw_input())  

print 'How many throw away images do you want to take?'
print '    (Original was 12)'
throwaway = int(raw_input()) 
#throwaway = 50

print 'Do you want to change the temp after?'
change_temp = str(raw_input())

if change_temp == 'y':
    print 'To what?'
    next_temp = int(raw_input())


numsub=8
subheight=int(2048/numsub)


'''
time.sleep(waittime)
burst()
time.sleep(5)
burst()
time.sleep(5)
burst()
'''

# HAWAII-1RG
if whichSCA == 'H1RG' :
    rd.nrow = 1024
    rd.ncol = 1024
    rd.itime = 5800

# HAWAII-2RG
if whichSCA == 'H2RG' :
    rd.nrow = 2048
    rd.ncol = 2048
    rd.itime = 11000

# SB-304
if whichSCA == 'SB304' :
    rd.nrow = 2048
    rd.ncol = 2048
    rd.itime = 11000

#filterBase.set("cds")

# What is our current temperatunumsubre?
tmps()
CurrTemp = int(round(readTemp())) # round and return as integer -- no decimal


for detbias in range(startbias, endbias+1, stepbias):
    # The applied reverse detector bias = (dsub-vreset), invert to get dsub
    detsub = dd.vreset + detbias
    dd.dsub = detsub
    # Set the object name for this data with the temp & bias in the name
    rd.object = "readnoise%dK_%dmV_subarray"%(CurrTemp, detbias)
    rd.nsamp = 1
    # Settle detector with new bias by reseting+reading array.
    for blah in range(throwaway):
        sscan()

    num_garbage = 20
    sample_size = 36
    for subarray in range(0, numsub):
        rd.nrowskip = subarray*(2048/(numsub))
        rd.nrow = subheight
        rd.itime = 1500
        rd.nsamp = 1
        for garbage in range(num_garbage):
            sscan()
        rd.nsamp = sample_size
        for kimages in range(64):
            sutr()

    #rd.nsamp = 36
    #for kimages in range(0, 64):
    #    sutr()

rd.nsamp = 1
rd.itime = 11000
rd.nrowskip = 0
rd.nrow = 2048
if change_temp ==y:
    ls332.setSetpointTemp(next_temp)
print 'Finished taking data'
crun2()
