'''
Modeled off of the Read Noise with subarrays
Should take an estimated 4 Hours
'''
from run import rd
import numpy
import xdir
import pyfits
import filterBase
import time
import ls332
from acq.nsr_general import nap, exposure

def WaitAndBurst_tempchange(NextTemp):
    not_at_temp = True
    while not_at_temp:
        tmps()
        current_temp = ls332.readTemp()
        if abs(NextTemp - current_temp) < 0.01:
            # crun()
            nap(300)
            # time.sleep(300)
            # burst()
            # time.sleep(5)
            # burst()
            tmps()
            current_temp = ls332.readTemp()
            if abs(NextTemp - current_temp) < 0.05:
                not_at_temp = False
        else:
            print 'Waiting 10 minutes before test starts!'
            nap(600)
            # crun2()
            # time.sleep(600)
            # burst()
            # time.sleep(5)
            # burst()
            # time.sleep(5)
            # burst()
    #time.sleep(waittime_tempchange) # wait for temperature to stabilize
    # burst()  # stop CRUN mode
    # time.sleep(5)
    # burst()  # sometimes, once is not enough
    # time.sleep(5)
    # burst()  # sometimes, once is not enough
    # time.sleep(5)
    # burst()  # sometimes, once is not enough
    # time.sleep(5)


def do_op():
    for detbias in range(startbias, endbias + 1, stepbias):
        # The applied reverse detector bias = (dsub-vreset), invert to get dsub
        detsub = dd.vreset + detbias
        dd.dsub = detsub

        # Set the object name for this data with the bias in the name.
        rd.object = "OperabilityNC1_%dK_%dmV"%(CurrTemp, detbias)

        rd.nsamp = 1
        rd.itime = fullarrayitime
        rd.nrow = fullrowend
        rd.nrowskip = 0
        for blah in range(20):
            sscan()  # just take some throw-away data to help new bias settle

        ###########################
        # Do SUTR 18 Ramps
        #
        # 50 ramps with 8 subarrays at 1.5s itime.
        # With garbage frames, should take ~3.2 Hours

        num_garbage = 20
        numsub = 8
        subheight = int(2048 / numsub)

        for subarray in range(0, numsub):
            rd.gc = 'Taking Operability Data'
            rd.lc = 'SUTR18- Subarray ' + str(subarray) + ' of ' + str(numsub)
            rd.nrowskip = int(subarray * (subheight))
            rd.nrow = subheight
            rd.itime = 1500
            rd.nsamp = 1
            for garbage in range(num_garbage):
                sscan()
            rd.nsamp = samples_orig
            for kimages in range(50):
                sutr()

        ###########################
        # Do Dark Ramp w/ 11s int time
        #
        # Just a normal SUTR ramp in the dark
        # With 200 frames and 20 garbage frames
        # Will take an estimated 0.73 Hours

        rd.nsamp = 1
        rd.itime = fullarrayitime
        rd.nrow = fullrowend
        rd.nrowskip = 0
        rd.lc = 'SUTR 200 in the dark'
        for blah in range(num_garbage):
            sscan()  # just take some throw-away data to help new bias settle.

        # Take some samples in the dark
        rd.nsamp = 200   # was 2000
        sutr()
        rd.nsamp = 1


print 'What is the starting detector bias you want to do?'
startbias = int(raw_input())

print 'What is the ending detector bias you want to do?'
endbias = int(raw_input())

print 'In what increments do you want to change the detector bias?'
stepbias = int(raw_input())

# print 'How many samples do you want up a normal ramp?  (18 for NC1 flight like!)'
samples_orig = 18

print 'How long do you want to wait for temperature stability before starting?'
print '(enter in seconds)'
waittime = int(raw_input())

print 'After taking data at the current temperature, do you want to change the temperature and take dark+light at the new temperature? (y/n)'
changeTemp = str(raw_input())

if (changeTemp == 'y' or changeTemp == 'Y' or changeTemp == 'yes'):
    print 'What is the FINAL temperature at which we are measuring dark current? Uses python\'s range so choose appropriately'
    FinalTemp = int(raw_input())  # integer because of range in for-loop
    print 'In what increments do you want to change the temperature? (include negative if decreasing!)'
    StepTemp = int(raw_input())
    print 'How long do you want to wait at next temp before starting?'
    print '    (enter in seconds)'
    waittime_tempchange = int(raw_input())

print 'Do you want to change temp after and NOT run another experiment? (y/n)'
justChangeTemp = str(raw_input())

if (justChangeTemp == 'y' or justChangeTemp == 'Y' or justChangeTemp == 'yes'):
    print 'to what? '
    justFinalTemp = int(raw_input())  # integer because of range in for-loop

rd.object = 'test'
subarrayitime = 1500
skip = 256
rowstart = 0
rowend = 2048
fullrowend = 2048
fullarrayitime = 11000
filterBase.set("cds")

tmps()
CurrTemp = int(round(ls332.readTemp()))

if (changeTemp == 'y' or changeTemp == 'Y' or changeTemp == 'yes'):
    # What is our current temperature?
    tmps()
    StartTemp = int(round(ls332.readTemp())) # round and return as integer - no decimal
    for NextTemp in range(StartTemp, FinalTemp+1, StepTemp):
        if NextTemp != StartTemp:
            time.sleep(1)
            ls332.setSetpointTemp(NextTemp) # Tell controller to change temperature
            time.sleep(1)
            WaitAndBurst_tempchange(NextTemp)  # wait some time, then stop crun
            do_op() # Take data at the current temperature
        else:
            nap(waittime)
            # time.sleep(waittime)
            # burst()
            # time.sleep(5)
            # burst()
            # time.sleep(5)
            # burst()
            do_op()
else :
    # burst()
    # time.sleep(2)
    # burst()
    # time.sleep(2)
    # burst()
    rd.object = 'test'
    nap(waittime)
    # crun2()
    # time.sleep(waittime)
    # burst()
    # time.sleep(5)
    # burst()
    # time.sleep(5)
    # burst()
    do_op()

if justChangeTemp == 'y':
    ls332.setSetpointTemp(justFinalTemp)

print 'All done taking data!'

crun2()
