""" runrun.py, parallel file of runrun.fth.

runrun interfaces the run data of run.py
with the hardware of dsp.py
not to mention the fits file, DV, and showall code.
it is kinda the glue code that brings things together
"""

__version__ = """$Id: runrun.py 426 2007-07-18 19:18:15Z dsp $ """

__author__ = "$Author: dsp $"

__URL__ = "$URL: https://astro.pas.rochester.edu/admin/svn/pydsp/trunk/pydsp/runrun.py $"

import os, sys, xdir, logging
import dsp as dsp
from dsp import dspthreadable, dev
from run import rd, savesetup
import det
from det import dd
import fits
import dv
import time
# import ociw_cpy as ociw
# import ociw
import amcc

logger = logging.getLogger("pydsp.runrun")
logger.debug("Opening runrun")
# dev = None

# "from runrun import *" will only import these names.
__all__ = [
    "sscan", "bscan", "bgscan", "scan",
    "srun", "brun", "run",
    "pedscan", "sigscan", 
    "bkg", "src", "vdiode",
    "pedrun", "sigrun", "sutr", "ldrsutr", "scandir", "scantime", "minItime",
    "crun", "burst", "calibrateItime", "preCheckItime"]

def burst():
    """
    Turns off crun mode and returns to the normal acquisition mode.
    """
    dsp.dspthread.interrupt("burst command")
    time.sleep(0.1)

def abort():
    """
    Same as "burst".
    """
    burst() 

def src():
    """
    Set a flag to make the current DV buffer store a source image.
    NOTE: This function should not be used by the user directly, use
    only if you know what you are doing. This function is called 
    by the 'sscan' and 'srun' commands.
    """
    rd.sampmode = rd.bufferflag = "src"

def bkg():
    """
    Set a flag to make the current DV buffer store a background image.
    NOTE: This function should not be used by the user directly, use
    only if you know what you are doing. This function is called 
    by the 'bscan' and 'brun' commands.
    """
    rd.sampmode = rd.bufferflag = "bkg"

def ped():
    """
    Set a flag to make the current DV buffer store a pedestal image.
    NOTE: This function should not be used by the user directly, use
    only if you know what you are doing. This function is called 
    by the 'pedscan' and 'pedrun' commands.
    """
    rd.sampmode = rd.bufferflag = "ped"

def sig():
    """
    Set a flag to make the current DV buffer store a signal image.
    NOTE: This function should not be used by the user directly, use
    only if you know what you are doing. This function is called 
    by the 'sigscan' and 'sigrun' commands.
    """
    rd.sampmode = rd.bufferflag = "sig"

def vdiode(wfile=sys.stdout) :
    """
    Print out the current diode voltage.
    """
    # display the current diode voltage
    print(dsp.vDiode(), file=wfile)

#calconst = [0.0, 0.017932, 0.022846, 0.00134271, 0.00950875] # re-calculate for hawaii 
calconst = [0.0, 0.0111465, 0.422537, 0.003448, 0.004913] # for SB-226
# If ctstime is calculated properly in the clock program (by the writer of the
# clock), then the overhead per pixel should be ZERO, i.e. ctstime = pix time
# Still, can't get the calibrateItime() routine to return calconst[0]=0
# calconst[0] is overhead per pix (col)  
# calconst[1] is overhead per row
# calconst[2] is overhead per frame (nsamp)
# calconst[3] is overhead per colskip
# calconst[4] is overhead per rowskip

def minItime():
    """
    Computes and returns the minimum value of the itime.
    type "print minItime()" to see the result at the command loop.
    """
    from run import rd
    # XXX rewrite using sum and zip, just for fun.
    nrow, ncol, nsamp, nrowskip, ncolskip, ctstime, nout  =  (
        rd.nrow, rd.ncol, rd.nsamp, rd.nrowskip, rd.ncolskip, rd.ctstime, dd.nout )
    if nout != 0:
        npix = nrow * ncol * nsamp / nout
        minitime = ( npix*ctstime/10000 
                + calconst[0]*npix 
                + calconst[1]*nrow*nsamp
                + calconst[2]*nsamp 
                + calconst[3]*nrow*ncolskip*nsamp/nout
                + calconst[4]*nrowskip*nsamp )
    else:
        logger.error("Error, can't divide by zero")
        raise ZeroDivisionError
    return int(minitime)

