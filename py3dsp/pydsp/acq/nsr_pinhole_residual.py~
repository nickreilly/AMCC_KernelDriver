"""
nsr_pinhole_residual.py - a script to run in pydsp.
    This is to study any residual signal from single pixel illuminations

to use, type: execuser filename
where filename is this file's name, but WITHOUT the .py
"""

from run import rd
import time
import filterBase
import ls332
import numpy as np

whichSCA ='H2RG'

full_itime = 11000 # full array read time
long_itime = 25000
full_row = 2048
full_col = 2048

sub_array = 50   # How big of a sub array do you want?
sub_skip = 975   # Where do you want this subarray to start?
sub_itime = 300  # What is the shortest itime you can get for this subarray?

start_bias = 250  # int(raw_input())
end_bias = 251  # int(raw_input())
step_bias = 100  # int(raw_input())
bias_list = np.arange(start_bias, end_bias + 1, step_bias)

filter_list = ["5.8"]
exposure_list = [(60 * 60)]#(27), (5 * 60), (30 * 60)] # in seconds
number_exposures = 8

orig_offset = dd.voffset

wait_time = 0 # int(raw_input())

nap_time = 30 * 60 # Units of seconds

def nap(wait_time):
    print('Napping!')
    initial_time = time.time()
    while True:
        sscan()
        time.sleep(0.001)
        now_time = time.time()
        if (now_time - initial_time) > wait_time:
            break

def exposure(itime, filt):  # , garbage_frames):
    # This function is intended to replace the deadtime when taking long 
    # exposures with a consistent reading throughout the exposure.
    # This is to mitigate the issue having the array cool off and have
    # negative values immediately after the exosure!
    filterBase.set(filt)
    for i_stable in range(10):
        sscan()
    initial_time = time.time()
    pedrun()  # take the pedestal frame before exposure
    dd.resetnhi = 0
    while True:
        sscan()
        time.sleep(0.001)
        now_time = time.time()
        if (now_time - initial_time) > itime:
            break
    pedrun()
    filterBase.set("cds")
    dd.resetnhi = 3300

# Set up subarray reads so that we can read out fast and meet NEOCam reqt
## Added ncols to make array read faster
rd.nrow = full_row
rd.nrowskip = 0
rd.ncol = full_col
rd.ncolskip = 0
rd.itime = full_itime

garbage_frames_scale = 0  # This is the scale that the number of garbage frames after exposure increases

nap(nap_time)

# What is our current temperature?
tmps()
current_temp = int(round(readTemp()))  # round and return as integer -- no decimal 

# Loop through a number of applied biases.
for i_bias, bias in enumerate(bias_list):
    # The applied reverse detector bias = (dsub-vreset), invert to get dsub
    detsub = dd.vreset + bias
    dd.dsub = detsub

    rd.object = "PinholeResidual_%dK_%dmV"%(current_temp, bias)
    rd.gc = 'Garbage Frames'
    rd.lc = ''
    rd.itime = full_itime
    rd.nsamp = 1
    filterBase.set(('cds'))
    rd.nrow = sub_array
    rd.nrowskip = sub_skip
    rd.itime = sub_itime

    print(' Taking garbage images')
    for blah in range(50):
        sscan()

    rd.gc = 'Taking residual image data.'
    rd.lc = 'Dark Images for exposure'

    '''
    print('Taking 50 dark images')
    for blah in range(50):
        srun()
    '''

    # for i_garbage in range(5):
    for i_filter, filter_used in enumerate(filter_list):
        for i_exposure, exposure_time in enumerate(exposure_list):
            for i_iter in range(number_exposures):
                rd.lc = 'This is the light image data'
                print('Starting Exposure!')
                exposure(exposure_time, filter_used)
                rd.lc = 'This is the first shortest possible reset.'# are images immediately after the first reset.'

                rd.ncol = 4
                rd.itime = 43 # I think 43 is as fast as this additional reset can handle!
                pedrun()
                # rd.itime = 11000
                rd.ncol = full_col

                rd.lc = 'These are images immediately after the fast reset.'
                rd.nrow = sub_array
                rd.nrowskip = sub_skip
                rd.itime = sub_itime
                rd.nsamp = 6
                sutr()
                sutr()
                rd.nsamp = 1
                rd.lc = 'This is just normal cds images with resets.'
                for i in range(50):
                    srun()

                print('...Now waiting between testings...')
                nap(120)


print('Finished taking your data')
rd.object = 'test'
dd.voffset = orig_offset
rd.lc = ''
rd.gc = ''
rd.nrowskip = 0
rd.ncolskip = 0
rd.nrow = full_row
rd.ncol = full_col
rd.nsamp = 1
rd.itime = full_itime
sscan()

crun2()
