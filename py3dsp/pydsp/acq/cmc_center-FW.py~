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

# Start about 20 fw below filter center and end 20 above.
#FWstart = 620
#FWend = 660
print 'What is the starting FW position? (About 20 FWP below filter center)'
FWstart = float(raw_input())
print 'What is the ending FW position? (About 20 FWP above filter center)'
FWend = float(raw_input())
#skip = 0.5 

print 'How long do you want to wait for temperature stability before starting?'
print '    (enter in seconds)'
waittime = int(raw_input()) 

rd.object = 'center-FW'

time.sleep(waittime)
burst()
time.sleep(5)
burst()
time.sleep(5)

# Get the image size for later use with statistics box
imgcol = rd.ncol
imgrow = rd.nrow

## Open a file, using "a" = append mode
#wfile = open(xdir.get_objpath() + "/"+rd['object']+".txt", "a")


# for loops can't do less than 1 (integer) steps.  So, we need to double the
# start and end in range to do FW steps of 0.5  
for newFWP in range(FWstart*2., FWend*2.):
    # Open a file, using "a" = append mode
    wfile = open(xdir.get_objpath() + "/"+rd['object']+".txt", "a")
    # Move filter wheel.
    rd.fw = newFWP/2.  # divide by 2 to get back to correct value.
    sscan() # Take an image, but do not save
    #sscan() # do two because we want a better reset than just once.
    # recreate file name (duplicate code from runrun)
    scanfilename = xdir.get_objpath() + "/" + rd.bufferflag + ".fits"
    scanfile = pyfits.open(scanfilename)
    # These are FITS files. Get both the data and header.
    scandata = scanfile[0].data
    imgheader = scanfile[0].header
    # get the mean, should be center of array
    centervalue = scandata[(imgrow/2-25):(imgrow/2+25),(imgcol/2-25):(imgcol/2+25)].mean()
    largercenter = scandata[(imgrow/2-50):(imgrow/2+50),50:-50].mean()
    ##centervalue = scandata[(imgrow/2-25):(imgrow/2+25),455:505].mean()
    ##centervalue = scandata[100:100+50,(imgcol/2-25):(imgcol/2+25)].mean()
    leftval = scandata[(imgrow/2-25):(imgrow/2+25),(imgcol/4-25):(imgcol/4+25)].mean()
    rightval = scandata[(imgrow/2-25):(imgrow/2+25),(imgcol*3/4-25):(imgcol*3/4+25)].mean()
    #wfile.write("%s\t%s \t%s \t%s \n" % (rd.fw, centervalue, leftval, rightval))
    wfile.write("%s\t%s \t%s \t%s \t%s \n" % (rd.fw, centervalue, leftval, rightval, largercenter)) # bigger last section
    #tmps()
    #CurrTemp = readTemp()
    ##wfile.write("%s\t%s \t%s \n" % (rd.fw, centervalue, CurrTemp))
    scanfile.close()
    #print newFWP/2.
    wfile.close()

#wfile.close()
filterBase.set("cds") # put array back in dark
sscan()
crun2()

