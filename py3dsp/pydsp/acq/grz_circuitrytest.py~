
from run import rd
from pydsp import cloop
from numpy import *
import xdir
import pyfits
import time
import filterBase
import numpy



def get_mosaic_img():
  global subarrayitime, skip
  # put comment in header
  rd.gc = 'sub-array reads were mosaiced to make full image'
  rd.itime = subarrayitime
  rd.nrow = skip # Set the number of rows to do a sub-array read.
  for rowskip in range(rowstart, rowend, skip):
    rd.nrowskip = rowskip

    sscan() # Take an image, but do not save
    sscan() # do two because we want a better reset than just once.
    # recreate file name (duplicate code from runrun)
    scanfilename = xdir.get_objpath() + "/" + rd.bufferflag + ".fits"
    scanfile = pyfits.open(scanfilename)
    # These are FITS files. Get both the data and header.
    scandata = scanfile[0].data
    imgheader = scanfile[0].header

    if rd.nrowskip == rowstart:
        newrowblock = scandata # can't concat on empty.
    else:
        oldrowblock = newrowblock
        newrowblock = numpy.concatenate((oldrowblock, scandata), axis=0)
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
  rd.nrow = rowend
  rd.nrowskip = 0

rd.object='CircuitryTestCap'
rd.itime=11000
rd.nrow=2049
rd.nrowskip=0
rowstart=0
rowend=2049
fullarrayitime=11000
rd.nsamp=200

for blah in range(1):
	sutr()

rd.nsamp=1
for blah in range(5):
	srun()

rd.nsamp=1
subarrayitime=3000
skip=256
for blah in range(5):
	get_mosaic_img()
rd.nsamp=30
for blah in range(2):
	sutr()

rd.nsamp=1
rd.nrow=2048