def scantime():
    """
    Compute and return the total amount of time it will take to acquire an image.
    """
    try:
        min_it = minItime()
    except ZeroDivisionError:
        logger.error("Scantime: zero division error")
        min_it = 0

    return min_it + rd.itime

def preCheckItime(input=input, wfile=sys.stdout):
    """
    This calls the scantime command with current settings to see if the set itime is appropriate.
    If itime is too short, a warning is presented to the user and waits for 3 possible response:
    * A carriage return, a 'n' or 'N' in the input string which aborts back to the command loop.
    * A 'y' or 'Y' where the itime will be set to the minimum possible.
    * If the user types a number, it will attempt to use that number instead as a correction to the original input value.
    """
    from run import rd
    try:
        minitime = minItime()
    except ZeroDivisionError:
        # nout = 0, may mean dd isn't loaded properly
        logger.error("precheckItime: Zero Division Error")
        print("DivideByZero Error: is nout = 0?")
        return 1

    if rd.itime >= minitime:
        return 0 # no problem.
    warn = "Itime is set too short! min is %d \n"%minitime
    while True :
        itime_input = input(warn+"What should I change it to?? (Y[cr]=minimum) [Abort]")
        itime_input = itime_input.strip()
        if not itime_input :
            print("itime unchanged.", file=wfile)
            return 1
        if "y" in itime_input or "Y" in itime_input :
            break
        try :
            newitime = int(itime_input)
            if newitime >= minitime :
                minitime = newitime
                break
            else :
                warn = "that's still too small!\n"
        except :
            print("???", file=wfile)
    print("changing itime to %d"%minitime, file=wfile)
    rd.itime = minitime
    return 0

def checkItime(wfile=sys.stdout):
    """
    Compares the measured pedestal time to the requested itime.
    If the measured pedestal was longer than the requested itime,
    the itime is crowbarred to the minimum, and a warning is
    printed. 
    """
    # XXX if using a shutter, this should always pass!
    minitime = dsp.getPedTime()
    if rd.itime < minitime :
        print("Itime was set too short!", file=wfile)
        print("changed to min itime of %d milliseconds"%minitime, file=wfile)
        rd.itime = minitime

def calibrateItime():
    """
    A Finds the magic calibration constants.

    Does a small number of frames w many columns and few rows
    does a small number of frames many rows and few columns
    does many frames with few rows and few columns
    uses linear algebra to figure out solution.
    modifies values for the rest of the run of pydsp.
    constants are persisted by hand editing runrun.py.
    adding them to the run dict is an exercise for the user. ;-)
    """
    from run import rd
    # from numarray.linear_algebra.LinearAlgebra2 import solve_linear_equations
    from numpy.linalg import solve
    rcsdata = ((1,8192,1,0,0), (8192,4,1,0,0), (1,4,8192,0,0), (4,1024,1,0,8192), (4,1024,1,8192,0))
    arraydata = []
    runtimes = []
    nout = dd.nout
    ctstime = rd.ctstime
    oldparms = rd.nrow, rd.ncol, rd.nsamp, rd.nrowskip, rd.ncolskip, rd.itime
    for nrow, ncol, nsamp, nrowskip, ncolskip in rcsdata:
        rd.nrow, rd.ncol, rd.nsamp, rd.nrowskip, rd.ncolskip = nrow, ncol, nsamp, nrowskip, ncolskip
        rd.itime = 1 # tiny itime.
        # dsp.set_rcs(nrow, ncol, nsamp) # tell driver (not the dsp) the RCS 
        arraydata.append([float(nrow*ncol*nsamp/nout),
                        float(nrow*nsamp),
                        float(nsamp),
                        float(nrow*ncolskip*nsamp/nout), 
                        float(nrowskip*nsamp)])
        acquire() # run the clocking program.
        # The pedestal time from the clock program is usually fairly accurate.
        # We had been rounding UP (add 1.0), to keep from underestimating,
        # but that was overestimating.
        runtimes.append(dsp.getPedTime()) 
    # ok, we are done. restore the parameters.
    rd.nrow, rd.ncol, rd.nsamp, rd.nrowskip, rd.ncolskip, rd.itime = oldparms
    result = solve(arraydata, runtimes)
    # The result is not the overhead for each operation, but the total time.
    # For nrow, nsamp, nrowskip, ncolskip the total time is the same as the
    # overhead, and thus is what we want.  However, the result for the ncol is
    # not the overhead per pixel (which should be zero - see above about 
    # ctstime and clock program)
    result[0] = result[0] - ctstime/10000.  # should be zero, but is not!
    global calconst
    calconst = tuple(result)
    return result

