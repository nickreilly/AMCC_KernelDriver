"""
This is a script to run in pydsp.
    
    For THz Gen 3 chips

  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
      IMPORTANT !!!
  This uses the modified scan and run code (version with "t" in front)
  which is in the cmc_runrun.py file.  This must be loaded first.

to use, type: execuser filename (no .py)
"""

from run import rd
import xdir
import numpy
import pyfits
import time
import os
import dv

"""
Look-up tables for Vgs values to use with THz Gen 2 chip
import sys
sys.path.append('/home/dsp/dsp/pydsp/acq/')
from cmc_thzgen2_Vgs import *
"""
# 
bestVgs295K = numpy.array([[477, 476, 450, 478, 473, 433, 448], 
                              [466, 475, 462, 477, 477, 482, 445],
                              [459, 480, 435, 481, 459, 486, 482],
                              [459, 465, 460, 455, 470, 477, 441],
                              [464, 480, 458, 471, 471, 483, 475],
                              [473, 453, 500, 434, 450, 446, 456],
                              [465, 444, 480, 468, 468, 477, 463]])

# This was based upon the high power data taken at 259K
# These values are for the center point of the response peak, which is wrong
bestVgs259K = numpy.array([[483, 477, 453, 478, 472, 431, 453], 
                              [468, 474, 462, 475, 491, 476, 444],
                              [463, 479, 441, 484, 458, 486, 482],
                              [461, 465, 458, 450, 465, 470, 437],
                              [473, 488, 457, 471, 472, 482, 477],
                              [480, 455, 507, 433, 455, 446, 455],
                              [473, 455, 486, 476, 475, 481, 468]])

# This was based upon the lower power data taken at 259K.
# These values are based upon the highest Vgs value for response.
bestVGs259K2 = numpy.array([[494, 491, 478, 490, 484, 450, 465],
                               [482, 490, 478, 495, 510, 495, 465],
                               [475, 491, 460, 496, 480, 502, 498],
                               [472, 478, 472, 466, 482, 490, 455],
                               [480, 497, 466, 482, 482, 494, 489],
                               [488, 470, 518, 455, 464, 462, 466],
                               [480, 465, 492, 485, 480, 486, 475]])

# Similar to above, but using the high power data to find highest Vgs
# prior to drop off of response.
bestVGs259K3 = numpy.array([[483, 481, 460, 481, 480, 439, 457],
                               [472, 481, 471, 481, 501, 490, 457],
                               [467, 488, 451, 491, 468, 492, 492],
                               [468, 472, 468, 454, 475, 484, 447],
                               [476, 496, 464, 479, 480, 490, 484],
                               [481, 462, 518, 443, 461, 457, 462],
                               [472, 457, 486, 475, 478, 482, 468]])

# Calculate the conversion for ADU to mV for the A/D converters and amplifiers 
ADCrange = dd.adrange * 1000.0 # in mV
preampgain = dd.ampgain
ADUtoVolt = ADCrange / ( 2**16 * preampgain)

# Set up some boxes for sub-array reads and/or statistics.
# Seems like Python should allow you to set box1=[0:5,2:10], 
# but instead you need to use a tuple with slice().  
box1 = (slice(0,7),slice(0,7)) # Whole array
box2 = (slice(3,5),slice(3,5)) # middle part.
box3 = (slice(2,3),slice(5,7)) # works when voffset=-550
box4 = (slice(0,2),slice(3,4)) # works when voffset=-100


def gen3on():
    # Biases
    dd.vdd_ring =3300 # should already be set first.
    dd.vdd_rc   =3300 
    dd.vdd_pix  =3300
    dd.vgs      =480
    dd.vrstarry =3300
    dd.vbiasp   =2600
    dd.vbias_sf =700 
    dd.vdd_sf   =3300 
    dd.vload    =0
    dd._vreset  =0
    dd.dsub     =0

    # clock rails..
    dd.rstCSBlo =0
    dd.rstCSBhi =3300
    dd.d1_CSlo  =0
    dd.d1_CShi  =3300
    dd.clk_CSlo =0
    dd.clk_CShi =3300

    dd.rstRSBlo =0
    dd.rstRSBhi =3300
    dd.d1_RSlo  =0
    dd.d1_RShi  =3300
    dd.clk_RSlo =0
    dd.clk_RShi =3300
    # Reset Array = 0 gives full reset of pixels
    dd.RstArrlo =0
    dd.RstArrhi =3300  

    dd.SelfRstl =0
    dd.SelfRsth =0
    dd.VgsOnlo  =0
    dd.VgsOnhi  =3300

