"""
cmc_linearity-SUTR.py - a script to run in pydsp.
    
Aquires images in SUTR mode.  The user should find a suitable filter that gives
a flux which is not too high. Then the user should determine how many samples to
do in the SUTR in order to reach saturation for the majority of the pixels.  
You should determine this for the highest bias that you will measure.  Thus at
lower biases, the number of samples will still be sufficient to saturate the pixels.

to use, type: execuser cmc_linearity-SUTR
"""

from run import rd
import filterBase
import time

# define functions
def nap(waittime):
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

startbias = 250        # bias of interest
endbias = 251          # 
stepbias = 100         # 
waittime = 600         # Naptime before experiment!
fwpoffset = 20         # How far away from filter do you want the extra pedestal frame taken at?
# numsamples = 300       # How many samples up the ramp do you want?
numramps = 1           # How many ramps at each illumination level do you want?
array_type = 'H2RG'    # What type of array is this?
subarrays = 1

filters_used = ["7.1", "cvfII_5800"]
num_samples_list = [20, 20]

time.sleep(2)

CurrTemp = int(round(readTemp()))  # round and return as integer -- no decimal

nap(44)

# When taking linearity data, the important stuff happens at starvation and 
# saturation, i.e. at the beginning and end of the data set. 
# Loop through a number of applied biases.

for detbias in range(startbias, endbias + 1, stepbias):
    detsub = dd.vreset + detbias
    dd.dsub = detsub
    for ramps in range(0, numramps):
        for i_filter, filter_now in enumerate(filters_used):
            subheight = int(2048 / subarrays)
            for i_subarray in range(subarrays):
                # The applied reverse detector bias = (dsub-vreset), invert to get dsub

                set_array_vars(array_type)  # For nap between filters
                nap(20)
                rd.nsamp = 1

                # Set the object name for this data with the bias in the name.
                rd.object = "LinearitySubarray_%dK_%dmV_%s"%(CurrTemp, detbias, filter_now)
                #filterBase.set(('cds'))
                rd.gc = 'Initial Dark Frames for filter ' + str(filter_now) + ', Subarray ' + str(i_subarray)

                if filter_now[:3] == 'cvf':
                    filter_cvf_name = str(filter_now.split('_')[0])
                    filter_cvf_wave = int(filter_now.split('_')[-1])
                    rd.lc = 'Pedrun near ' + str(filter_now) + ' before ramp'
                    filterBase.set((filter_cvf_name, filter_cvf_wave))
                    rd.lc = 'Pedrun at ' + str(filter_now)
                    nap(20)
                    pedrun()

                else:
                    filterBase.set((filter_now))
                    rd.lc = 'Pedrun at ' + str(filter_now)
                    nap(20)
                    pedrun()

                # We are going to take SUTR data.
                rd.nsamp = num_samples_list[i_filter]
                sutr()
                # rd.nsamp = 2
                # sutr()

print 'Finished taking linearity data'

filterBase.set('cds')
set_array_vars(array_type)
crun2()
