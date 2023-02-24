"""
cmc_center-FW.py - a script to run in pydsp.
Center the filter wheel.  Take some data, get means, find half power points

To use, type: execuser filename
where filename is this file's name, but WITHOUT the .py
"""

from run import rd
import numpy
import xdir
import pyfits
import filterBase
import time
import numpy as np

stepsize = 0.5

# # Filter Wheel 2
#filter_positions = [680.5, 763.5, 805.5, 944.5] # NC1
filter_positions = [597.25, 638.75, 847.25, 888.75, 944.5] # NC2

# # Filter Wheel 5
#filter_positions = [891.75]# 836, 864, 891.75] # NC1
#filter_positions = [836, 864, 891.75] # NC2

fwp_window_size = 15

larger_window = [1000, 128]  #[Start, size]
larger_window_size = 50
centerline_start = 100
min_row, max_row = int(larger_window[1]), int(larger_window[1] + larger_window_size)
min_col, max_col = int(larger_window[0]), int(larger_window[0] + larger_window_size)

rd.object = 'center-FW'

# Get the image size for later use with statistics box
imgcol = rd.ncol
imgrow = rd.nrow

#for newFWP in np.arange(FWstart, FWend + stepsize, stepsize):
for i_filter in filter_positions:
    FWstart = i_filter - fwp_window_size
    FWend = i_filter + fwp_window_size
    for newFWP in np.arange(FWstart, FWend + stepsize, stepsize):
        # Open a file, using "a" = append mode
        wfile = open(xdir.get_objpath() + "/"+rd['object']+".txt", "a")
        
        # Move filter wheel.
        rd.fw = newFWP
        #srun()
        sscan() # Take an image, but do not save
        
        # recreate file name (duplicate code from runrun)
        scanfilename = xdir.get_objpath() + "/" + rd.bufferflag + ".fits"
        scanfile = pyfits.open(scanfilename)
        
        # These are FITS files. Get both the data and header.
        scandata = scanfile[0].data.astype('float64')
        #imgheader = scanfile[0].header
        

        window = np.mean(scandata[min_row:max_row, min_col:max_col])
        centerline = np.mean(scandata[centerline_start:centerline_start+100, :])

        # get the mean, should be center of array
        centervalue = scandata[(imgrow/2-25):(imgrow/2+25),(imgcol/2-25):(imgcol/2+25)].mean()
        largercenter = window #.mean()
        
        leftval = scandata[(imgrow/2-25):(imgrow/2+25),(imgcol/4-25):(imgcol/4+25)].mean()
        rightval = scandata[(imgrow/2-25):(imgrow/2+25),(imgcol*3/4-25):(imgcol*3/4+25)].mean()
        
        # Write the file
        wfile.write("%s\t%s \t%s \t%s center: \t%s centerline: \t%s\n" % (rd.fw, centervalue, leftval, rightval, largercenter, centerline)) 
        scanfile.close()
        wfile.close()

"""
filterBase.set("cds") # put array back in dark
rd.itime = 11000
rd.object = 'test'
sscan()
crun2()
"""
