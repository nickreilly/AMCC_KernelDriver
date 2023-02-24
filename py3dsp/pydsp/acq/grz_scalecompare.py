from run import rd
import numpy
import xdir
import pyfits
import filterBase
import time
import ls332

print 'What detector bias do you want to use?'
detbias = int(raw_input())

print 'How many subarrays would you like?'
numsub=int(raw_input())

print 'How many rows in the full image?'
rowend = int(raw_input())
skip=int(rowend/numsub)

print 'What integration time would you like to use?'
subarrayitime = int(raw_input())
fullarrayitime = int(subarrayitime)

print 'How many subarray images would you like to take?'
numframes=int(raw_input())

print 'How many full frame images would you like to take?'
numfull=int(raw_input())

print 'How long do you want to wait for temperature stability before starting?'
print '    (enter in seconds)'
waittime = int(raw_input())

def SetFullArraySize():
  rd.nsamp = 1
  rd.itime = fullarrayitime
  rd.nrow = rowend
  rd.nrowskip = rowstart
  rd.ncol = colend
  rd.ncolskip = colskip

def SetSubArraySize():
  rd.nsamp = 1
  rd.itime = subarrayitime
  rd.nrow = numrows
  rd.nrowskip = rowskip
  rd.ncol = numcols
  rd.ncolskip = colskip

def get_mosaic_img():
  global oldrowblock
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

rowstart=0

def get_mosaic_img_reverse():
  # put comment in header
  rd.gc = 'sub-array reads were mosaiced backwards to make full image'
  rd.itime = subarrayitime
  rd.nrow = skip # Set the number of rows to do a sub-array read.
  for rowskip in range(rowend-skip, rowstart-1, -skip):
    rd.nrowskip = rowskip

    sscan() # Take an image, but do not save
    sscan() # do two because we want a better reset than just once.
    # recreate file name (duplicate code from runrun)
    scanfilename = xdir.get_objpath() + "/" + rd.bufferflag + ".fits"
    scanfile = pyfits.open(scanfilename)
    # These are FITS files. Get both the data and header.
    scandata = scanfile[0].data
    imgheader = scanfile[0].header

    if rd.nrowskip == rowend-skip:
        newrowblock = scandata # can't concat on empty.
    else:
        oldrowblock = newrowblock
        newrowblock = numpy.concatenate((scandata,oldrowblock), axis=0)
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

time.sleep(waittime)
burst()
time.sleep(5)
burst()
time.sleep(5)
burst()
tmps()
CurrTemp = int(round(ls332.readTemp())) # round and return as integer -- no decimal. Changed it to match the command from temp modulation code
# Set the object name for this data with the bias in the name.
rd.object = "scalecompare_%dK_%dmV"%(CurrTemp, detbias)
rd.nsamp=1
rd.itime = fullarrayitime
rd.nrow=rowend
rd.nrowskip=0
detsub = dd.vreset + detbias
dd.dsub = detsub
for blah in range(100): #Take throwaway data to check noise
    sscan()
 
# Loop through a number of applied biases.


# The applied reverse detector bias = (dsub-vreset), invert to get dsub



rd.nsamp = 1
rd.itime = fullarrayitime
rd.nrow = rowend
rd.nrowskip = 0

for blah in range(numframes):
    get_mosaic_img()


rd.nsamp = 1
rd.itime = fullarrayitime
rd.nrow = rowend
rd.nrowskip = 0
for blah in range(numfull):
    srun()


filterBase.set("cds")
print 'Finished taking your data'
rd.lc = ''
rd.gc = ''
rd.nrowskip = 0
rd.ncolskip = 0
rd.nrow=2048
rd.nsamp = 1
rd.itime = fullarrayitime
sscan()

crun()