def gen3SRon():
    # Self Reset operating voltages
    # Biases
    dd.vdd_ring =3300 # should already be set first.
    dd.vdd_rc   =3300 
    dd.vdd_pix  =3300
    dd.vgs      =0
    dd.vrstarry =2500
    dd.vbiasp   =2600
    dd.vbias_sf =700 
    dd.vdd_sf   =3300 
    dd.vload    =0
    dd._vreset  =0
    dd.dsub     =0

    # clock rails..
    dd.rstCSBlo =0
    dd.rstCSBhi =3300
    dd.d1_CSlo  =0
    dd.d1_CShi  =3300
    dd.clk_CSlo =0
    dd.clk_CShi =3300

    dd.rstRSBlo =0
    dd.rstRSBhi =3300
    dd.d1_RSlo  =0
    dd.d1_RShi  =3300
    dd.clk_RSlo =0
    dd.clk_RShi =3300
    # Reset Array = 0 gives full reset of pixels
    dd.RstArrlo =0
    dd.RstArrhi =3300  

    dd.SelfRstl =0
    dd.SelfRsth =3300
    dd.VgsOnlo  =0
    dd.VgsOnhi  =0


def gettemp():
    # What is our current temperature?
    tmps()
    CurrTemp = int(round(readTemp())) # round and return as integer -- no decimal
    return CurrTemp



def checkoffset(step, whichbox, LowerLimit=-10000, UpperLimit=10000):
    """ Give step in units of mV and whichbox is given like those at 
     the beginning of this program, e.g. box1 = (slice(3,5),slice(3,5))
     The Upper and LowerLimit are the pixel values in ADU within which 
     this routine will aim to get the pedestal level. 
     The limits are set to default if user does not give them.
     Set range higher for this chip.  Output goes down when THz illuminates
    """    
    value = 123000 # just set initial value out of range.
    while value > UpperLimit or value < LowerLimit : 
        tpedscan() # Take a single read after reset (no integration yet)
        # need a delay so that the shutter stays closed for minimum time
        #time.sleep(0.1) # 50ms should be long enough.
        pedfilename = xdir.get_objpath() + "/ped.fits" # file name of above
        pedfile = pyfits.open(pedfilename) # Open the FITS file
        peddata = pedfile[0].data # get the actual data
        value = peddata[whichbox].mean() # take mean in the given "whichbox"
        # adjust voffset as needed
        if value > UpperLimit : 
            dd.voffset = dd.voffset - step
        if value < LowerLimit :
            dd.voffset = dd.voffset + step
        pedfile.close()

def mywriteFITS(data, header, outputfilename):
    # This is a lot easier and simpler in newer versions of pyfits!
    fitsobj = pyfits.HDUList() # new fitsfile object
    hdu = pyfits.PrimaryHDU()
    fitsobj.append(hdu) # and
    hdu.data = data
    fitsobj[0].header = header
    print 'Writing image to \n %s'%outputfilename
    fitsobj.writeto(outputfilename)

def aveimg(srcORped, NumImg):
    """ Take a number of (Ped Or Src) scan images and average them.
     This does NOT write the final image to disk, because sometimes we
     might want to do something different when writing the file, 
     e.g. change the name by adding something to it, do scans or runs.
     NumImg = Number of images to take an average over
    """
    for i in range(NumImg):
        if srcORped == 'ped' :
            # Set the PED image file name
            imgfilename =  xdir.get_objpath() + "/ped.fits"
            # Take data
            tpedscan()
        elif srcORped == 'bkg' :
            imgfilename = xdir.get_objpath() + "/bkg.fits"
            tbscan()
        else :
            imgfilename = xdir.get_objpath() + "/src.fits"
            #imgfilename = xdir.get_nextobjfilename() + ".fits"
            tsscan()
        # need a delay so that the shutter stays closed for minimum time
        #time.sleep(0.1) # 0.1s should be long enough.
        # Open that FITS file.
        imgfile = pyfits.open(imgfilename)
        imgdata = imgfile[0].data
        header = imgfile[0].header
        # As we go through this loop, add all the images together
        if i == 0:
            coaddimg = imgdata
        else : 
            coaddimg += imgdata
        imgfile.close()

    # Divide by number of images (from above "for i in range(NumImg)")
    image = coaddimg / float(NumImg)
    myhistory = 'This image is an average of %d images'%NumImg
    header.add_history(myhistory)
    return image, header

def aveimgrun(srcORped, NumImg):
    image, header = aveimg(srcORped, NumImg)
    outputfilename = xdir.get_nextobjfilename() + ".fits"
    mywriteFITS(image, header, outputfilename)
    rd.objnum = xdir.objnum + 1 # increase object number for next filename
    dv.load_src(outputfilename)

