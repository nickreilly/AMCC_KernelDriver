"""
nsr_filtercheck.py - a script to run in pydsp.
This is a script to run through a list of filters to check their 
signal levels
"""
from run import rd
import numpy
import xdir
import pyfits
import filterBase
import time
import numpy as np
import ls332


def nap(waittime):
    rd.nsamp = 1
    print('Napping!')
    initial_time = time.time()
    while True:
        sscan()
        time.sleep(0.001)
        now_time = time.time()
        if (now_time - initial_time) > waittime:
            print 'Waking up!'
            return

def set_array_vars(array_type):
    if array_type == 'H2RG':
        rd.object = 'test'
        rd.nrow = 2048
        rd.nrowskip = 0
        rd.ncol = 2048
        rd.itime = 11000
        rd.nsamp = 1
        rd.gc = ''
        rd.lc = ''

class NC1_chars:
    def __init__(self):
        sub_rows = 256
        sub_start_row = 650 # Pinhole Dimensions
        sub_end_row = 1650 # Pinhole Dimensions
        full_start = 0
        full_end = 2048
        nrowskips_sub = np.arange(sub_start_row, sub_end_row, sub_rows)
        nrowskips = np.arange(full_start, full_end, sub_rows)

        self.mission_itime = 1500
        self.mission_samples = 18
        self.sub_rows = sub_rows
        self.full_skips = nrowskips
        self.small_skips = nrowskips_sub
        self.sub_start_row = sub_start_row
        self.sub_end_row = sub_end_row
        self.detbias = 250

def nc1_sutr(num_ramps, row_start=0, row_end=2048, number_garbage_frames=10):
    nc1_chars = NC1_chars()
    nskips = np.arange(row_start, row_end, nc1_chars.sub_rows)
    ynums = nskips[-1] + nc1_chars.sub_rows
    rd.gc = 'NC1 Style SUTR ramps:' + str(num_ramps) + ' at each subarray, ynums:' + str(ynums)
    for i_sub, nrowskip in enumerate(nskips):
        print('Starting Garbage Frames')
        for i_garbage in range(number_garbage_frames):
            rd.nrow = nc1_chars.sub_rows + 10
            rd.nrowskip = nrowskip - 5
            rd.itime = nc1_chars.mission_itime
            rd.nsamp = 1
            sscan()
        for ramp in range(num_ramps):
            # for ramps in range(numramps):
            print('Starting subarray ' + str(i_sub) + ' Ramp num: ' + str(ramp))
            rd.lc = 'Ramp number:' + str(ramp) + ' of ' + str(num_ramps)
            rd.nrow = nc1_chars.sub_rows
            rd.nrowskip = nrowskip
            rd.nsamp = nc1_chars.mission_samples # num_samples_list[i_filter]
            sutr()
            # rd.nsamp = 1
    set_array_vars('H2RG')
    
#########################
burst()
time.sleep(2)
burst()
time.sleep(2)
burst()

nc1_chars = NC1_chars()

#filter_list = ["11.6", "8.6", "5.8", "m\'", "l\'", "3.3", "8.8", "7.1", "cds"] # full list
#filter_list = ["3.3", "5.8",  "l\'", "m\'","8.8", "7.1", "8.6", "11.6", "cds"] # In order
#filter_list = ["cds"]

#wavelength_list = np.arange(4300, 6301, 100) # All 21 wavelengths, each takes about 160 min

wavelength_list = [4400, 5400]

presleep_time = 15 * 60 # seconds
num_images = 50
garbage_frames = 50


# The applied reverse detector bias = (dsub-vreset), invert to get dsub
detsub = dd.vreset + nc1_chars.detbias
dd.dsub = detsub
CurrTemp = int(round(ls332.readTemp()))

# Set the object name for this data with the bias in the name.
rd.object = 'test'
rd.gc = 'Taking a nap before data.'
filterBase.set('cds')
set_array_vars('H2RG')

nap(presleep_time)


rd.gc = 'Taking a tour of the filter wheel using all filters and itimes!'
'''
# For discrete filters
# for i_filt, filter_name in enumerate(filter_list):
    rd.object = "MTFsubarray_%dK_%dmV"%(CurrTemp, nc1_chars.detbias)
    # Move filter wheel.
    filterBase.set(filter_name)
    
    nc1_sutr(num_images, row_start=0, row_end=2048, number_garbage_frames=garbage_frames)
    
    print("Finished this color!")
    # print(str(filter_name))
    nap(10 * 60)
'''

# For cvf filters
for i_filt, wavelength in enumerate(wavelength_list):
    rd.object = "MTFsubarray_%dK_%dmV"%(CurrTemp, nc1_chars.detbias)
    # Move filter wheel.
    filterBase.set(("cvfII", wavelength)) 

    nc1_sutr(num_images, row_start=0, row_end=2048, number_garbage_frames=garbage_frames)

    print("Finished this color!")
    # print(str(wavelength))
    nap(10 * 60)
    
    
rd.object = "MTFsubarray_%dK_%dmV"%(CurrTemp, nc1_chars.detbias)
# Move filter wheel.
filterBase.set("cds")

nc1_sutr(num_images, row_start=0, row_end=2048, number_garbage_frames=garbage_frames)

print("Finished this color!")
# print(str(filter_name))
nap(10 * 60)



print("Finished taking data!")

set_array_vars('H2RG')
filterBase.set("cds") # put array back in dark

sscan()
sscan()

crun2()


# Time Calculation
# 1 full array using mission SUTR 18 - Guess
# (50 * ((1.5 * 18) + num_garbage * (3))) / 60

# actually took 5 full frame acquisitions from 
# 9:53.36 to 10:30.23 - 36 minutes 47 seconds
# so 7 minutes 21 seconds per ramp on average
# with 50 garbage frames inbetween each subarray
# each series of garbage frames takes
# 2 minutes, 40 seconds
# that leaves 4 minutes 42 seconds per ramp

# 4.689 * 50 + 2.66 = 237 minutes per filter, 
# close to 4 hours per color!


