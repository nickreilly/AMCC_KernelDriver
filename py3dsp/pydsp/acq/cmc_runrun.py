"""
This is a script to run in pydsp.
    
    Re-write of runrun.py routines for a detector that requires clocking
    which does a reset, read, integrate and read on each pixel before 
    moving onto the next pixel.  
    For THz Gen 2 & 3 chips

to use, type: execuser filename (no .py)
"""

from autouser import AutoUser
from run import rd
from pydsp import cloop
import det
import xdir
import numpy
import pyfits
import time
import os
import fits
import dv
import ociw
import dsp
#from runrun import checkItime

def PedClockit(bufs, wfile=sys.stdout, **kwds) : 
    """ copy of dsp.clockit()
    Acquire an image into the driver buffer.

    first, tell device driver to get ready.
    then, tell clock program to go.
    then, sit and wait for the dsp 
    (all the while reading and displaying pixels to go.)
    separate thread would be nice.
    """
    global starttime
    starttime = time.time()
    ociw.clear_fifo()
    ociw.command(31, 0) # tell the clock program its ok to start clocking.
    dsp.startingAlert() # tell any observers too!
    # print "pixels to go:"
    for buf in bufs:
        buf.grab()
        if dsp.dspthread.interrupted:
            print >> wfile, "aborting"
            dsp.stoppingAlert()
            raise KeyboardInterrupt
    else:
        dsp.stoppingAlert()

def BigBuffAcquire(**kwds) :
    """
    Acquire (and return) coadded signal and pedestal.

    Return both as numpy int32 arrays, without division by nsamp.
    this function is where sync of time and temp data *should* be done.
    """
    from run import rd
    import ls332
    rd.ftime =  time.ctime()
    bufname = rd.bufferflag
    # make a list of buffers
    bufs = []
    nrow, ncol, nsamp = rd.nrow, rd.ncol, rd.nsamp

    rd.pre_temp = ls332.readTemp()
    import numpy
    # Note:                        row & col reversed here!
    if bufname == "ped" :
        rawbuf = numpy.zeros(shape=(ncol, nsamp*nrow), dtype=numpy.float32)
    else :
        rawbuf = numpy.zeros(shape=(ncol, 2*nsamp*nrow), dtype=numpy.float32)

    def addtorawbuf(buf, rawbuf=rawbuf) :
        rawbuf += buf.data
        del buf.data
        
    prepix, postpix = det.prepix, det.postpix
    #for i in range(nsamp*2) :
    # Again, row & col reversed from usual sense.
    if bufname == "ped" :
        bufs.append(ociw.RawFrame(ncol, nsamp*nrow, prepix, postpix))
    else :
        bufs.append(ociw.RawFrame(ncol, 2*nsamp*nrow, prepix, postpix))

    for buf in bufs :
        buf.funcs.append(addtorawbuf)
    # Only one buffer needed since everything is in one big image read. 
    #bufs.append(addtorawbuf)

    if bufname == "ped" :
        PedClockit(bufs=bufs)
    else :
        dsp.clockit(bufs=bufs)
    if dsp.dspthread.interrupted:
        print "acquire was interrupted."
        raise KeyboardInterrupt

    rd.post_temp = ls332.readTemp()

    dd.temp = "%7.3f" % rd.post_temp
    rd.ltime =  time.ctime()
    return rawbuf

BigBuffAcquire = dsp.dspthreadable(BigBuffAcquire)