def aveimgscan(srcORped, NumImg):
    image, header = aveimg(srcORped, NumImg)
    if srcORped == 'ped' :
        outputfilename =  xdir.get_objpath() + "/ped.fits"
    elif srcORped == 'bkg' :
        outputfilename = xdir.get_objpath() + "/bkg.fits"
    else :
        outputfilename = xdir.get_objpath() + "/src.fits"
    if os.access(outputfilename,6) : # Does the file exist already?
        os.remove(outputfilename) # if so, remove it.
    mywriteFITS(image, header, outputfilename)
    dv.load_src(outputfilename)

def SFgain(whichbox):
    """ Take Source Follower Gain data.  
        User supplies the sub-array box for the checkoffset and statistics
        that will be calculated for each PED read.
        
    """
    # Give the detector reset voltage range
    vrst_start = -200
    vrst_end = 3401
    vrst_step = 25

    CurrTemp = gettemp()
    # Set the object name and include the temperature in the name
    rd.object = "SFgain%dK_900biascurr"%(CurrTemp)
    #
    rd.lc = 'Taking source follower gain data. Reset is always on.'
    #
    # Store some of the current voltage settings.
    OrigOffset= dd.voffset
    OriggblRsthi = dd.glbRsthi
    OrigVreset = dd.vreset
    # Now, turn reset always on (remember inverter on the gate of reset switch)
    dd.glbRsthi = 0 

    dd.vreset = vrst_start
    # Get some initial data to make sure the beginning of this data ramp is
    # still on scale.
    checkoffset(25,whichbox)
    # Open a file to store data, using "a" = append mode
    wfile = open(xdir.get_objpath() + "/" + rd.object +".txt", "a")
    for v in range( vrst_start, vrst_end, vrst_step) :
        dd.vreset = v
        #pedfilename = xdir.get_objpath() + "/ped.fits"
        #tpedscan()
        pedfilename = xdir.get_nextobjfilename() + ".fits"
        tpedrun()
        pedfile = pyfits.open(pedfilename)
        # These are FITS files. Get just the data and ignore header.
        peddata = pedfile[0].data
        # Write some statistics.
        value1 = peddata[whichbox].mean()


        # Convert from ADUs to milliVolts
        volts1 = value1*ADUtoVolt - (dd.voffset)
        #volts = value*ADUtoVolt - (dd.voffset)

        wfile.write("%s " % dd.vreset)
        wfile.write("%s " % volts1)

        wfile.write("%s " % value1)

        wfile.write("%s " % dd.voffset)
        wfile.write("\n")
        pedfile.close()

        print "# vreset Output-ADU  Output-Voltage    voffset"
        print dd.vreset, value1, volts1, dd.voffset

        # We need to make sure the voltage after amplifiers is within range of
        # the A/D converters.  So, if average value in ADU is above or below
        # specified number, then adjust Voffset to put everything back in range
        if value1 > 15000 :
            dd.voffset = dd.voffset - vrst_step*2
        if value1 < -15000 :
            dd.voffset = dd.voffset + vrst_step*2

    
    wfile.close()
    # restore original voltage values.
    dd.voffset = OrigOffset 
    dd.glbRsthi = OriggblRsthi  
    dd.vreset = OrigVreset 
    rd.lc = ''

def varyVgs(VgsStart,VgsEnd,VgsStep,NumImg):
    # User supplies 3 voltages (start,end,step) for Vgs, and number of images.
    OrigOffset= dd.voffset
    OrigVgs = dd.vgs
    OrigVreset = dd.vreset
    for voltage in range(VgsStart,VgsEnd+1,VgsStep):
        dd.vgs = voltage # Set the new Vgs value.
        # Changed Vgs, so check the voltage into preamp and adjust offset
        # but we need to do that for EACH PIXEL INDIVIDUALLY.
        AveValImg = numpy.zeros([9,10])  # create a blank array of zeros
        PixelVoffset = numpy.zeros([9,10])
        for row in range(9):
            for col in range(10):
                # Create tuple for the index of just this pixel.  We could
                # actually pass thisPix = row,col  but then you can't do
                # an array.mean() on that one pixel because it returns a 
                # single value (Error: int object has no attribute mean).
                # However we can do a mean on an array that only has one value.
                thisPix = (slice(row,row+1),slice(col,col+1))
                # adjust the voffset to get output in range of A/D converters.
                checkoffset(20,thisPix, 15000, 25000)
                # Get an averaged image
                image, header = aveimg('src', NumImg) 
                # Store this particular pixel average value in an array
                AveValImg[row,col] = image[row,col]
                PixelVoffset[row,col] = dd.voffset
        # Add comment to FITS header
        myhistory = 'Each pixel has a different setting of Voffset'
        header.add_history(myhistory)
        # Finally, write the averaged and assembled image to disk.
        nextfilename = xdir.get_nextobjfilename() # Get filename, use later too.
        outputfilename =  nextfilename + ".fits" 
        mywriteFITS(AveValImg, header, outputfilename)
        rd.objnum = xdir.objnum + 1
        # File for the Voffset values.
        wfile = open(nextfilename + "_Voffset_Table.txt", "a")
        wfile.write("%s" % (PixelVoffset))
        wfile.close()

    # Return these voltages to the original settings.
    dd.voffset = OrigOffset
    dd.vgs = OrigVgs
    dd.vreset = OrigVreset