# use inheritance instead of composition for scan functions??
# maybe, but it seems a little inappropriate
# we do want to have common behavior, but function calls would
# probably be more understandable.
# we may also want to fork off a worker thread somewhere.

class ScanFunc :
    def __init__(self, callable) :
        self.callable = callable
        self.__doc__ = callable.__doc__
    def __call__(self,*args,**kwds) :
        self.preScan(*args,**kwds) # do pre-scan checking
        self.callable(*args,**kwds) # do the general scanning stuff.
        self.postScan(*args,**kwds) # do the postScan checking too.
    def preScan(*args,**kwds) :
        preCheckItime(*args,**kwds)
    def postScan(*args,**kwds) :
        checkItime(*args,**kwds)

useTempControl = 'hardwareTempControl'
def acquire(**kwds) :
    """
    Acquire (and return) coadded signal and pedestal.

    Return both as numarray int32 arrays, without division by nsamp.
    this function is where sync of time and temp data *should* be done.
    """
    from run import rd
    if useTempControl == 'hardwareTempControl':
        import ls332
    if useTempControl == 'softwareTempControl':
        import tmptr
    rd.ftime =  time.ctime()
    # make a list of buffers
    bufs = []
    nrow, ncol, nsamp = rd.nrow, rd.ncol, rd.nsamp
    if useTempControl == 'hardwareTempControl':
        rd.pre_temp = ls332.readTemp()
    import numpy
    ped = numpy.zeros(shape=(nrow, ncol), dtype=numpy.float32)
    sig = numpy.zeros(shape=(nrow, ncol), dtype=numpy.float32)

    def addtosig(buf, sig=sig) :
        sig += buf.data
        del buf.data
        
    def addtoped(buf, ped=ped) :
        ped += buf.data
        del buf.data
        
    prepix, postpix = det.dd.prepix, det.dd.postpix
    for i in range(nsamp*2) :
        bufs.append(amcc.RawFrame(dev, nrow, ncol, prepix, postpix))

    for buf in bufs[nsamp:] :
        buf.funcs.append(addtosig)
    for buf in bufs[:nsamp] :
        buf.funcs.append(addtoped)

    dsp.clockit(bufs=bufs)
    if dsp.dspthread.interrupted:
        print("acquire was interrupted.")
        raise KeyboardInterrupt

    # oo. list comprehension! cool! 
    # also, confusing till you are used to them.
    if useTempControl == 'softwareTempControl':
        rd.pre_temp = sum([ 
            tmptr.tdiode(dsp.adc2diode(buf.postdata[1]))
            for buf in bufs[:nsamp]
          ])/ nsamp 

        rd.post_temp = sum([ 
            tmptr.tdiode(dsp.adc2diode(buf.postdata[1]))
            for buf in bufs[nsamp:]
          ])/ nsamp 
    else:
        # import ls332
        rd.post_temp = ls332.readTemp()

    dd.temp = "%7.3f" % rd.post_temp
    rd.ltime =  time.ctime()
    return sig, ped

acquire = dspthreadable(acquire)

def scan(**kwds) :
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

    sig, ped = acquire()
    checkItime()

    if os.access(filename,6) : # is it there already?
        os.remove(filename) # if so, remove it.

    if bufname in ("src","bkg") :
        image = sig - ped
    elif bufname == "ped" :
        image = ped
    elif bufname == "sig" :
        image = sig

    image  /= float(rd.nsamp)
    fits.write_image(filename, image)

    # and tell dv to display it.
    if bufname == "bkg" :
        dv.load_bkg(filename) # fits file writer observer??
    else :
        dv.load_src(filename)

scan = dspthreadable(scan)