def GetPedSig(**kwds) :
    """
    Return the Fowler averaged Ped and Sig images from the large raw buffer
    that is obtained in BigBuffAcquire().
    """
    from run import rd
    from det import dd
    nrow, ncol, nsamp, nout = rd.nrow, rd.ncol, rd.nsamp, dd.nout
    rawbuf = BigBuffAcquire() 

    # First, sort the image by each output into "vertical stripes" or "blocks".
    # Note: it is up to the user to be sure that ncol/nout is an integer.
    # Start with the first block from first output because it is a bad thing
    # to try to start with an empty array and use numpy.concatenate with
    # another array that is not empty. 
    # In the limit of 1 output, this does nothing!
    newblock = rawbuf[:, ::nout]
    i=1
    for i in range(1,nout):
        tmpblock = rawbuf[:, i::nout]
        oldblock = newblock
        newblock = numpy.concatenate((oldblock, tmpblock), axis=1)
    SortedNOutBuf = newblock

    # Now create two arrays with all 0.
    # row & col are reversed here.  
    pedbuf = numpy.zeros(shape=(ncol, nrow), dtype=numpy.float32)
    sigbuf = numpy.zeros(shape=(ncol, nrow), dtype=numpy.float32)
    # Descramble the buffer by the sample number.
    # ncol  * 2*nsamp*nrow
    for i in range(0,nsamp):
        # get every (2*nsamp)th pixel.
        tmpped = SortedNOutBuf[:,i::2*nsamp]
        pedbuf += tmpped
        tmpsig = rawbuf[:,i+nsamp::2*nsamp]
        sigbuf += tmpsig
        
    # Divide by nsamp for averaged image.
    sigbuf  /= float(nsamp)
    pedbuf  /= float(nsamp)

    return sigbuf, pedbuf
    

def GetPed(**kwds) :
    """
    Return the averaged Ped image from the large raw buffer
    that is obtained in BigBuffAcquire().
    """
    from run import rd
    from det import dd
    nrow, ncol, nsamp, nout = rd.nrow, rd.ncol, rd.nsamp, dd.nout
    rawbuf = BigBuffAcquire() 

    # First, sort the image by each output into "vertical stripes" or "blocks".
    # Note: it is up to the user to be sure that ncol/nout is an integer.
    # Start with the first block from first output because it is a bad thing
    # to try to start with an empty array and use numpy.concatenate with
    # another array that is not empty. 
    # In the limit of 1 output, this does nothing!
    newblock = rawbuf[:, ::nout]
    i=1
    for i in range(1,nout):
        tmpblock = rawbuf[:, i::nout]
        oldblock = newblock
        newblock = numpy.concatenate((oldblock, tmpblock), axis=1)
    SortedNOutBuf = newblock

    # Now create array with all 0.
    # row & col are reversed here.  
    pedbuf = numpy.zeros(shape=(ncol, nrow), dtype=numpy.float32)
    # Descramble the buffer by the sample number.
    # ncol  * nsamp*nrow
    for i in range(0,nsamp):
        # get every (nsamp)th pixel.
        tmpped = SortedNOutBuf[:,i::nsamp]
        pedbuf += tmpped
        
    # Divide by nsamp for averaged image.
    pedbuf  /= float(nsamp)

    return pedbuf
    
def BigBuffScan(**kwds) :
    """
    This function depends on the acquisition mode set using the commands 'bkg', 'ped', 'sig', and 'src'.

    This acquires a fowler pedestal and signal and writes the normalized difference into the src buffer
    or the pedestal into ped buffer, or the signal into sig buffer, depending upon the last set acquisition mode.

    NOTE: This command does not save the image to disk, a subsequent invocation of this command will 
    overwrite the buffer. Image will be displayed in the next available DV buffer.
    """
    from run import rd
    bufname = rd.bufferflag
    filename = xdir.get_objpath() + "/" + bufname + ".fits"

    if bufname == "ped" :
        ped = GetPed() #acquire()
    else :
        sig, ped = GetPedSig() #acquire()
    #checkItime()

    if os.access(filename,6) : # is it there already?
        os.remove(filename) # if so, remove it.

    if bufname in ("src","bkg") :
        image = sig - ped
    elif bufname == "ped" :
        image = ped
    elif bufname == "sig" :
        image = sig


    fits.write_image(filename, image)

    # and tell dv to display it.
    if bufname == "bkg" :
        dv.load_bkg(filename) # fits file writer observer??
    else :
        dv.load_src(filename)

BigBuffScan = dsp.dspthreadable(BigBuffScan)

