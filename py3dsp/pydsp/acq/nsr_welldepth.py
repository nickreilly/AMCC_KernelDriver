'''
Modeled off of the Read Noise with subarrays
Should take an estimated 4 Hours
'''
from run import rd
import numpy
import xdir
import pyfits
import filterBase
import time
import ls332

'''
I will get to this later!  
def get_new_mosaic_SUTR(nsamp):
    # put comment in header
    rd.gc = 'mosaiced SUTR ramps to make full image'
    rd.lc = ''
    previous_nsamp = rd.nsamp
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

    #sscan() # Take an image, but do not save # Removed for SUTR
    #sscan() # do two because we want a better reset than just once.
    srun()
    srun() # want four initial resets before SUTR ramp to avoid first frame effect
    rd.nsamp = nsamp
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
    rd.nsamp = previous_samp
'''
'''
print 'What is the starting detector bias you want to do?'
startbias = int(raw_input())

print 'What is the ending detector bias you want to do?'
endbias = int(raw_input())

print 'In what increments do you want to change the detector bias?'
stepbias = int(raw_input())

#print 'How many samples do you want up a normal ramp?  (18 for NC1 flight like!)'
samples_orig = 18

print 'How long do you want to wait for temperature stability before starting?'
print '(enter in seconds)'
waittime = int(raw_input())

print 'Do you want to change the temp after the experiment?'
change_temp = str(raw_input())

if change_temp == 'y':
    print 'To what?'
    next_temp = int(raw_input())

'''
rd.object = 'test'
filterBase.set("cds")

time.sleep(600)
burst()
time.sleep(5)
burst()
time.sleep(5)
burst()

tmps()
CurrTemp = int(round(ls332.readTemp()))

wd_biases = [350]

for detbias in wd_biases:
    # The applied reverse detector bias = (dsub-vreset), invert to get dsub
    detsub = dd.vreset + detbias
    dd.dsub = detsub

    # Set the object name for this data with the bias in the name.

    rd.object = "WellDepth_%dK_%dmV"%(CurrTemp, detbias)
    rd.nsamp = 2
    rd.itime = 11000
    rd.gc = 'Taking Well Depth Data.'
    rd.lc = '10 minute separated SUTR2s with no reset between'
    for i_set in range(0, 14400, 600):
        print 'We are on ' + str(i_set) + ' s of 21600s (6 hours)'
        if i_set == 0:
            filterBase.set("cds")
            sutr()
            dd.resetnhi = 0
            filterBase.set("m'")
            time.sleep(600)
        else:
            sutr()
            time.sleep(600)

    rd.object = 'test'
    rd.itime = 11000
    rd.nsamp = 1
    rd.gc = ''
    rd.lc = ''
    dd.resetnhi = 3300
    filterBase.set("cds")
    rd.nsamp = 1
'''
if change_temp == 'y':
    # ls332.setSetpointTemp(next_temp)
    ls332.setSetpointTemp(60)
'''
print 'All done taking data!'

crun2()