def run(**kwds) :
    """
    This function depends on the acquisition mode set using the commands 'bkg', 'ped', 'sig', and 'src'.

    This acquires a fowler pedestal and signal and writes the normalized difference into a src buffer
    or the pedestal into a ped buffer, or the signal into a sig buffer, depending upon the last set acquisition mode.

    NOTE: This command will save the image to disk using an incrementing filename scheme. 
    Image will be displayed in the next available DV buffer.
    """

    from run import rd
    runfile = xdir.get_nextobjfilename() + ".fits"

    sig, ped = acquire()
    checkItime()

    bufname = rd.bufferflag

    if bufname in ("src","bkg") :
        image = sig - ped
    elif bufname == "ped" :
        image = ped
    elif bufname == "sig" :
        image = sig
    # and tell dv to display it.
    image /= float(rd.nsamp)
    fits.write_image(runfile, image) # procedural!
    rd.objnum = xdir.objnum + 1
    if bufname == "bkg" :
        dv.load_bkg(runfile) # fits file writer observer??
    else :
        dv.load_src(runfile)

run = dspthreadable(run)

def sscan(**kwds) :
    """
    This acquires a fowler pedestal and signal and writes the normalized difference into a src buffer.

    NOTE: This command does not save the image to disk, a subsequent invocation of this command will 
    overwrite the buffer. Image will be displayed in the next available DV buffer.
    """
    src()
    scan()

sscan = dspthreadable(sscan)

def bscan() :
    """
    This acquires a fowler pedestal and signal and writes the normalized difference into the
    bkg buffer (in DV this is always buffer F).

    NOTE: This command does not save the image to disk, a subsequent invocation of this command will 
    overwrite the buffer. Image will be displayed in the next available DV buffer.
    """
    bkg()
    scan()

def pedscan() :
    """
    This acquires a pedestal frame writes it into a ped buffer.

    NOTE: This command does not save the image to disk, a subsequent invocation of this command will 
    overwrite the buffer. Image will be displayed in the next available DV buffer.
    """
    ped()
    scan()

def sigscan() :
    """
    This acquires a signal frame and writes it into a sig buffer.

    NOTE: This command does not save the image to disk, a subsequent invocation of this command will 
    overwrite the buffer. Image will be displayed in the next available DV buffer.
    """
    sig()
    scan()

# logical sequence is: acquire the images, including peds, sigs, etc.
# then, after the images are acquired, decide which to save..
# this works for everything except SUTR.

def srun() :
    """
    This acquires a fowler pedestal and signal and writes the normalized difference into a src buffer.

    NOTE: This command will save the image to disk using an incrementing filename scheme and displayed 
    in the next available DV buffer.
    """
    src()
    run()

def brun() :
    """
    This acquires a fowler pedestal and signal and writes the normalized difference into a bkg buffer.

    NOTE: This command will save the image to disk using an incrementing filename scheme and displayed 
    in the next available DV buffer.
    """
    bkg()
    run()

def pedrun() :
    """
    This acquires a pedestal frame and writes it into a ped buffer.

    NOTE: This command will save the image to disk using an incrementing filename scheme and displayed 
    in the next available DV buffer.
    """
    ped()
    run()

def sigrun() :
    """
    This acquires a signal frame and writes into into a sig buffer.

    NOTE: This command will save the image to disk using an incrementing filename scheme and displayed 
    in the next available DV buffer.
    """
    sig()
    run()

