"""
cmc_THzGen1.py - a script to run in pydsp. Gives user some defined commands.
to use, type: execuser cmc_THzGen1
"""

#from autouser import AutoUser
from run import rd
#from pydsp import cloop
import time
import numpy
import xdir
import ociw
import det
from det import dd
import ls332
import pyfits

OrigOffset = dd.voffset

ADUtoVolt = dd.adrange / ( 2**16 * dd.ampgain)

#######################################################################################

# Basic Acquisition code for "Shutter Per Pixel" --------------------------------------

def SampPerPixAcquire(**kwds) :
    """
    Acquire (and return) coadded signal and pedestal.

    Return both as numarray int32 arrays, without division by nsamp.
    this function is where sync of time and temp data *should* be done.
    """

    rd.ftime =  time.ctime()

    nrow, ncol, nsamp, nout = rd.nrow, rd.ncol, rd.nsamp, dd.nout
    print nrow, ncol, ncol*2*nsamp, det.prepix, det.postpix
    rd.pre_temp = ls332.readTemp()

    ociw.clear_fifo() # flush out any pixels.
    ociw.command(20,0) # tell the clock program it is ok to start clocking.

    rawimage = ociw.RawFrame(nrows=nrow, ncols=ncol*nsamp, prepix=det.prepix, postpix=det.postpix)
    
    rawimage.grab()
    print 'here'

    """
    if dsp.dspthread.interrupted:
        print "acquire was interrupted."
        raise KeyboardInterrupt
    """

def THzUnscramble(rawimage) :
    # This part really isn't part of the unscramble, but needs to go somewhere...
    rd.post_temp = ls332.readTemp()
    dd.temp = "%7.3f" % rd.post_temp
    rd.ltime =  time.ctime()

    # First separate pixels by outputs by taking every Nth pixel and putting it
    # into a separate row of pixels.  Where N goes from 1 to det.dd.nout.
    # Start with the first block from first output because it is a bad thing
    # to try to start with an empty array and use numpy.concatenate with
    # another array that is not empty. 
    newimage = rawimage.data[:, ::det.dd.nout] # every NOUT-th pixel
    for i in range(1,det.dd.nout):
        tmprow = rawimage.data[:, i::det.dd.nout]
        oldimage = newimage
        # Concatentate in the row direction (paste at bottom of old image)
        newimage = numpy.concatenate((oldimage, tmprow), axis=0)
    PixByOut = newimage

    # Create empty arrays for each buffer
    ped = numpy.zeros(shape=(nrow, ncol), dtype=numpy.float32)
    sig = numpy.zeros(shape=(nrow, ncol), dtype=numpy.float32)

    # Take each set of NSAMP pixels and add together, without division by nsamp. 
    # Also, separate into PED and SIG arrays.
    for row in range(0, nrow):
        for col in range(0, ncol):
            CoaddedPedPix = 0
            CoaddedSigPix = 0
            # Add up the samples for each pixel of the Ped and Sig
            for i in range(0, rd.nsamp):
                CoaddedPedPix += PixByOut[row, ((col*nsamp*2)+i)]
                CoaddedSigPix += PixByOut[row, ((col*nsamp*2)+i+rd.nsamp)]
            ped[row, col] = CoaddedPedPix
            sig[row, col] = CoaddedSigPix
    return sig, ped

def sampbypix(**kwds) :
    """
    This function requires the clock program THz-gen1rev1-CDS.

    This function acquires a set of pedestal and signal samples per pixel.  In other
    words it will get all the samples (ped and sig) for a given pixel before it clocks
    to the next pixel. 
    writes the normalized difference into a src buffer

    NOTE: This command will save the image to disk using an incrementing filename scheme. 
    Image will be displayed in the next available DV buffer.
    """

    runfile = xdir.get_nextobjfilename() + ".fits"

    THzrawimage = SampPerPixAcquire() # Take the data
    sig, ped = THzUnscramble(THzrawimage) # put the pixels in the correct order
    checkItime()  # don't check itime since this is per pixel and minimum itime is 1


    # Which kind of image (ped, sig, src or bkg) did user ask for?  
    bufname = rd.bufferflag
    if bufname in ("src","bkg") :
        image = sig - ped
    elif bufname == "ped" :
        image = ped
    elif bufname == "sig" :
        image = sig
    # Divide by the number of samples taken.
    image /= float(rd.nsamp)
    
    fits.write_image(runfile, image) # procedural!
    rd.objnum = xdir.objnum + 1
    # and tell dv to display it.
    if bufname == "bkg" :
        dv.load_bkg(runfile) # fits file writer observer??
    else :
        dv.load_src(runfile)

