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

which_filterwheel = 'FW2'
is_mwir = False

burst()
time.sleep(2)
burst()
time.sleep(2)

# A subarray to just center illumination pattern
rd.object = "FilterCheck"
rd.nrow = 300
rd.nrowskip = 850
rd.gc = 'Taking a tour of the filter wheel using all filters and itimes!'

box_width = 100

cent_minrow, cent_mincol = 150, 1024
dark_minrow, dark_mincol = 100, 100

filter_list = ["cds", "11.6", "8.6", "5.8", "m\'", "l\'", "3.3", "8.8", "7.1"] # FW 2
if which_filterwheel == 'FW5':
    filter_list = ["cds", "m'", "l''", "3.3", "k", "h", "j", "cd2"] # FW 5
int_times_discrete = [3000, 11000]#, 27000, 60000]  


wavelength_list_JH = np.arange(1150, 2250, 100)  
wavelength_list_K = np.arange(1850, 2750, 100)
wavelength_list_L = np.arange(2350, 4550, 200)

wavelength_list_II = np.arange(4300, 8001, 500)
wavelength_list_III = np.arange(7800, 11001, 500)

int_times_cvf = [3000, 11000]

cvfs_used = ["cvfII", "cvfIII"]
cvf_wavelengths_used = [wavelength_list_II, wavelength_list_III]

if is_mwir:
    cvfs_used = ["cvfII"]
    cvf_wavelengths_used = [np.arange(4200, 6001, 200)]

if which_filterwheel == 'FW5':
    cvfs_used = ["cvfJH", "cvfK", "cvfL"]
    cvf_wavelengths_used = [wavelength_list_JH, wavelength_list_K, wavelength_list_L]



def calculate_info(scandata, imgheader):
    # get the mean, should be center of array
    center_value = np.median(scandata[(cent_minrow-box_width):(cent_minrow+box_width),(cent_mincol-box_width):(cent_mincol+box_width)].flatten())
    dark_value = np.median(scandata[(dark_minrow-box_width):(dark_minrow+box_width),(dark_mincol-box_width):(dark_mincol+box_width)].flatten())

    result_string = ("filter: " + str(imgheader['FILTER']) +
        ", itime: " + str(imgheader['ITIME']) +
        ", wavelength: " + str(imgheader['LAMBDA']) +
        ", bandwidth: " + str(imgheader['BNDWIDTH']) +
        ", trans: " + str(imgheader['TRANSMIT']) +
        ", center: " + str(round(center_value)) +
        ", dark: " + str(round(dark_value)) + 
        ", all: " + str(round(np.median(scandata.flatten()))) + 
        ", flux: " + str(round(1000 * center_value / float(imgheader['ITIME']))) + "ADU/s \n")
    # print(result_string)
        
    return result_string
    
base_filename = xdir.get_objpath() + "/" + rd['object'] + '_'



counter = 1
# For discrete filters
for i_filt, filter_name in enumerate(filter_list):
    # Move filter wheel.
    filterBase.set(filter_name)
    for i_time, int_time in enumerate(int_times_discrete):
        # Open a file, using "a" = append mode
        wfile = open(xdir.get_objpath() + "/"+rd['object']+".txt", "a")
        rd.itime = int_time
        #sscan() # Take an image, but do not save
        srun()

        # recreate file name (duplicate code from runrun)
        #scanfilename = xdir.get_objpath() + "/" + rd.bufferflag + ".fits"
        scanfilename = base_filename + str(counter).zfill(3) + '.fits'
        scanfile = pyfits.open(scanfilename)
        
        # These are FITS files. Get both the data and header.
        scandata = scanfile[0].data
        imgheader = scanfile[0].header
        result_string = calculate_info(scandata, imgheader)
        
        # Write data and close fits file!
        wfile.write(result_string)
        scanfile.close()
        wfile.close()
        counter += 1


# For cvf filters
for i_cvf, cvf_name in enumerate(cvfs_used):
    # Move filter wheel
    wavelengths = list(cvf_wavelengths_used[i_cvf])
    for i_wave, wavelength in enumerate(wavelengths):
        filterBase.set((cvf_name, wavelength)) 
        for i_time, int_time in enumerate(int_times_cvf):
            # Open a file, using "a" = append mode
            wfile = open(xdir.get_objpath() + "/"+rd['object']+".txt", "a")
            rd.itime = int_time
            #sscan() # Take an image, but do not save
            srun()

            # recreate file name (duplicate code from runrun)
            # scanfilename = xdir.get_objpath() + "/" + rd.bufferflag + ".fits"
            scanfilename = base_filename + str(counter).zfill(3) + '.fits'
            scanfile = pyfits.open(scanfilename)
            
            # These are FITS files. Get both the data and header.
            scandata = scanfile[0].data
            imgheader = scanfile[0].header
            result_string = calculate_info(scandata, imgheader)
            
            # Write data and close fits file!
            wfile.write(result_string)
            scanfile.close()
            wfile.close()
            
            counter += 1


rd.nrowskip = 0
rd.nrow = 2048
rd.itime = 11000
rd.object = "test"
filterBase.set("cds") # put array back in dark
sscan()
sscan()
print("Finished taking data!")
crun2()