def BigBuffRun(**kwds) :
    """
    This function depends on the acquisition mode set using the commands 'bkg', 'ped', 'sig', and 'src'.

    This acquires a fowler pedestal and signal and writes the normalized difference into a src buffer
    or the pedestal into a ped buffer, or the signal into a sig buffer, depending upon the last set acquisition mode.

    NOTE: This command will save the image to disk using an incrementing filename scheme. 
    Image will be displayed in the next available DV buffer.
    """

    from run import rd
    bufname = rd.bufferflag
    runfile = xdir.get_nextobjfilename() + ".fits"

    if bufname == "ped" :
        ped = GetPed() #acquire()
    else :
        sig, ped = GetPedSig() #acquire()
    #checkItime()

    if bufname in ("src","bkg") :
        image = sig - ped
    elif bufname == "ped" :
        image = ped
    elif bufname == "sig" :
        image = sig
    # and tell dv to display it.

    fits.write_image(runfile, image) # procedural!
    rd.objnum = xdir.objnum + 1
    if bufname == "bkg" :
        dv.load_bkg(runfile) # fits file writer observer??
    else :
        dv.load_src(runfile)

BigBuffRun = dsp.dspthreadable(BigBuffRun)

def tsscan(**kwds) :
    """
    This acquires a fowler pedestal and signal and writes the normalized difference into a src buffer.

    NOTE: This command does not save the image to disk, a subsequent invocation of this command will 
    overwrite the buffer. Image will be displayed in the next available DV buffer.
    """
    rd.sampmode = rd.bufferflag = "src"
    BigBuffScan()

def tbscan() :
    """
    This acquires a fowler pedestal and signal and writes the normalized difference into the
    bkg buffer (in DV this is always buffer F).

    NOTE: This command does not save the image to disk, a subsequent invocation of this command will 
    overwrite the buffer. Image will be displayed in the next available DV buffer.
    """
    rd.sampmode = rd.bufferflag = "bkg"
    BigBuffScan()

def tpedscan() :
    """
    This acquires a pedestal frame writes it into a ped buffer.

    NOTE: This command does not save the image to disk, a subsequent invocation of this command will 
    overwrite the buffer. Image will be displayed in the next available DV buffer.
    """
    rd.sampmode = rd.bufferflag = "ped"
    BigBuffScan()

def tsigscan() :
    """
    This acquires a signal frame and writes it into a sig buffer.

    NOTE: This command does not save the image to disk, a subsequent invocation of this command will 
    overwrite the buffer. Image will be displayed in the next available DV buffer.
    """
    rd.sampmode = rd.bufferflag = "sig"
    BigBuffScan()

# logical sequence is: acquire the images, including peds, sigs, etc.
# then, after the images are acquired, decide which to save..
# this works for everything except SUTR.

def tsrun() :
    """
    This acquires a fowler pedestal and signal and writes the normalized difference into a src buffer.

    NOTE: This command will save the image to disk using an incrementing filename scheme and displayed 
    in the next available DV buffer.
    """
    rd.sampmode = rd.bufferflag = "src"
    BigBuffRun()

def tbrun() :
    """
    This acquires a fowler pedestal and signal and writes the normalized difference into a bkg buffer.

    NOTE: This command will save the image to disk using an incrementing filename scheme and displayed 
    in the next available DV buffer.
    """

    rd.sampmode = rd.bufferflag = "bkg"
    BigBuffRun()

def tpedrun() :
    """
    This acquires a pedestal frame and writes it into a ped buffer.

    NOTE: This command will save the image to disk using an incrementing filename scheme and displayed 
    in the next available DV buffer.
    """
    rd.sampmode = rd.bufferflag = "ped"
    BigBuffRun()

def tsigrun() :
    """
    This acquires a signal frame and writes into into a sig buffer.

    NOTE: This command will save the image to disk using an incrementing filename scheme and displayed 
    in the next available DV buffer.
    """
    rd.sampmode = rd.bufferflag = "sig"
    BigBuffRun()

