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

def mission_sutr(num_ramps):
    mission_itime = 1500
    mission_samples = 18
    sub_rows = 256
    sub_start_row = 0
    sub_end_row = 2048
    nrowskips = np.arange(sub_start_row, sub_end_row, sub_rows)
    for i_sub, nrowskip in enumerate(nrowskips):
        print 'Starting subarray ' + str(i_sub)
        for i_garbage in range(5):
            rd.nrow = sub_rows
            rd.nrowskip = nrowskip
            rd.itime = mission_itime
            rd.nsamp = 1
            sscan()
        for ramp in range(num_ramps):
            rd.nsamp = mission_samples
            sutr()
            rd.nsamp = 1
    rd.nrow = 2048
    rd.nrowskip = 0
    rd.itime = 11000
    rd.nsamp = 1


detbias = 250
presleep_time = 30 * 60 # seconds
num_images = 50
garbage_frames = 10
fullarray_size = [2048, 0] 
subarray_size = [900, 700]
do_sutr = False

burst()
time.sleep(5)
burst()
time.sleep(5)
burst()

# The applied reverse detector bias = (dsub-vreset), invert to get dsub
detsub = dd.vreset + detbias
dd.dsub = detsub
CurrTemp = int(round(ls332.readTemp()))

# Set the object name for this data with the bias in the name.
rd.object = "MTF_%dK_%dmV"%(CurrTemp, detbias)
rd.gc = 'Taking a nap before data.'
filterBase.set('cds')
rd.nrow = fullarray_size[0]
rd.nrowskip = fullarray_size[1]
rd.itime = 11000
crun2()
time.sleep(presleep_time)

burst()
time.sleep(5)
burst()
time.sleep(5)
burst()


rd.gc = 'Taking a tour of the filter wheel using all filters and itimes!'

#filter_list = ["cds", "11.6", "8.6", "5.8", "m\'", "l\'", "3.3", "8.8", "7.1"] # full list
filter_list = ["cds"]#"5.8", "l\'", "3.3"] # Check results of filtercheck for good filters/itimes to use
# int_times_discrete = [[5000, 11000, 27000, 60000]] # for CDS

int_times_discrete = [5000, 11000, 27000, 60000] 

#wavelength_list = np.arange(4300, 6301, 100) # All 21 wavelengths, each takes about 160 min

#wavelength_list = [4300]
#wavelength_list = np.arange(4400, 4700, 100)
#wavelength_list = np.arange(4700, 5000, 100)  
#wavelength_list = np.arange(5000, 5200, 100)
#wavelength_list = np.arange(5200, 5500, 100)
#wavelength_list = np.arange(5500, 5900, 100)
#wavelength_list = np.arange(5900, 6300, 100)
wavelength_list = [6300]
int_times_cvf = [5000, 11000, 27000, 60000]  # 84 conditions - > 95 conditions total! 

# For discrete filters
for i_filt, filter_name in enumerate(filter_list):
    # Move filter wheel.
    filterBase.set(filter_name)
    if not do_sutr:
        for i_time, int_time in enumerate(int_times_discrete):
            rd.itime = int_time
            if int_time < 11000:
                rd.nrow = subarray_size[0]
                rd.nrowskip = subarray_size[1]
            else:
                rd.nrow = fullarray_size[0]
                rd.nrowskip = fullarray_size[1]
            for i_garbage in range(garbage_frames):
                sscan()
            for i_im in range(num_images):
                rd.lc = 'Image ' + str(i_im + 1) + ' of ' + str(num_images) + ' for this condition'
                srun() 
    if do_sutr:
        mission_sutr(num_images)
    print("Finished this color!")
    print(str(filter_name))
    print(str(int_time))
"""
# For cvf filters
for i_filt, wavelength in enumerate(wavelength_list):
    # Move filter wheel.
    filterBase.set(("cvfII", wavelength)) 
    if not do_sutr:
        for i_time, int_time in enumerate(int_times_cvf):
            rd.itime = int_time
            if int_time < 11000:
                rd.nrow = subarray_size[0]
                rd.nrowskip = subarray_size[1]
            else:
                rd.nrow = fullarray_size[0]
                rd.nrowskip = fullarray_size[1]
            for i_garbage in range(garbage_frames):
                sscan()
            for i_im in range(num_images):
                rd.lc = 'Image ' + str(i_im + 1) + ' of ' + str(num_images) + ' for this condition'
                srun() 
    if do_sutr:
        mission_sutr(num_images)

    print("Finished this color!")
    print(str(wavelength))
    print(str(int_time))
"""
            
print("Finished taking data!")

rd.nrow = 2048
rd.nrowskip = 0
rd.itime = 11000
rd.object= 'test'
filterBase.set("cds") # put array back in dark

sscan()
sscan()

crun2()
