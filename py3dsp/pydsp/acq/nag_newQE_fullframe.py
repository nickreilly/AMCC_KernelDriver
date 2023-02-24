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

print 'What bias do you want to use to measure the QE?'
detbias=int(raw_input())

print 'What are you looking at?'
print '1) Black Cloth'
print '2) Envelope Window'
print '3) Liquid Nitrogen'
target=int(raw_input())

if target ==1:
	print 'What is the temperature of the cloth (C)?'
	clothtemp=float(raw_input())
	print 'What is the temperature of the room (F)?'
	roomtemp=float(raw_input())
	rd.lc = 'Cloth Temp = ' + str(clothtemp) + ' C. Room Temp = ' + str(roomtemp) + ' F'


print 'How long do you want to wait for temperature stability?'
waittime=int(raw_input())

time.sleep(waittime)
burst()
time.sleep(5)
burst()
time.sleep(5)
burst()


detsub = dd.vreset + detbias
dd.dsub = detsub

tmps()
CurrTemp = int(round(ls332.readTemp()))
if target==1:
	rd.object = "QE_Cloth_fullframe_%dK_%dmV"%(CurrTemp, detbias)
elif target==2:
	rd.object = "QE_Window_fullframe_%dK_%dmV"%(CurrTemp, detbias)
elif target==3:
	rd.object = "QE_LN2_fullframe_%dK_%dmV"%(CurrTemp, detbias)

rowstart=0
rowend = 2048
fullrowend=2048
fullarrayitime=11000

for blah in range(20):
	sscan()

subarrayitime = 500
skip=64
for wavelen in range(12000, 7899, -100):
	filterBase.set(("cvfIII", wavelen))
	get_new_mosaic_img()

subarrayitime = 3000
skip=512
for wavelen in range(8000, 4299, -100):
	filterBase.set(("cvfII", wavelen))
	get_new_mosaic_img()
  
subarrayitime = 500
skip=64
filterBase.set("7.1")
get_new_mosaic_img()

filterBase.set("8.8")
get_new_mosaic_img()

filterBase.set("5.8")
get_new_mosaic_img()

filterBase.set("8.6")
get_new_mosaic_img()

filterBase.set("11.6")
get_new_mosaic_img()

rd.itime=60000
filterBase.set("3.3")
srun()

rd.itime=11000
filterBase.set("l'")
srun()

filterBase.set("cds")
srun()

rd.gc=''
rd.lc=''

crun2()
