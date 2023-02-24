from run import rd
import numpy
import xdir
import pyfits
import filterBase
import time
import ls332
import dv

skip = 64
subarrayitime = 450
fullarrayitime = 11000
rowstart = 900
rowend = 1200

fullrowend = 2048
fullrowstart = 0

def get_new_mosaic_img():
  # put comment in header
  rd.gc = 'sub-array reads were mosaiced to make full image'
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
  hdu.data = image.astype(int)
  fitsobj[0].header = imgheader
  outputfilename = xdir.get_nextobjfilename() + ".fits" # Get the filename
  rd.objnum = xdir.objnum + 1 # increase object number for next filename
  print 'Writing mosaic image to \n %s'%outputfilename
  fitsobj.writeto(outputfilename)
  #pyfits.writeto(outputfilename, newrowblock)
  # Clear comment
  rd.gc = ''
  dv.load_src(outputfilename)

  rd.itime = fullarrayitime
  rd.nrow = fullrowend
  rd.nrowskip = 0
