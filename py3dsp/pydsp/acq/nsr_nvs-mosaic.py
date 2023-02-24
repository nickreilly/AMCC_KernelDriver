from run import rd
from pydsp import cloop
from numpy import *
import xdir
import pyfits
import time
import filterBase
import numpy
import ls332

def get_new_mosaic_img():
  # put comment in header
  rd.gc = 'sub-array reads were mosaiced to make full image'
  rd.lc = ''
  rd.itime = subarrayitime
  rd.nrow = skip+10 # Set the number of rows to do a sub-array read.
  newrowstart=rowstart-skip/2
  toofar=0
  if newrowstart <0:
    newrowstart=0
  for rowskip in range(newrowstart, rowend - skip/2, skip/2):
    if rowskip + skip + 10> fullrowend:
      rd.nrow=fullrowend-rowskip
      toofar=1

    rd.nrowskip = rowskip

    sscan() # Take an image, but do not save
    sscan() # do two because we want a better reset than just once.
    # recreate file name (duplicate code from runrun)
    scanfilename = xdir.get_objpath() + "/" + rd.bufferflag + ".fits"
    scanfile = pyfits.open(scanfilename)
    # These are FITS files. Get both the data and header.
    scandata = scanfile[0].data
    imgheader = scanfile[0].header

    if rd.nrowskip == newrowstart:
        if newrowstart==0:
            newrowblock = scandata[:-10,:] # can't concat on empty.
        elif toofar==1:
            newrowblock=scandata[skip/2:,:]
        else:
            newrowblock=scandata[skip/2:-10,:]
    else:
        if toofar==1:
            oldrowblock = newrowblock
            newrowblock = numpy.concatenate((oldrowblock, scandata[skip/2:,:]), axis=0)            
        else:
            oldrowblock = newrowblock
            newrowblock = numpy.concatenate((oldrowblock, scandata[skip/2:-10,:]), axis=0)
    scanfile.close()
  image = newrowblock

  fitsobj = pyfits.HDUList() # new fitsfile object
  hdu = pyfits.PrimaryHDU()
  fitsobj.append(hdu) # and
  hdu.data = image
  fitsobj[0].header = imgheader
  outputfilename = xdir.get_nextobjfilename() + ".fits" # Get the filename
  rd.objnum = xdir.objnum + 1 # increase object number for next filename
  print 'Writing mosaic image to \n %s'%outputfilename
  fitsobj.writeto(outputfilename)
  #pyfits.writeto(outputfilename, newrowblock)
  # Clear comment
  rd.gc = ''

  rd.itime = fullarrayitime
  rd.nrow = fullrowend
  rd.nrowskip = 0

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

print 'What is the full array integration time to be (enter in milliseconds)?'
int_time = int(raw_input())

print 'How long do you want to wait for temperature stability before starting?'
print '    (enter in seconds)'
waittime = int(raw_input())

print "How many PAIRS of images do you want to take at each signal level?"
NumImgPairs = int(raw_input())
# NumImgPairs = 2

print 'What temp do you want to change the temp to after the experiment?'
nexttemp = int(raw_input())

subarrayitime = 5500
skip=1024
rowstart=0
rowend = 2048
fullrowend=2048
fullarrayitime=11000


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


# Loop through a number of applied biases.
for detbias in range(startbias, endbias+1, stepbias):
    # The applied reverse detector bias = (dsub-vreset), invert to get dsub
    detsub = dd.vreset + detbias
    dd.dsub = detsub
    # Set the object name for this data with the bias in the name.
    #rd.object = "cap_perpix_%dK_%dmV"%(CurrTemp, detbias)
    rd.object = "capacitance_mosaic_%dK_%dmV"%(CurrTemp, detbias)
    rd.itime = int_time
    #sscan() # just take some throw-away data to help new bias settle.
    #sscan()
    # Vary wavelength to get successively larger number of photons
    # At wavelengths <4300 and >7900, the edge cuts off light.
    for wavelen in range(8000, 9001, 100):
        filterBase.set(("cvfIII", wavelen)) # move the filter wheel.
        for zzzz in range(0, NumImgPairs):
            # Need to setup file name before doing [ped,s,b]run
            runfilename1 = xdir.get_nextobjfilename() + ".fits"
            get_new_mosaic_img() # Take our images. 
            runfilename2 = xdir.get_nextobjfilename() + ".fits"
            get_new_mosaic_img()


print 'Finished taking Capacitance data'
rd.lc = ''
filterBase.set("cds")  # put detector back in dark
rd.itime = int_time
sscan() # take an image to reset array -- helps clear residual images.

crun2()
ls332.setSetpointTemp(nexttemp)