def tsutr(wfile=sys.stdout, **kwds) :
    """
    Sample Up The Ramp
    This command waits on the device driver and when a raw buffer has data in it,
    it passes the buffer to a fits object and writes it out.

    NOTE: This function is both high level and low level.
    Since sample up the ramp MUST acquire and write, there
    really is no way to separate them.
    """
    # Set as a src image
    rd.sampmode = rd.bufferflag = "src"
    import ls332

    rd.sampmode = "sutr"
    nrow, ncol, nframes, nout = rd.nrow, rd.ncol, rd.nsamp, dd.nout
    if nframes % 2 :
        print>>wfile, "nsamp must be even for sutr mode!"
        return

    BaseObjFile = xdir.get_nextobjfilename()
    filebase=BaseObjFile +"_%03d.fits"
    runfile=BaseObjFile +".fits"

    # make a list of buffers
    bufs = []

    rd.pre_temp = ls332.readTemp()
    rd.ftime =  time.ctime()
    import numpy
    # Note:                        row & col reversed here!
    rawbuf = numpy.zeros(shape=(ncol, nframes*nrow), dtype=numpy.float32)

    def addtorawbuf(buf, rawbuf=rawbuf) :
        rawbuf += buf.data
        del buf.data
        
    prepix, postpix = det.prepix, det.postpix

    # Again, row & col reversed from usual sense.
    bufs.append(ociw.RawFrame(ncol, nframes*nrow, prepix, postpix))

    for buf in bufs :
        buf.funcs.append(addtorawbuf)
    # Only one buffer needed since everything is in one big image read. 

    dsp.rampnext() # clocking program special behavior.
    dsp.clockit(bufs=bufs)
    if dsp.dspthread.interrupted:
        print "acquire was interrupted."
        raise KeyboardInterrupt

    rd.post_temp = ls332.readTemp()
    rd.ltime =  time.ctime()

    # First, sort the image by each output into vertical stripes or blocks.
    # Note: it is up to the user to be sure that ncol/nout is an integer.
    # Start with the first block from first output because it is a bad thing
    # to try to start with an empty array and use numpy.concatenate with
    # another array that is not empty. 
    # In the limit of 1 output, this does nothing!
    newblock = rawbuf[:, ::nout]
    i=1
    for i in range(1,nout):
        tmpblock = rawbuf[:, i::nout]
        oldblock = newblock
        newblock = numpy.concatenate((oldblock, tmpblock), axis=1)
    SortedNOutBuf = newblock
    # Now descramble by sample number and write to files.
    for f in range(nframes) :    
        sampfilename = filebase%(f+1)
        if os.access(sampfilename,6) : 
            os.remove(sampfilename) # delete it if it exists.
        # Descramble the buffer by the sample number.
        # ncol  * nsamp*nrow
        # get every (nsamp)th pixel.
        frame = SortedNOutBuf[:,f::nframes]
        if f == 0 :
            ped = frame
        fits.write_image(sampfilename, frame)
    fits.write_image(runfile, frame-ped)
    rd.objnum = xdir.objnum + 1

    print>>wfile, "saved to %s and others!"%runfile
    savesetup()

tsutr = dsp.dspthreadable(tsutr)


def tcrun(wfile=sys.stdout) :
    """
    Run the array continuously. Type 'burst' to abort. This command will return immediatly and run in the background.
    Type 'burst' to enter 'burst' mode which ends 'crun' mode.
    """
    import threading
    def tcrunfunc() :
        from run import rd
        try :
            # we want to return immediately if dsp is busy.
            # is there an easier way?
            bufs = []
            #for i in range(rd.nsamp*2) :
            bufs.append(ociw.RawFrame(rd.ncol, 2*rd.nsamp*rd.nrow, prepix=det.prepix, postpix=det.postpix))
            print>>wfile, "CRUN mode. type 'burst' to abort"
            while 1 :
                time.sleep(0.05)
                dsp.dspthread.do(dsp.clockit, bufs=bufs)
                if dsp.dspthread.interrupted:
                    print "saw interrupt flag."
                    raise KeyboardInterrupt
                # could update dv here?
        except StopIteration :
            print "DSP already busy."
        except KeyboardInterrupt :
            print "abort."
        except :
            import traceback
            traceback.print_exc()
            print "some other exception"
        
    # how about do_nowait??
    # typing crun twice will start two of these things!
    thread = threading.Thread(target=tcrunfunc)
    thread.setDaemon(True) # kills crun if we bail out.
    thread.start()
    time.sleep(0.1) # allow thread to start and print its stuff.
    
