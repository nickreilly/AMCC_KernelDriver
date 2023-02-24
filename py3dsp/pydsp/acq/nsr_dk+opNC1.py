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

def run_op():
    

print 'What is the starting detector bias you want to do for dark current?'
startbias1 = int(raw_input())

print 'What is the ending detector bias you want to do for dark current?'
endbias1 = int(raw_input())

print 'What is the starting detector bias you want to do for OPERABILITY?'
startbias = int(raw_input())

print 'What is the ending detector bias you want to do for OPERABILITY?'
endbias = int(raw_input())

print 'In what increments do you want to change the detector bias?'
stepbias = int(raw_input())

#print 'How many samples do you want up a normal ramp?  (18 for NC1 flight like!)'
samples_orig = 18

print 'How long do you want to wait for temperature stability before starting?'
print '(enter in seconds)'
waittime = int(raw_input())

print 'Do you want to change the temp after the experiment?'
change_temp = str(raw_input())

if change_temp == 'y':
    print 'To what?'
    next_temp = int(raw_input())


rd.object = 'test'
subarrayitime = 1500
skip = 256  
rowstart = 0
rowend = 2048
fullrowend = 2048
fullarrayitime = 11000
filterBase.set("cds")

time.sleep(waittime)
burst()
time.sleep(5)
burst()
time.sleep(5)
burst()

tmps()
CurrTemp = int(round(ls332.readTemp()))

# Loop through a number of applied biases.
for detbias in range(startbias1, endbias1 + 1, stepbias):
    # The applied reverse detector bias = (dsub-vreset), invert to get dsub
    detsub = dd.vreset + detbias
    dd.dsub = detsub

    # Set the object name for this data with the bias in the name.
    rd.object = "dk_%dK_%dmV"%(CurrTemp, detbias)
    #rd.object = "dk_%sK_%dmV"%(str(CurrTemp), detbias)
    rd.gc = 'Taking dark+light data. No reset between SUTR sets.'
    #rd.lc = 'Post proton-radiation exposure'
    rd.lc = 'This is the start of the dark data.'

    rd.nsamp = 1
    rd.itime = fullarrayitime
    rd.nrow = fullrowend
    for blah in range(20):
        sscan()  # just take some throw-away data to help new bias settle.

    # Take some samples in the dark
    rd.nsamp = 200  # was 2000
    # rd.itime = fullitime
    sutr()
    # Turn off the reset clock so that we still do non-destructive reads.
    dd.resetnhi = 0
    # Also, move the Voffset voltage to keep things on scale with A/D converters.
    # dd.voffset = OrigOffset - extraoffset
    # Take some samples in the light
    ## filterBase.set("8.6")
    filterBase.set("m'") # Maybe try m' if this does not work well.
    rd.nsamp = 50    # was 100
    rd.itime = 60000  # was 60 sec with ND filter.
    rd.gc = 'Taking dark+light data. No reset between SUTR sets.'
    rd.lc = 'This is the light exposed data.'
    sutr()
    # Move filter back to dark and take more samples
    filterBase.set("cds")
    rd.nsamp = 200
    rd.itime = fullarrayitime
    rd.gc = 'Taking dark+light data. No reset between SUTR sets.'
    rd.lc = 'This is the data back in the dark after light exposure.'
    sutr()
    # Turn on the upper rail of reset clock (allows clocking reset, not continuous)
    dd.resetnhi = 3300


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

if change_temp == 'y':
    ls332.setSetpointTemp(next_temp)

print 'All done taking data!'

crun2()