# Code for taking data ----------------------------------------------------------------

# This is a way to automatically adjust the offset voltage to the preamp so that
# the pixel voltage is in range of the AD converter.
def check_offset(offset_step) :
    #offset_step=100
    oldoffset = dd.voffset  # Store the original value.
    rd.nsamp = 1
    pedscan()  # Take a frame (not correlated double sample!)
    pedfilename = xdir.get_objpath() + "/" + rd.bufferflag + ".fits" # PYDSP file naming 
    pedfile = pyfits.open(pedfilename) # open that ped file.
    peddata = pedfile[0].data  # make an array of the data.
    value = peddata[0:1, 1:11].mean()  # Find a mean value of some pixels.
    # print value
    # Now change the offset voltage to be on scale.
    if value > 15000 :
         dd.voffset = oldoffset - offset_step
    if value < -15000 :
         dd.voffset = oldoffset + offset_step
    pedfile.close()
    return value


def pixnoise(myrow,mycol) :
    """
    This code takes a very large number of reads for a single pixel, which are meant
    to be used in a FFT noise analysis.  The clock program is told to do "singlepix"
    which means that (ncol*nrow/nout) is the number of times that single pixel is read
    instead of ncol and nrow being used as the number of clocks to do for the shift
    register.  So, USER MUST SET nrow and ncol!!!

    To get to the different pixels in a row, ncolskip is used by this program.

    Note: because we have two outputs (and hence two rows) per image, there will
    be two pixels read at a time for this program just like any other for this chip.
    Also, it is vital that the number of samples be divisible by 2 for the
    outputformatting.
    """
    
    # Set the FITS formating to block so that we can use nrow as part of the
    # number of times to sample.  Otherwise, the "no_rowshiftreg" format will 
    # complain about wrong size arrays.  This will leave us with NOUT separate 
    # blocks for each output, e.g. left half and right half in the case of 2 outputs. 
    # Note: you could also use "interleaved" format, but that requires separation later. 
    dd.outputformat = "block"
    # Loop through all the pixels.
    for col in range(1,12):
        rd.ncolskip = col # This goes to a specific pixel in the row.
        rd.ctstime = 800 # set pixel dwell time longer for Voffset check.
        rd.nrow = 4
        rd.ncol = 24
        rd.itime = 50
        pixval = 32768
        while pixval < -15000 or pixval > 15000 :
            singlepix()
            pixval = check_offset(50) # set Voffset if out of bounds
        rd.nrow = myrow
        rd.ncol = mycol
        rd.ctstime = 100 # set pixel dwell time short for the actual noise data.
        singlepix()  # tells clock program that the next image is a single pixel read.
        pedrun() # ped so that we get single reads, not a pair subtracted.
    # Reset the FITS formating back to the original
    dd.outputformat = "no_rowshiftreg"
    rd.nrow = 2
    rd.ncol = 12
    rd.ncolskip = 0
    rd.ctstime = 500
    rd.itime = 100


def vgs_vs_itime():
    """
    Code to take a set of images with several different integration times.  
    Then repeat that set of images for different values of Vgs.
    """
    # Store some of the original settings
    OrigOffset = dd.voffset
    VgsOrig = dd.vgs
    # At vgs=300, for Tiny Imager 4, the offset needs to be about -2800.
    dd.voffset = -2800

    for voltage in range(300,1001,50) :
        rd.itime = 100

        dd.vgs = voltage
        # We changed Vgs so, we might need to adjust the offset voltage.
        # Do this multiple times with smaller adjustments in the offset because lowest 
        # and highest values of Vgs need less change in the offset voltage than the
        # values of Vgs between 550-700.
        pixval = 32768
        while pixval < -15000 or pixval > 15000 :
            pixval = check_offset(100) # set Voffset if out of bounds

        # Do the shortest integration time possible.
        rd.itime = 1
        srun()
        # Then do larger integration times.
        for inttime in range(10,101,10):
            rd.itime = inttime
            srun()
    
        for inttime in range(500,20001,500):
            rd.itime = inttime
            srun()

    # Return the voffset to its original value
    dd.voffset = OrigOffset
    dd.vgs = VgsOrig