def RunBestVgs(srcORped, NumImg, lookupVgs=bestVgs295K):
    # This will create an image where each pixel uses the best Vgs value above.
    # User supplies integer number of frames to average for final image.
    # Very time consuming since this has a lot of checks for Voffset.
    rd.gc = 'Averaged %d independent images together'%NumImg
    # Create array of zeros to store the final averaged value image.
    AveValImg = numpy.zeros([7,7])
    for row in range(7):
      for col in range(7):
        # Set Vgs to value from look-up table 
        dd.vgs = lookupVgs[row,col]
        # Create tuple for the index of just this pixel.  We could
        # actually pass thisPix = row,col  but then you can't do
        # an array.mean() on that one pixel because it returns a 
        # single value (Error: int object has no attribute mean).
        # However we can do a mean on an array that only has one value.
        thisPix = (slice(row,row+1),slice(col,col+1))
        # adjust the voffset to get output in range of A/D converters.
        checkoffset(20,thisPix, 15000, 25000)
        # Get an averaged image
        image, header = aveimg(srcORped, NumImg) 
        # Set this particular pixel average value in the array
        AveValImg[row,col] = image[row,col]
    # Add comment to FITS header
    myhistory = 'Each pixel has a different setting of Vgs and Voffset'
    header.add_history(myhistory)
    # Finally, write the averaged and assembled image to disk.
    outputfilename = xdir.get_nextobjfilename() + ".fits" # Get the filename
    mywriteFITS(AveValImg, header, outputfilename)
    rd.objnum = xdir.objnum + 1 # increase object number for next filename
    rd.gc = '' # clear comment field

def arraynoise(NumImgSamp, lookupVgs=bestVgs295K):
    """ 
    This will take a series of single pixel reads, which can then be used
    to determine the noise.
    
    This chip seems to integrate up in dark current (and visible light?)
    which is why the Voffset is set to put us in the -15000 to -5000 range.
    It reaches a saturation level after 3 to 4 ms.
     
    If you take a large number (>200) of images for each pixel, then there
    will be a very large number of files, e.g. NumImgSamp=1000 gives 49000 
    files. Since, the OS doesn't like to move or copy that many in one 
    directory, then we have to break it into separate directories.
    """
    for row in range(7):
      for col in range(7):
        # Set the object name (see above about large number of files)
        # and include the temperature and pixel number in the name
        CurrTemp = gettemp()
        rd.object = "noise%dK-%d-%d"%(CurrTemp,row+1,col+1)
        # Set Vgs to value from look-up table 
        dd.vgs = lookupVgs[row,col]
        # Set array parameters back to nominal so that we can check Voffset.
        rd.nrowskip=0
        rd.ncolskip=0
        rd.nrow = 7
        rd.ncol = 7
        rd.nsamp = 1
        tpedscan() # just to get the new Vgs value settled.
        # Create tuple for the index of just this pixel.  We could
        # actually pass thisPix = row,col  but then you can't do
        # an array.mean() on that one pixel because it returns a 
        # single value (Error: int object has no attribute mean).
        # However we can do a mean on an array that only has one value.
        thisPix = (slice(row,row+1),slice(col,col+1))
        # adjust the voffset to get output in range of A/D converters.
        checkoffset(20,thisPix, -15000, -5000)
        # Now set the skipping to the correct pixel.
        # Of course these are header values, so we know which pixel this is!
        rd.nrowskip = row
        rd.ncolskip = col
        # These are not the number of real pixels, but instead are
        # the number of reads of the single pixel.
        rd.nrow = 128
        rd.ncol = 128
        # Get the image(s).
        # Each image has noise and darkcurrent data, but for only one reset.
        # Each subsequent image has an independent reset.
        for blah in range(NumImgSamp):
          singlepix() # must be issued every time, since clock prog clears it.
          rd.nsamp = 1
          tpedrun()
        # SUTR takes more data, but no independent reset. So not using...
        #rd.nsamp = NumImgSamp
        #sutr() 
        #
        
def varyGlbRst(NumImgSamp):
    # Vary the Low state of Global Reset clock and take lots of data.
    oldGlbRstLo = dd.glbRstlo
    for ResetVoltage in range(0,2001,100):
        dd.glbRstlo = ResetVoltage
        for blah in range(NumImgSamp):
            tsrun()
    dd.glbRstlo = oldGlbRstLo