def sutr(wfile=sys.stdout, **kwds) :
    """
    Sample Up The Ramp
    This command waits on the device driver and when a raw buffer has data in it,
    it passes the buffer to a fits object and writes it out.

    NOTE: This function is both high level and low level.
    Since sample up the ramp MUST acquire and write, there
    really is no way to separate them.
    """
    # Set as a src image
    src()
    if useTempControl == 'softwareTempControl':
        import tmptr
    if useTempControl == 'hardwareTempControl':
        import ls332

    from run import rd
    rd.sampmode = "sutr"
    nrow, ncol, nframes = rd.nrow, rd.ncol, rd.nsamp
    if nframes % 2 :
        print("nsamp must be even for sutr mode!", file=wfile)
        return

    BaseObjFile = xdir.get_nextobjfilename()
    filebase=BaseObjFile +"_%03d.fits"
    runfile=BaseObjFile +".fits"
    dsp.rampnext() # clocking program special behavior.
    # import amcc
    dev.clear_fifo() # flush out any pixels.
    dev.command(20,0) # tell the clock program its ok to start clocking.
    # amcc.clear_fifo() # flush out any pixels.
    # amcc.command(20,0) # tell the clock program its ok to start clocking.
    for f in range(nframes) :
        rd.ftime =  time.ctime()
        if useTempControl == 'hardwareTempControl':
            # rd.pre_temp = "%7.3f" % ls332.readTemp()
            rd.pre_temp = ls332.readTemp()
        # make sure we can write it.. probably pyfits has a better way..
        sampfilename = filebase%(f+1)
        if os.access(sampfilename,6) : 
            os.remove(sampfilename) # delete it if it exists.
        frame = amcc.RawFrame(dev, nrows=nrow, ncols=ncol, prepix=det.prepix, postpix=det.postpix)
        if f == 0 :
            ped = frame
        print("acquiring buffer %d"%f, file=wfile)
        frame.grab()
        if dsp.dspthread.interrupted:
            print("got abort in sample up the ramp.")
            return
        if useTempControl == 'hardwareTempControl':
            #rd.post_temp = "%7.3f" % ls332.readTemp()
            rd.post_temp = ls332.readTemp()
            rd.tempB = ls332.readTempB()
        else:
            rd.pre_temp = tmptr.tdiode(dsp.adc2diode(frame.postdata[1]))
            rd.post_temp = tmptr.tdiode(dsp.adc2diode(frame.postdata[2]))
        dsp.dspthread.tc_in_acq(rd.pre_temp)
        # dd.temp = "%7.3f" % rd.pre_temp 
        #print frame.postdata[3]
        #print frame.postdata[4]
        #print frame.postdata[5]
        #print frame.postdata[6]
        rd.ltime =  time.ctime()
        fits.write_image(sampfilename, frame.data)
    fits.write_image(runfile, frame.data-ped.data)
    rd.objnum = xdir.objnum + 1

    print("saved to %s and others!"%runfile, file=wfile)
    rd.lc = "" # clear out local comment
    savesetup()

sutr = dspthreadable(sutr)

def ldrsutr(sampnum, extraoffset, wfile=sys.stdout, **kwds) :
    """
    Large Dynamic Range Run using Sample Up The Ramp
    This is a copy of the above sutr(), with added code near end.
    Syntax:
        ldrsutr(12, 200)
    will change the voffset by 200mV on the sample that ends in .012  
    Remember that this SUTR also counts up from zero.

    This command waits on the device driver and when a raw buffer has data in it,
    it passes the buffer to a fits object and writes it out.

    NOTE: This function is both high level and low level.
    Since sample up the ramp MUST acquire and write, there
    really is no way to separate them.
    """
    # Set as a src image
    src()
    if useTempControl == 'softwareTempControl':
        import tmptr
    if useTempControl == 'hardwareTempControl':
        import ls332

    from run import rd
    rd.sampmode = "sutr"
    nrow, ncol, nframes = rd.nrow, rd.ncol, rd.nsamp
    if nframes % 2 :
        print("nsamp must be even for sutr mode!", file=wfile)
        return

    BaseObjFile = xdir.get_nextobjfilename()
    filebase=BaseObjFile +"_%03d.fits"
    runfile=BaseObjFile +".fits"
    dsp.rampnext() # clocking program special behavior.
    # import amcc
    dev.clear_fifo() # flush out any pixels.
    dev.command(20,0) # tell the clock program its ok to start clocking.
    # amcc.clear_fifo() # flush out any pixels.
    # amcc.command(20,0) # tell the clock program its ok to start clocking.
    oldoffset = dd.voffset
    for f in range(nframes) :
        # Here we change the Voffset voltage to allow for larger signals
        # levels than are normally permitted given the A/D converter input
        # range and the preamp gain.  We make the change after the user
        # supplied frame number.
        if f == sampnum :
            dd.voffset = oldoffset - extraoffset  
            # While clocking the DSP (see ociw.command(20,0) above),
            # the normal way of changing a bias voltage does not work.
            # instead dd.voffset (or any dd.*) just changes the value in
            # the header and not on the actual pin.  So, we need to get
            # the attention of the DSP.
            dsp.writebias(10, dd.voffset)  # bias dacnum is 10 for Voffset
            #ociw.write
        # Get the current time for the header.
        rd.ftime =  time.ctime()
        if useTempControl == 'hardwareTempControl':
            # rd.pre_temp = "%7.3f" % ls332.readTemp()
            rd.pre_temp = ls332.readTemp()
        # make sure we can write it.. probably pyfits has a better way..
        sampfilename = filebase%(f+1)
        if os.access(sampfilename,6) : 
            os.remove(sampfilename) # delete it if it exists.
        frame = amcc.RawFrame(dev, nrows=nrow, ncols=ncol, prepix=dd.prepix, postpix=dd.postpix)
        if f == 0 :
            ped = frame
        print("acquiring buffer %d"%f, file=wfile)
        frame.grab()
        if dsp.dspthread.interrupted:
            print("got abort in sample up the ramp.")
            return
        if useTempControl == 'hardwareTempControl':
            #rd.post_temp = "%7.3f" % ls332.readTemp()
            rd.post_temp = ls332.readTemp()
        else:
            rd.pre_temp = tmptr.tdiode(dsp.adc2diode(frame.postdata[1]))
            rd.post_temp = tmptr.tdiode(dsp.adc2diode(frame.postdata[2]))
        dsp.dspthread.tc_in_acq(rd.pre_temp)
        # dd.temp = "%7.3f" % rd.pre_temp 
        #print frame.postdata[3]
        #print frame.postdata[4]
        #print frame.postdata[5]
        #print frame.postdata[6]
        rd.ltime =  time.ctime()
        fits.write_image(sampfilename, frame.data)

    fits.write_image(runfile, frame.data-ped.data)
    rd.objnum = xdir.objnum + 1

    print("saved to %s and others!"%runfile, file=wfile)
    rd.lc = "" # clear out local comment
    dd.voffset = oldoffset # Change Voffset back to original value.
    savesetup()

