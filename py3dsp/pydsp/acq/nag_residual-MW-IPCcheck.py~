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


whichSCA ='H2RG'

fullitime = 11000 # full array read time
longitime = 25000
fullrow = 2048
fullcol = 2048
subarray = 256
subskip = 896

startbias = 250 #int(raw_input())

endbias = 251 # int(raw_input())

stepbias = 100 # int(raw_input())


OrigOffset = dd.voffset

waittime = 0 # int(raw_input())

naptime = 2 * 60 * 60 # Units of seconds


def nap(waittime):
    print 'Napping!'
    initial_time = time.time()
    while True:
        sscan()
        time.sleep(0.001)
        now_time = time.time()
        if (now_time - initial_time) > waittime:
            break

def exposure(itime, filt, garbage_frames):
    filterBase.set(filt)
    for i_stable in range(10):
        sscan()
    pedrun()  # take the pedestal frame before exposure
    initial_time = time.time()
    dd.resetnhi = 0
    while True:
        sscan()
        time.sleep(0.001)
        now_time = time.time()
        if (now_time - initial_time) * 1000 > itime:
            break
    pedrun()
    filterBase.set("cds")
    if garbage_frames > 0:
        for i_garbage in range(garbage_frames):
            sscan()
    pedrun()  # take the final image after exposure
    dd.resetnhi = 3300



# def ResidualVsBias():
# Set up subarray reads so that we can read out fast and meet NEOCam reqt
rd.nrow = 2048
rd.nrowskip = 0
## Added ncols to make array read faster
rd.ncol = 2048
rd.ncolskip = 0
garbage_frames_scale = 0  # This is the scale that the number of garbage frames after exposure increases


nap(naptime)

# What is our current temperature?
tmps()
CurrTemp = int(round(readTemp()))  # round and return as integer -- no decimal 

# Loop through a number of applied biases.
for detbias in range(startbias, endbias+1, stepbias):
    # The applied reverse detector bias = (dsub-vreset), invert to get dsub
    detsub = dd.vreset + detbias
    dd.dsub = detsub

    rd.object = "CrossTestAR_%dK_%dmV"%(CurrTemp, detbias)
    rd.gc = 'Garbage Frames'
    rd.lc = ''
    rd.itime = 11000
    rd.nsamp = 1
    filterBase.set(('cds'))


    print ' Taking 2 garbage images'
    for blah in range(2):
        sscan()


    rd.gc = 'Taking saturated image data.'
    rd.lc = 'Dark Images for exposure'

    print ' Taking 50 dark images'
    for blah in range(25):
        srun()


    for i_garbage in range(5):
        total_garbage = 0 # int(i_garbage * garbage_frames_scale)
        rd.lc = 'This is the light image data with ' + str(total_garbage) + ' Garbage Frames.'
        exposure(5 * 60 * 1000, "5.8", total_garbage)
        rd.lc = 'This is the first shortest possible reset.'# are images immediately after the first reset.'
        '''
        rd.ncol = 4
        rd.itime = 43
        pedrun()
        rd.itime = 11000
        rd.ncol = 2048
        '''
        rd.lc = 'These are images immediately after the fast reset.'
        rd.nrow = 300
        rd.nrowskip = 690
        rd.itime = 1600
        rd.nsamp = 6
        sutr()
        sutr()
        rd.nsamp = 1
        rd.lc = 'This is just normal cds images with resets.'
        for i in range(20):
            srun()

        print '  Now waiting between testings...'
        nap(120)


print 'Finished taking your data'
rd.object = 'test'
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

crun2()