def vgs_vs_sutr():
    """
    Code to take a set of sample up the ramp images with different integration times.  
    Then repeat that set of images for different values of Vgs.
    """
    # Store some of the original settings
    OrigOffset = dd.voffset
    VgsOrig = dd.vgs
    # At vgs=300, for Tiny Imager 4, the offset needs to be about -2800.
    dd.voffset = -2800

    for voltage in range(300,1001,50) :
        rd.itime = 100

        dd.vgs = voltage
        # We changed Vgs so, we might need to adjust the offset voltage.
        # Do this few times with smaller adjustments in the offset because the lowest 
        # and highest values of Vgs need less change in the offset voltage than the
        # values of Vgs between 550-700.
        pixval = 32768
        while pixval < -15000 or pixval > 15000 :
            pixval = check_offset(100) # set Voffset if out of bounds

        # Do the shortest integration time possible.
        rd.itime = 1
        rd.nsamp = 200
        sutr()
        # Then do larger integration times.
        rd.itime = 20
        rd.nsamp = 200
        sutr()
        # Repeat
        rd.itime = 1
        rd.nsamp = 200
        sutr()
        # 
        rd.itime = 20
        rd.nsamp = 200
        sutr()

    # Return the voffset to its original value
    dd.voffset = OrigOffset
    dd.vgs = VgsOrig

def runmany(val):
    for val in range(0,val):
        srun()
#--------------------------------------------------------------------------------------

def tinyimager(val,**kwds):
    """
    Switch between the different Tiny Imagers of the THz Chip 
    Syntax is:  tinyimager(1)
    which will turn on the Tiny Imager 1. 
    """
    if val==0:
        # First turn all tiny imagers off
        dd.CS1_lo = 0
        dd.CS2_lo = 0
        dd.CS3_En = 0
        dd.CS4_En = 0

    elif val==1:
        # First turn all tiny imagers off
        dd.CS1_lo = 0
        dd.CS2_lo = 0
        dd.CS3_En = 0
        dd.CS4_En = 0
        # Then turn on 1
        dd.vgs = 0
        dd.vbiascur = 700
        dd.CS1_lo = 3300

    elif val==2:
        # First turn all tiny imagers off
        dd.CS1_lo = 0
        dd.CS2_lo = 0
        dd.CS3_En = 0
        dd.CS4_En = 0
        # Then turn on 2
        dd.vgs = 0
        dd.vbiascur = 700
        dd.CS2_lo = 3300

    elif val==3:
        # First turn all tiny imagers off
        dd.CS1_lo = 0
        dd.CS2_lo = 0
        dd.CS3_En = 0
        dd.CS4_En = 0
        # Then turn on 3
        dd.vgs = 700
        dd.vbiascur = 0
        dd.CS3_En = 3300

    elif val==4:
        # First turn all tiny imagers off
        dd.CS1_lo = 0
        dd.CS2_lo = 0
        dd.CS3_En = 0
        dd.CS4_En = 0
        # Then turn on 4
        dd.vgs = 700
        dd.vbiascur = 0
        dd.CS4_En = 3300

    else : 
        print "value not in range"


def photodiode(val,**kwds):
    """
    The photodiodes need a different set of voltages than the THz pixels.
    """
    if val == 'on':
        tinyimager(1) # turn on tiny imager 1
        dd.vbiascur=700
        dd.vreset=3300
    elif val == 'off':
        dd.vbiascur=0  # not sure???
        dd.vreset=3300 # not sure???
    else :
        print 'what did you say?'


