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

burst()
time.sleep(2)
burst()
time.sleep(2)

# A subarray to just center illumination pattern
rd.object = "FilterCheck"
rd.nrow = 850
rd.nrowskip = 750
rd.gc = 'Taking a tour of the filter wheel using all filters and itimes!'

box_width = 50

cent_minrow, cent_mincol = 523, 1083
dark_minrow, dark_mincol = 403, 987
leftarc_minrow, leftarc_mincol = 195, 397
rightarc_minrow, rightarc_mincol = 161, 1747

filter_list = ["cds", "11.6", "8.6", "5.8", "m\'", "l\'", "3.3", "8.8", "7.1"]
int_times_discrete = [5000, 11000, 27000, 60000]  

wavelength_list = np.arange(4300, 6001, 100)
int_times_cvf = [5000, 11000, 27000]

def calculate_info(scandata, imgheader):
    # get the mean, should be center of array
    center_value = np.median(scandata[(cent_minrow):(cent_minrow+box_width),(cent_mincol):(cent_mincol+box_width)].flatten())
    dark_value = np.median(scandata[(dark_minrow):(dark_minrow+box_width),(dark_mincol):(dark_mincol+box_width)].flatten())

    leftarc = np.median(scandata[(leftarc_minrow):(leftarc_minrow+box_width),(leftarc_mincol):(leftarc_mincol+box_width)].flatten())
    rightarc = np.median(scandata[(rightarc_minrow):(rightarc_minrow+box_width),(rightarc_mincol):(rightarc_mincol+box_width)].flatten())
    
    result_string = ("filter: " + str(imgheader['FILTER']) +
        ", itime: " + str(imgheader['ITIME']) +
        ", wavelength: " + str(imgheader['LAMBDA']) +
        ", bandwidth: " + str(imgheader['BNDWIDTH']) +
        ", trans: " + str(imgheader['TRANSMIT']) +
        ", center: " + str(round(center_value)) +
        ", dark: " + str(round(dark_value)) +
        ", left: " + str(round(leftarc)) +
        ", right: " + str(round(rightarc)) + "\n")
    # print(result_string)
        
    return result_string

# For discrete filters
for i_filt, filter_name in enumerate(filter_list):
    # Move filter wheel.
    filterBase.set(filter_name)
    for i_time, int_time in enumerate(int_times_discrete):
        # Open a file, using "a" = append mode
        wfile = open(xdir.get_objpath() + "/"+rd['object']+".txt", "a")
        rd.itime = int_time
        sscan() # Take an image, but do not save

        # recreate file name (duplicate code from runrun)
        scanfilename = xdir.get_objpath() + "/" + rd.bufferflag + ".fits"
        scanfile = pyfits.open(scanfilename)
        
        # These are FITS files. Get both the data and header.
        scandata = scanfile[0].data
        imgheader = scanfile[0].header
        result_string = calculate_info(scandata, imgheader)
        
        # Write data and close fits file!
        wfile.write(result_string)
        scanfile.close()
        wfile.close()
        
# For cvf filters
for i_filt, wavelength in enumerate(wavelength_list):
    # Move filter wheel.
    filterBase.set(("cvfII", wavelength)) 
    for i_time, int_time in enumerate(int_times_cvf):
        # Open a file, using "a" = append mode
        wfile = open(xdir.get_objpath() + "/"+rd['object']+".txt", "a")
        rd.itime = int_time
        sscan() # Take an image, but do not save

        # recreate file name (duplicate code from runrun)
        scanfilename = xdir.get_objpath() + "/" + rd.bufferflag + ".fits"
        scanfile = pyfits.open(scanfilename)
        
        # These are FITS files. Get both the data and header.
        scandata = scanfile[0].data
        imgheader = scanfile[0].header
        result_string = calculate_info(scandata, imgheader)
        
        # Write data and close fits file!
        wfile.write(result_string)
        scanfile.close()
        wfile.close()

rd.nrowskip = 0
rd.nrow = 2048
rd.itime = 11000
rd.object('test')
filterBase.set("cds") # put array back in dark
sscan()
sscan()
print("Finished taking data!")
crun2()