ldrsutr = dspthreadable(ldrsutr)

def crun(wfile=sys.stdout) :
    """
    Run the array continuously. Type 'burst' to abort. This command will return immediatly and run in the background.
    Type 'burst' to enter 'burst' mode which ends 'crun' mode.
    """
    import threading
    def crunfunc() :
        from run import rd
        try :
            # we want to return immediately if dsp is busy.
            # is there an easier way?
            bufs = []
            for i in range(rd.nsamp*2) :
                bufs.append(amcc.RawFrame(dev, rd.nrow, rd.ncol, prepix=det.prepix, postpix=det.postpix))
            print("CRUN mode. type 'burst' to abort", file=wfile)
            while True :
                time.sleep(0.05)
                dsp.dspthread.do(dsp.clockit, bufs=bufs)
                if dsp.dspthread.interrupted:
                    print("saw interrupt flag.")
                    raise KeyboardInterrupt
                # could update dv here?
        except StopIteration :
            print("DSP already busy.")
        except KeyboardInterrupt :
            print("abort.")
        except :
            import traceback
            traceback.print_exc()
            print("some other exception")
        
    # how about do_nowait??
    # typing crun twice will start two of these things!
    thread = threading.Thread(target=crunfunc)
    thread.setDaemon(True) # kills crun if we bail out.
    thread.start()
    time.sleep(0.1) # allow thread to start and print its stuff.
    
def scandir(names = ["itime","nsamp","vreset","dsub","TEMP","COMMENT1",] , wfile=sys.stdout) :
    """
    Writes a scandir text log for the acquired images in a run. It defaults to cataloging the following fields:
    "itime","nsamp","vreset","dsub","TEMP","COMMENT1".

    The list of names is a default one, and a different list
    could be passed in.
    """
    import astropy.io.fits as pyfits
    scanstrings=[]
    scanstrings.append("last file: "+ xdir.get_objfilename()+ '\n')
    scanstrings.append("file")
    for name in names :
        scanstrings.append( "\t"+ name)
    scanstrings.append('\n')
    for num in range(1, xdir.objnum+1) :
        f = pyfits.open(xdir.get_objfilename(num))
        scanstrings.append(repr(num))
        for name in names :
          try:
            scanstrings.append("\t" + repr(f[0].header[name]))
          except:
            scanstrings.append('\t')
        scanstrings.append('\n')
    print(''.join(scanstrings), file=wfile)

def bgscan() :
    """
    Perform a 'sscan' in a background thread. When invoked, the command will return immediately to a command prompt.
    """
    try: 
        dsp.dspthread.do_nowait(sscan) # try a scan.
    except StopIteration :
        print("Error. DSP executing %s."%dsp.dspthread.callable)
        return 
