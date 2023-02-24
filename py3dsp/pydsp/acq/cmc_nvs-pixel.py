"""
cmc_nvs-pixel.py - a script to run in pydsp.
    
    We are taking data to measure the capacitance of a detector array.
    This is the version to use when you want to measure the capacitance on
    a pixel-by-pixel basis, i.e. when you have a LWIR array that has many
    non-uniform pixels and there are many high dark current pixels.

    This aquires a series of images for either changing wavelength or
    changing integration times.  
    Later, the user will process these data on a pixel-by-pixel basis to 
    extract the noise squared and signal values, and then plot to get a slope. 

to use, type: execuser filename
where filename is this file's name, but WITHOUT the .py
"""

from run import rd
from pydsp import cloop
from numpy import *
import xdir
import pyfits
import time
import filterBase


print 'Which method do you want to use to get changing fluence:\
       \n  1) Changing wavelength (preferred method) \
       \n  2) Changing integration time'
method = int(raw_input())

print 'What is the starting detector bias you want to do?'
startbias = int(raw_input())

print 'What is the ending detector bias you want to do?'
endbias = int(raw_input())

print 'In what increments do you want to change the detector bias?'
stepbias = int(raw_input())

print 'What do you want the integration time to be (enter in milliseconds)?'
int_time = int(raw_input())

print 'How long do you want to wait for temperature stability before starting?'
print '    (enter in seconds)'
waittime = int(raw_input())


# A small number (2 or 4) is needed for doing box-mean version of capacitance 
# calculation. But a larger number (50) is needed to calculate per-pixel 
# capacitance.  Always do this in pairs for compatibility with the 
# difference-box-mean method.  Also, the calculation of stats within this
# program requires PAIRS of images!

#print "How many PAIRS of images do you want to take at each signal level?"
#NumImgPairs = int(raw_input())
NumImgPairs = 25

print "Do you want this program to write statistics to a file on these data?\
       \n Yes  (Note: program not smart enough to reject CR or hot pixels)\
       \n No   (User processes data later)"
query_stats = str(raw_input())
if query_stats == 'Yes' or query_stats == 'Y' or query_stats == 'yes' or query_stats == 'y':
    do_stats = True
else :
    do_stats = False


time.sleep(waittime)
burst()
time.sleep(5)
burst()
time.sleep(5)


print 'taking noise squared vs. signal data...'

rd.lc = 'taking noise squared vs. signal data for capacitance'
rd.nsamp = 1

# What is our current temperature?
tmps()
#print rd.pre_temp
#print dd.temp
# rd.pre_temp may be old -- from last time sscan or srun was taken.
# dd.temp sometimes has space that int() doesn't like -- thinks it is string 
CurrTemp = int(round(readTemp())) # round and return as integer -- no decimal

