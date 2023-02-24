from run import rd
import numpy
import xdir
import pyfits
import filterBase
import time
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

print 'What is the starting detector bias you want to do?'
startbias = int(raw_input())

print 'What is the ending detector bias you want to do?'
endbias = int(raw_input())

print 'In what increments do you want to change the detector bias?'
stepbias = int(raw_input())

print 'How long do you want to wait for temperature stability before starting?'
print '    (enter in seconds)'
waittime = int(raw_input())

print 'Do you want to change the temp after the experiment? (y or n)'
change_temp = str(raw_input())

if change_temp == 'y':
    print 'To what temp?'
    new_temp = int(raw_input())

subarrayitime = 3000
skip=512
rowstart=0
rowend = 2048
fullrowend=2048
fullarrayitime=11000
filterBase.set("cds")

time.sleep(waittime)
burst()
time.sleep(5)
burst()
time.sleep(5)
burst()

tmps()
CurrTemp = int(round(ls332.readTemp())) # 

for detbias in range(startbias, endbias+1, stepbias):
    # The applied reverse detector bias = (dsub-vreset), invert to get dsub
    detsub = dd.vreset + detbias
    dd.dsub = detsub

    # Set the object name for this data with the bias in the name.
    rd.object = "Operability_%dK_%dmV"%(CurrTemp, detbias)

    rd.nsamp = 1
    rd.itime = fullarrayitime
    rd.nrow = fullrowend
    for blah in range(10):
        sscan() # just take some throw-away data to help new bias sett

    for blah in range(100): #was 100
        get_new_mosaic_img()

    rd.gc = 'Taking dark+light data. No reset between SUTR sets.'
    rd.lc = 'This is the start of the dark data.'

    rd.nsamp = 1
    rd.itime = fullarrayitime
    rd.nrow = fullrowend
    for blah in range(10):
        sscan() # just take some throw-away data to help new bias settle.
    
    ## Take some samples in the dark
    rd.nsamp = 30   # was 200
    sutr()
    ## Turn off the reset clock so that we still do non-destructive reads.
    dd.resetnhi = 0
    ## Also, move the Voffset voltage to keep things on scale with A/D converters.
    ## Take some samples in the light
    filterBase.set("8.6")
    #filterBase.set("l''")
    rd.nsamp = 20    # was 100
    rd.gc = 'Taking dark+light data. No reset between SUTR sets.'
    rd.lc = 'This is the light exposed data.'
    sutr()
    ## Move filter back to dark and take more samples
    filterBase.set("cds")
    rd.nsamp = 100
    rd.gc = 'Taking dark+light data. No reset between SUTR sets.'
    rd.lc = 'This is the data back in the dark after light exposure.'
    sutr()
    ## Turn on the upper rail of reset clock (allows clocking reset, not continuous)
    dd.resetnhi = 3300

rd.nsamp = 1
if change_temp == 'y':
    ls332.setSetpointTemp(new_temp)
crun2()

