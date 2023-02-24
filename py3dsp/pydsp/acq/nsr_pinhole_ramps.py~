'''
This script will take SUTR ramps of varying lengths at different illumination levels
intended to be used with the pinhole mask on surface, but just series of ramps

To use, make sure the experimental inputs are correct
The filteres_used and num_samples_list are important- 
    will take SUTR (num_samples_list[i]) at 
    each wavelength in the list!

'''

from run import rd
import filterBase
import time

# define functions
def nap(waittime):
    rd.nsamp = 1
    print 'Napping!'
    initial_time = time.time()
    while True:
        sscan()
        time.sleep(0.001)
        now_time = time.time()
        if (now_time - initial_time) > waittime:
            print 'Waking up!'
            break


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


#############################
# Experimental inputs

startbias = 150        # bias of interest
endbias = 151          # 
stepbias = 100         # 
waittime = (30 * 60)   # Naptime before experiment!
# fwpoffset = 20       # How far away from filter do you want the extra pedestal frame taken at?
# numsamples = 300     # How many samples up the ramp do you want?
numramps = 3           # How many ramps at each illumination level do you want?
array_type = 'H2RG'    # What type of array is this?
# subarrays = 8
number_garbage_frames = 10
do_subarray = False

# # Full List
# filters_used = ["cds", "l'", "5.8", "cvfII_4500", "cvfII_4800", "cvfII_5100", "3.3", "m'"]
# num_samples_list = [100, 100, 100, 250, 250, 250, 300, 500]

#filters_used = ["cds", "l'", "5.8", "cvfII_4500", "cvfII_4800", "cvfII_5100"]#, "3.3", "m'"]
#num_samples_list = [100, 100, 100, 250, 250, 250]#, 300, 500]
filters_used = ["m'"]  # , "3.3"]
num_samples_list = [500]  # , 300]


'''
# For testing purposes!
filters_used = ["cvfII_4500", "5.8"] # "cds",]
num_samples_list = [2, 2]
number_garbage_frames = 1
numramps = 1
waittime = 23
'''

burst()
time.sleep(5)
burst()
time.sleep(5)
burst()


time.sleep(2)
set_array_vars(array_type)

CurrTemp = int(round(readTemp()))  # round and return as integer -- no decimal

rd.object = 'test'
nap(waittime)

# When taking linearity data, the important stuff happens at starvation and 
# saturation, i.e. at the beginning and end of the data set. 
# Loop through a number of applied biases.

for detbias in range(startbias, endbias + 1, stepbias):
    detsub = dd.vreset + detbias
    dd.dsub = detsub
    
    for i_filter, filter_now in enumerate(filters_used):
        rd.object = "PinholeRamps_%dK_%dmV_%s"%(CurrTemp, detbias, filter_now)
        print 'Starting Garbage Frames at filter ' + str(filter_now)
        if filter_now[:3] == 'cvf':
            filter_cvf_name = str(filter_now.split('_')[0])
            filter_cvf_wave = int(filter_now.split('_')[-1])
            filterBase.set((filter_cvf_name, filter_cvf_wave))
        else:
            filterBase.set((filter_now))
        for i_garbage in range(number_garbage_frames):
            rd.nsamp = 1
            sscan()
        for ramps in range(numramps):
            print 'Starting filter ' + str(filter_now)
            rd.nsamp = num_samples_list[i_filter]
            sutr()
            rd.nsamp = 1

print 'Finished Pinhole Ramps'

filterBase.set('cds')
set_array_vars(array_type)
crun2()

