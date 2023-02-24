from run import rd
import numpy
import xdir
import pyfits
import filterBase
import time
import ls332

print 'What is the starting detector bias you want to do?'
startbias = int(raw_input())

print 'What is the ending detector bias you want to do?'
endbias = int(raw_input())

print 'In what increments do you want to change the detector bias?'
stepbias = int(raw_input())

print 'How many rows do you want to read for each sub-array?'
skip = int(raw_input())

print 'How many rows in the full image?'
rowend = int(raw_input())

print 'What integration time would you like for sub-array read?'
subarrayitime = int(raw_input())

print 'What integration time would you like for full array read?'
fullarrayitime = int(raw_input())

print 'How many images would you like to take at each bias?'
numframes=int(raw_input())

print 'How long do you want to wait for temperature stability before starting?'
print '    (enter in seconds)'
waittime = int(raw_input())
rowstart=0


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
print('Change the object back!')
#rd.object = "scalefactor_rrNoRstSkip_%dK"%(CurrTemp) #Change this for each type of trial to identify them!
rd.object="scalefactor_%dK"%(CurrTemp) #Change back to this when done changing things!
rd.nsamp=1
rd.itime = fullarrayitime
rd.nrow=rowend
rd.nrowskip=0

 
# Loop through a number of applied biases.
#setSetpointTemp #This seems random?
for detbias in range(startbias, endbias+1, stepbias):
    # The applied reverse detector bias = (dsub-vreset), invert to get dsub
    detsub = dd.vreset + detbias
    dd.dsub = detsub


    rd.nsamp = 1
    rd.itime = fullarrayitime
    rd.nrow = rowend
    rd.nrowskip = 0
    for blah in range(5):
        sscan() # just take some throw-away data to help new bias settle.
        sscan()
 
    for blah in range(numframes):
        get_mosaic_img()

'''
    filterBase.set("l''")
    for blah in range(numframes):
        get_mosaic_img()
        get_mosaic_img_reverse()
    filterBase.set("8.6")
    for blah in range(numframes):
        get_mosaic_img()
        get_mosaic_img_reverse()
'''

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


crun2()