# Subroutine for writing statistics to a file
def write_my_stats(runfilename1, runfilename2):
    runfile1 = pyfits.open(runfilename1)
    # These are FITS files. Get the data and header.
    rundata1 = runfile1[0].data
    numcol = runfile1[0].header['NAXIS1']
    numrow = runfile1[0].header['NAXIS2']

    runfile2 = pyfits.open(runfilename2)
    rundata2 = runfile2[0].data
    #
    # subtract images
    subimage=rundata1 - rundata2
    # 
    # Write some statistics. [start row, end row, start col, end col]
    # This is more of a pain for the user to enter with prompts.  So do some
    # automatic boxes in 5 locations, based upon array size.
    box1=[int(numrow/2)-25,int(numrow/2)+25,int(numcol/2)-25,int(numcol/2)+25]
    box2=[int(numrow/4)-25,int(numrow/4)+25,int(numcol/4)-25,int(numcol/4)+25]
    box3=[int(numrow/4)-25,int(numrow/4)+25,int(numcol*3/4)-25,int(numcol*3/4)+25]
    box4=[int(numrow*3/4)-25,int(numrow*3/4)+25,int(numcol/4)-25,int(numcol/4)+25]
    box5=[int(numrow*3/4)-25,int(numrow*3/4)+25,int(numcol*3/4)-25,int(numcol*3/4)+25]
    # OR hard code: 
    #box1=[480,530,480,530]
    #box2=[320,370,940,990]
    #box3=[525,575,900,950]
    #box4=[200,250,700,750]
    #box5=[450,500,700,750]
    # These are mean signal values and noise squared.
    value1 =  rundata1[box1[0]:box1[1], box1[2]:box1[3]].mean()
    noise1 = ((subimage[box1[0]:box1[1], box1[2]:box1[3]].stddev())/sqrt(2.))**2
    value2 =  rundata1[box2[0]:box2[1], box2[2]:box2[3]].mean()
    noise2 = ((subimage[box2[0]:box2[1], box2[2]:box2[3]].stddev())/sqrt(2.))**2
    value3 =  rundata1[box3[0]:box3[1], box3[2]:box3[3]].mean()
    noise3 = ((subimage[box3[0]:box3[1], box3[2]:box3[3]].stddev())/sqrt(2.))**2
    value4 =  rundata1[box4[0]:box4[1], box4[2]:box4[3]].mean()
    noise4 = ((subimage[box4[0]:box4[1], box4[2]:box4[3]].stddev())/sqrt(2.))**2
    value5 =  rundata1[box5[0]:box5[1], box5[2]:box5[3]].mean()
    noise5 = ((subimage[box5[0]:box5[1], box5[2]:box5[3]].stddev())/sqrt(2.))**2
    #
    wfile.write("%s\t" % value1)
    wfile.write("%s\t" % noise1)
    wfile.write("%s\t" % value2)
    wfile.write("%s\t" % noise2)
    wfile.write("%s\t" % value3)
    wfile.write("%s\t" % noise3)
    wfile.write("%s\t" % value4)
    wfile.write("%s\t" % noise4)
    wfile.write("%s\t" % value5)
    wfile.write("%s\t" % noise5)
    wfile.write("\n")
    runfile1.close()
    runfile2.close()
    #
    print value1, noise1, value2, noise2, value3, noise3, value4, noise4, value5, noise5
    return

# Loop through a number of applied biases.
for detbias in range(startbias, endbias+1, stepbias):
    # The applied reverse detector bias = (dsub-vreset), invert to get dsub
    detsub = dd.vreset + detbias
    dd.dsub = detsub
    # Set the object name for this data with the bias in the name.
    #rd.object = "cap_perpix_%dK_%dmV"%(CurrTemp, detbias)
    rd.object = "capacitance_%dK_%dmV"%(CurrTemp, detbias)
    rd.itime = int_time
    sscan() # just take some throw-away data to help new bias settle.
    sscan()
    if do_stats == True:
        # Open a file, using "a" = append mode. This is where stats are written
        wfile = open(xdir.get_objpath() + "/" + rd['object'] + ".txt", "a")
    if method == 1 :
        # Vary wavelength to get successively larger number of photons
        # At wavelengths <4300 and >7900, the edge cuts off light.
        for wavelen in range(4300, 6301, 100):
            filterBase.set(("cvfII", wavelen)) # move the filter wheel.
            for zzzz in range(0, NumImgPairs):
                # Need to setup file name before doing [ped,s,b]run
                runfilename1 = xdir.get_nextobjfilename() + ".fits"
                srun() # Take our images. 
                runfilename2 = xdir.get_nextobjfilename() + ".fits"
                srun()
                if do_stats == True:
                    write_my_stats(runfilename1, runfilename2)
    elif method == 2 :
        # Vary integration time to get successively larger number of photons
        for inttime in range(5500, 10001, 500):
            # set the integration time 
            rd.itime = inttime           
            for zzzz in range(0, NumImgPairs):
                runfilename1 = xdir.get_nextobjfilename() + ".fits"
                srun() # Take our images. 
                runfilename2 = xdir.get_nextobjfilename() + ".fits"
                srun()
                if do_stats == True:
                    write_my_stats(runfilename1, runfilename2)
    if do_stats == True:
        wfile.close()

print 'Finished taking Capacitance data'
rd.lc = ''
filterBase.set("cds")  # put detector back in dark
rd.itime = int_time
sscan() # take an image to reset array -- helps clear residual images.

crun2()
