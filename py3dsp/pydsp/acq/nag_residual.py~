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
burst()
time.sleep(2)
burst()
time.sleep(2)
burst()

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

print 'NC1 or NC2?'
bandpass_used = str(raw_input())

print 'Do you want to use the mission definition or bright source by time?'
print '(Type "mission" or "time" or "LargerReset")\n\n'
print '        mission      |  time'
print 'Test 1: 3.3  28s     |  5.8  1s'
print 'Test 2: 5.8  28s     |  5.8  3s'
print 'Test 3: l\'   3s      |  5.8  28s'
print 'Test 4: 5.8  3s      |  5.8  60s'
print 'Test 5: 5.8  1Hr     |  5.8  1Hr'
test_type = str(raw_input())

OrigOffset = dd.voffset

print 'How long do you want to wait between tests due to previous bright source residual decay before starting?'
print 'Try 60?  (enter in seconds)'
waittime = int(raw_input())

print 'How long do you want to wait before starting?'
naptime = int(raw_input())

tests_taken = ['Test1', 'Test2', 'Test3', 'Test4', 'Test5']
repeats = [16, 16, 16, 16, 8]

repeats = [5, 5, 5, 5, 2]

##practice
#tests_taken = ['Test1_practice', 'Test2_practice', 'Test3_practice', 'Test4_practice', 'Test5_practice']
#repeats = [1, 1, 1, 1, 1]

if 'mission' in test_type:
    if bandpass_used == 'NC1':
        test_filters = ["3.3", "5.8", "l'", "5.8", "5.8"]
        test_exposures = [28000, 28000, 3000, 3000, 60*60*1000]
    if bandpass_used == 'NC2':
        test_filters = [('cvfIII',10000), "8.8", "l'", "8.8", "8.8"]
        test_exposures = [28000, 28000, 3000, 3000, 60*60*1000]

if 'time' in test_type:
    test_filters = ["5.8", "5.8", "5.8", "5.8", "5.8"]
    test_exposures = [1000, 3000, 28000, 60000, 60*60*1000]

if 'LargerReset' in test_type:
    test_filters = ['5.8']
    test_exposures = [10 * 60 * 1000]
    tests_taken = ['Test20Rows']
    repeats = [3]


if test_type != 'time' and test_type != 'mission' and test_type != 'LargerReset':
    test_filters = ['5.8']
    test_exposures = [2 * 60 * 1000]
    tests_taken = ['Test']
    repeats = [1]


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
    #if type(filt) == tuple:
    #    filterBase.set((filt[0], filt[1]))
    #else:
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
    filterBase.set("cds")
    if garbage_frames > 0:
        for i_garbage in range(garbage_frames):
            sscan()
    pedrun()  # take the final image after exposure
    dd.resetnhi = 3300

def small_reset(larger_sub_size):
    initial_skip = rd.nrowskip
    initial_rows = rd.nrow
    initial_cols = rd.ncol
    initial_itime = rd.itime

    rd.nrow = larger_sub_size
    rd.nrowskip = int(initial_skip - ((larger_sub_size - initial_rows) / 2))
    rd.ncol = 4
    rd.itime = 26  # Shortest itime for ~20 rows (14 even defaults to this!)

    reset_time = time.time()
    rd.lc = 'This is the RESET image- reset at ' + str(reset_time)
    srun()

    rd.nrow = initial_rows
    rd.ncol = initial_cols
    rd.nrowskip = initial_skip
    rd.itime = initial_itime

    return reset_time

def full_reset():
    rd.nrowskip=0
    rd.nrow=2048
    rd.itime=11000
    for i_garbage_full in range(5):
        sscan()
    rd.nrow=10
    rd.nrowskip=1020
    rd.itime=550
    for blah in range(20):
        sscan()

nap(naptime)

# def ResidualVsBias():
# Set up subarray reads so that we can read out fast and meet NEOCam reqt
rd.nrow = 10
rd.nrowskip = 1020
## Added ncols to make array read faster
rd.ncol = 2048
rd.ncolskip = 0





# What is our current temperature?
tmps()
CurrTemp = int(round(readTemp()))  # round and return as integer -- no decimal 

# Loop through a number of applied biases.
for detbias in range(startbias, endbias+1, stepbias):
    # The applied reverse detector bias = (dsub-vreset), invert to get dsub
    detsub = dd.vreset + detbias
    dd.dsub = detsub

    for i_test, test_name in enumerate(tests_taken):
        if i_test != 4:
            continue
        rd.object = "residual_%s_%dK_%dmV_%s"%(test_type, CurrTemp, detbias, test_name)
        rd.gc = 'Garbage Frames'
        rd.lc = ''
        rd.itime = 550
        rd.nsamp = 1
        
        print ' Taking 100 garbage images'
        for blah in range(100):
            sscan()

        rd.gc = 'Taking residual image data.'
        rd.lc = 'Dark Images for %s'%(test_name)
        '''

        print ' Taking 100 dark images'
        for blah in range(100):
            srun()
        '''

        for i_exposure in range(0, int(repeats[i_test]), 1):
            #print(test_exposures[i_test], type(test_exposures[i_test]))
            #print(test_filters[i_test], type(test_filters[i_test]))
            full_reset()
            rd.lc = 'This is the light image data for %s (%dms).'%(test_name, test_exposures[i_test])
            exposure(test_exposures[i_test], test_filters[i_test], 0)
            reset_time = small_reset(20)
            delay_time = time.time() - reset_time
            rd.lc = 'This is the FIRST dark image data - delay of ' + str(delay_time)
            # for blah in range(200):
            #     srun()
            rd.nsamp = 2
            sutr()
            rd.nsamp = 1
            rd.lc = 'This is the dark image data for %s (%dms).'%(test_name, test_exposures[i_test])
            for i_ramp in range(200):
                srun()
            print '  Now waiting between testings...'
            nap(waittime)


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
