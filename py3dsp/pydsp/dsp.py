"""
dsp.py
Duplicate of dsp_cpy.py. and moved old one to backup!
Encapulates command level access to the running 56303 dsp program.

This module deals with the command table that appears in the clock program.
Challenge faced here: different dsp programs need to
be talked to in different ways!

It depends upon the ociw module, which handles the physical link
(booting the dsp, serial communication, etc)
ociw does not know or care what is inside the clock program.
(well, should not anyway.)
This dependency is somewhat hardcoded.. although that is 
kinda hard to do in Python. ;-)

Could we make this communication a MONITOR?
We could just boot the monitor section, and replace
clocking programs without replacing the monitor!

Module Todo:
Check to see if clock program has been compiled.
if not, do that from here.
May want to break the command #defines out into a
separate file, and read the same file into both
C and Python. (Once and only once.)
(Apply the "command" pattern here?
Instead of writing this code, use template of command
various templates could be defined in the dsp's header
file. Populate command code in python dynamically
by reading the C header file.)
Check to see if the compiled clock program is already
running on the dsp. If so, don't re-download..
just re-sync??
This module is a candidate for a separate
thread.

""" 

"""
joseph:
I'd like to create an abstract layer for talking to the "dsp program"
over different media. Notably:
    - pci
    - dummy mode
    - usb*

The USB is new

"""

__version__ = """$Id: dsp.py 405 2019-05-21 15:12:11Z joseph $ """

__author__ = "$Author: joseph $"
 
__URL__ = "$URL: http://astro.pas.rochester.edu/svn/pydsp/trunk/pydsp/dsp.py $"

# import ociw_cpy as ociw
# import ociw
import amcc
import sys
import time
import sload
import logging as log

starttime = time.time()
dspthread = None

dev = None

_dummymode = False


# Logging stuff
logger = log.getLogger("pydsp.dsp")
logger.debug("reading dsp")

def dummymode():
    """
    Kick dsp into an offline mode for system use if no hardware available.
    """
    global _dummymode
    print('Setting to DUMMYMODE', file=sys.stdout)
    logger.debug("Setting Dummy Mode")
    _dummymode = True

    """
    # open ociw as a text file?
    import datetime, os
    timestr = datetime.datetime.now().strftime("%y%m%d-%H%M%S")
    fname = os.environ["PYDSPHOME"]+"/pydsp/dummy/"+timestr+"ociw0.txt"
    print "creating dummy ociw0: "+fname
    open(fname, 'a').close()
    ociw.open_ociw(os.environ["PYDSPHOME"]+"/pydsp/dummy/"+timestr+"ociw0.txt")


    def dummyread(dataexpected = True):
        if dataexpected:
            return 0 # or dummyread(dataexpected)
        else:
            raise SystemError # normal behaviour if fifo empty.

    ociw.read = dummyread 

    def dummy_load_srec(file):
        return 0

    ociw.load_srec = dummy_load_srec

    def NULL(*args,**kwargs):
        pass

    def dummyReset():
        print "Resetting dummy ociw"

    ociw.start = NULL
    
    ociw.reset = dummyReset
        
    ociw.data24 = NULL
    ociw.data16 = NULL
    ociw.command = NULL
    ociw.clear_fifo = NULL
    """

def dummymodeOff(device):
    pass
    # don't do anything until more fleshed out
    """
    global _dummymode 
    reload(ociw)
    ociw.open(device)
    _dummymode = False
    """

def read(dataexpected=1):#dataexpected = 1):
    """wrapper for read"""
    global dev
    # return dev.read(dataexpected)
    return dev.read(dataexpected)

def senddsp(val, cnum, **kwds):
    global dev
    "Send command (cnum) and value (val), to the clocking program."
    dev.data24(val)
    dev.command(cnum, 0)

def _writeX(address, data):
    """
    Write the specified 24 bit data word
    to the X memory at specified address. 
    """
    global dev
    dev.data24(data)
    dev.command(7, address)


def _writeY(address, data):
    """
    Write the specified 24 bit data word
    to the Y memory at specified address. 
    """
    global dev
    dev.data24(data)
    dev.command(8, address)

def _readX(address):
    """
    Read low 16 bits of data word
    from X memory at specified address. 
    """
    global dev
    dev.command(10, address)
    return dev.read()
    
def _readY(address):
    """
    Read low 16 bits of data word
    from Y memory at specified address. 
    """
    global dev
    dev.command(11, address)
    return dev.read()

def readAdcs():
    """ 
    issue the read adc command
    """
    global dev
    dev.command(14, 0)
    return dev.read(), dev.read(), dev.read(), dev.read()

def writeSeqBits(bitpat):
    """ 
    Write the 20 bit sequence register with a specific pattern.
    """
    SEQREG = 0x00FFFF88
    _writeY(SEQREG, bitpat)
    
def setSeqBits(bitpat=0x0FFFFF):
    """ 
    set only the bits in bitpat.
    leave the others unchanged.
    """
    SEQOR = 0x00FFFF89 # the or port allows you to set certain bits
    _writeY(SEQOR, bitpat)

def clearSeqBits(bitpat=0x0FFFFF):
    """ 
    clear only the bits that are SET in bitpat.
    leave the others unchanged.
    """
    SEQNAND = 0x00FFFF8A # the or port allows you to set certain bits
    _writeY(SEQNAND, bitpat)
    
def _writeheadbits(data, **kwds):
    """
    Write 4 header board control bits.
    (8 would be nice.)
    """
    HDR = 0x00FFFFC9 # HDR means Host port Data Register
    current = _readX(HDR)
    _writeX(HDR, (current&0x0FFF)|(data<<12))

def _readHeader() :
    """
    Read the header from the video inputs.
    This is a debug mode operation.
    """
    import time
    for i in range(16) :
        _writeheadbits(i)
        setSeqBits(0x0FFFF0)
        print(i, "****************")
        time.sleep(0.01)
        bi, bv, ci, cv = readAdcs()
        print(bv, cv, "set", end=' ')
        clearSeqBits(0x0FFFF0)
        time.sleep(0.01)
        bi, bv, ci, cv = readAdcs()
        print(bv, cv, "clear")

def _initheadbits():
    HDDR = 0x00FFFFC8 # HDDR means Host port Data Register
    current = _readX(HDDR)
    _writeX(HDDR, (current|0x00F002))

def writedac(dacnum, dacval, wfile=sys.stdout, **kwds):
    """Write a voltage in daccounts off of zero to a clock DAC (0-31)

    clips dac write if past rails"""
    global dev
    dacval += 4096 # and offset binary.
    if dacval < 0: # and a good place for sanity checks.
        dacval = 0 
        print(" clipping dac #%d write! %d too low! " % (dacnum, dacval), file=wfile)
    elif dacval > 8191:
        dacval = 8191
        print(" clipping dac #%d write! %d too high!" % (dacnum, dacval), file=wfile)
    dacval = int(dacval)
    dev.data24(dacval) # dac is offset, 0V = 4096
    dev.command(13, dacnum) # command 13 writes the data to the dac.

def writebias(biasnum, biasval, **kwds) :
    """
    write a voltage in dac counts off of zero to a bias DAC (0-15)
    """
    DACDATA = 0x00FFFF97
    BIAS0 = 0x00FFFF94
    BIAS1 = 0x00FFFF95
    biasval = int(biasval)
    biasval += 4096 # and offset binary.
    if biasval < 0: # a good place for sanity check.
        biasval = 0 
    elif biasval > 8191:
        biasval = 8191
    dacdata = ((biasnum & 0x7)<<13) | biasval
    _writeY(DACDATA, dacdata) # put both address and data in the register
    if biasnum & 8 :
        _writeY(BIAS1, 0) # clock both into max547
    else :
        _writeY(BIAS0, 0)
    
class ClockLine(object) :
    """
    Simple "bunch" object, Just holds related data.
    """
    def __init__(self, **args): 
        self.__dict__.update(args)


clocks = []
#
# THIS section tells how to map pin names to sequence bits
# 
clocks.append(ClockLine(dbpin=1, bpname="IPCA", dacs=(0,1), seqbit=4, headcode=8))
clocks.append(ClockLine(dbpin=2, bpname="RGA", dacs=(4,5), seqbit=5, headcode=10))
clocks.append(ClockLine(dbpin=3, bpname="SWA", dacs=(8,9), seqbit=12, headcode=12))
clocks.append(ClockLine(dbpin=4, bpname="TGA", dacs=(12,13), seqbit=13, headcode=14))

clocks.append(ClockLine(dbpin=9, bpname="IPCB", dacs=(2,3), seqbit=6, headcode=0))
clocks.append(ClockLine(dbpin=10, bpname="RGB", dacs=(6,7), seqbit=7, headcode=1))
clocks.append(ClockLine(dbpin=11, bpname="P2B", dacs=(28,29), seqbit=18, headcode=2))
clocks.append(ClockLine(dbpin=12, bpname="P3B", dacs=(30,31), seqbit=19, headcode=3))

clocks.append(ClockLine(dbpin=17, bpname="S1B", dacs=(18,19), seqbit=9, headcode=4))
clocks.append(ClockLine(dbpin=18, bpname="SWB", dacs=(10,11), seqbit=10, headcode=5))
clocks.append(ClockLine(dbpin=19, bpname="P1A", dacs=(20,21), seqbit=14, headcode=6))
clocks.append(ClockLine(dbpin=20, bpname="P2A", dacs=(22,23), seqbit=15, headcode=9))

clocks.append(ClockLine(dbpin=21, bpname="TGB", dacs=(14,15), seqbit=11, headcode=11))
clocks.append(ClockLine(dbpin=22, bpname="P3A", dacs=(24,25), seqbit=16, headcode=13))
clocks.append(ClockLine(dbpin=23, bpname="P1B", dacs=(26,27), seqbit=17, headcode=15))
clocks.append(ClockLine(dbpin=24, bpname="S3A", dacs=(16,17), seqbit=8, headcode=7))

clockpin2headcode = dict( [ (c.dbpin, c.headcode) for c in clocks] )

def _seeclockpin(pin, **kwds) :
    """
    Show clock pin on monitor.

    Outputs value of specified clock pin to black box output.
    """
    headcode = clockpin2headcode.get(pin, None)
    if headcode != None :
        _writeheadbits(headcode)
    else:
        print("unknown pin number")


biases = []

BiasLine = ClockLine

biases.append(BiasLine(dbpin=1, bpname="AX", dac=0, headcode=0))
biases.append(BiasLine(dbpin=2, bpname="BX", dac=1, headcode=1))
biases.append(BiasLine(dbpin=3, bpname="OGB", dac=2, headcode=2))
biases.append(BiasLine(dbpin=4, bpname="OGA", dac=3, headcode=3))
biases.append(BiasLine(dbpin=5, bpname="RDB", dac=4, headcode=4))
biases.append(BiasLine(dbpin=6, bpname="RDA", dac=5, headcode=5))
biases.append(BiasLine(dbpin=7, bpname="ODA", dac=6, headcode=6))
biases.append(BiasLine(dbpin=8, bpname="ODB", dac=7, headcode=7))

biases.append(BiasLine(dbpin=9, bpname="CX", dac=8, headcode=8))
biases.append(BiasLine(dbpin=10, bpname="DX", dac=9, headcode=9))
biases.append(BiasLine(dbpin=11, bpname="OGD", dac=10, headcode=10))
biases.append(BiasLine(dbpin=12, bpname="OGC", dac=11, headcode=11))
biases.append(BiasLine(dbpin=13, bpname="RDD", dac=12, headcode=12))
biases.append(BiasLine(dbpin=14, bpname="RDC", dac=13, headcode=13))
biases.append(BiasLine(dbpin=15, bpname="ODD", dac=15, headcode=14))

biaspin2headcode = dict( [ (b.dbpin, b.headcode) for b in biases] )

def _seebiaspin(pin, **kwds) :
    """
    Show bias pin on monitor.

    Outputs value of specified clock bias to black box output.
    """
    headcode = biaspin2headcode.get(pin, None)
    if headcode != None :
        _writeheadbits(headcode)
    else:
        print("unknown pin number")

def testClockRails():
    _initheadbits()
    import time
    for clock in clocks:
      try :
        writedac(clock.dacs[0], -50)
        writedac(clock.dacs[1], +50)
        time.sleep(0.1)
        for headcode in range(16):
            _writeheadbits(headcode)
            clearSeqBits(0x0FFFF0)
            time.sleep(0.01)
            bi, bv, ci, cv = readAdcs()
            if headcode != clock.headcode :
                assert -50 < cv < 50
            else :
                assert cv < -50
            _writeheadbits(headcode)
            setSeqBits(0x0FFFF0)
            time.sleep(0.01)
            bi, bv, ci, cv = readAdcs()
            if headcode != clock.headcode :
                assert -50 < cv < 50
            else :
                assert cv > 50
        print(clock.headcode,  "pass")
      except :
        print("error", clock.headcode, headcode, cv) 
        raise
      writedac(clock.dacs[0], 0)
      writedac(clock.dacs[1], 0)

def testBiases():
    _initheadbits()
    import time
    for bias in biases:
      try :
        writebias(bias.dac, -50)
        time.sleep(2.0)
        for headcode in range(16):
            _writeheadbits(headcode)
            time.sleep(0.1)
            bi, bv, ci, cv = readAdcs()
            if headcode != bias.headcode :
                assert -50 < bv < 50
            else :
                assert bv < -50
        print("pass", bias.headcode)
      except :
        print("error", bias.headcode, headcode, bv) 
        raise
      writebias(bias.dac, 0)

def _higain():
    """
    turn the attenuator off
    """
    HDR = 0x00FFFFC9 # HDR means Host port Data Register
    current = _readX(HDR)
    _writeX(HDR, (current&0x0FFF)&(~2))

def _logain():
    """
    turn the attenuator on
    """
    HDR = 0x00FFFFC9 # HDR means Host port Data Register
    current = _readX(HDR)
    _writeX(HDR, (current&0x0FFF)|2)

## teadBext stuff needs to move up higher.
# might go in run or runrun a little better.
#
# Problem: if we lose any pixels, this will hang.. forever!
# may need to know itime and frametime and have the PC
# bail out if it determines that something is wrong.
# right now, it seems the failure mode is extra pixels, not missing pixels.
# another idea is to send the checksum again and again at the end of the acq
# and have the driver compute its own.
# when the driver sees all pixels are in, it can tell the dsp
# "no more checksums, please"
 
startfuncs = []
stopfuncs = []

def startingAlert():
    "Calls observers who want to know that clocking has started."
    for func in startfuncs:
        func()

def stoppingAlert():
    "Calls observers who want to know that clocking has stopped."
    for func in stopfuncs:
        func()

def clockit(bufs, wfile=sys.stdout, **kwds):
    """Acquire an image into the driver buffer.

    first, tell device driver to get ready.
    then, tell clock program to go.
    then, sit and wait for the dsp 
    (all the while reading and displaying pixels to go.)
    separate thread would be nice.
    """
    global starttime, dev
    starttime = time.time()
    dev.clear_fifo()
    dev.command(20, 0) # tell the clock program its ok to start clocking.
    startingAlert() # tell any observers too!
    # print "pixels to go:"
    for buf in bufs:
        buf.grab()
        if dspthread.interrupted:
            print("aborting", file=wfile)
            stoppingAlert()
            raise KeyboardInterrupt
    else:
        stoppingAlert()

def rnorm(**kwds):
    "Set reset to normal, use ron to set always-on reset"
    senddsp(0,28) # reset image is normal, i.e. reset, then read-read

def ron(**kwds):
    "Set reset to alwayson, use rnorm to revert to normal reset"
    senddsp(1,28) # reset image 28 is always on = 1, 

def rrow(**kwds):
    "Set to reset row mode"
    senddsp(0,29) # reset mode 29 is row reset

def rglobal(**kwds):
    "Set to reset global mode"
    senddsp(1,29) # reset mode 29 is global reset

def rampnext(**kwds):
    "Get the itime between each sample for the next image"
    global dev
    dev.command(0, 11)

def singlepix():
    "Tell clock program to do single pixel (no col/row clocks) multiple read"
    global dev
    dev.command(0, 14)

def tweak_heater(heater):
    """
    Single word command to bump heater up or down a little bit.
    current heater voltage must be near the new voltage.
    +/- 100 dac counts.
    """
    global dev
    heater &= 0x00ff # just send down the low byte.
    dev.write(256*heater + 27) # 27 is TWEAK_HEATER command

def set_heater_by_tweak(desired_mv):
    from det import dd
    current_mv = dd.heater
    desired_mv = min( desired_mv, current_mv + 100)
    desired_mv = max( desired_mv, current_mv - 100)
    daccounts = desired_mv * 0.5 
    daccounts += 4096
    tweak_heater(int(daccounts))
    dd.data['heater'] = int(desired_mv)
    gname = dd.namemap.get('heater','heater')
    dd.set_widget(gname +"_txt", str(int(desired_mv)))

def getFrameTime(**kwds):
    "Return actual time of last frame.\nRequires clock code to cooperate."
    dev.command(0, 12)
    frametime = dev.read()
    try:
        # might want common code for checking fifo.
        dev.read(False) # should fail
    except SystemError: # should raise this exception
        return frametime # and this is a normal return
    raise RuntimeError

def getPedTime(**kwds):
    "Return actual time of nsamp pedestal frames.\n"
    "Requires clock code to cooperate."
    dev.command(0, 13)
    frametime1 = dev.read()
    frametime2 = dev.read()
    frametime = (frametime1&0x0000ffff) + ((frametime2&0x0000ff)<<16)
    try:
        # might want common code for checking fifo.
        dev.read(expected = False) # we expect no data
    except: # exception is normal
        return frametime # this is normal return
    raise RuntimeError ##  this is abnormal

def timer(wfile=sys.stdout, **kwds):
    "Return the current value of the dsp timer. no error checking."
    dev.command(22, 0)
    time.sleep(0.05) # give the DSP a little time!
    print(dev.read(), file=wfile) # possible that other stuff was in fifo!

def read7888(chan, **kwds):
    "Read and return last 7888 ADC conversion and setup next conversion"
    try:
        dev.read(False) # Should raise SystemError
        # if we get here? not good!
        raise RuntimeError("ADC: unexpected fifo data.")
        # there is data in the FIFO that should not be there!
    except SystemError:
        pass # hitting this is actually normal.
    dev.command(25, chan<<3) # command 25 is "read 7888"
    time.sleep(0.02) # give the DSP a little time. 0.01 too fast.
    try:
        retval = dev.read()
        return retval
    except:
        return None

def readBiasCurrent(**kwds):
    " read the bias current in microamps"
    read7888(1)
    v = read7888(1)
    if v != None:
        v = (v-2048) * 0.24425
    return v

def readBiasVoltage(**kwds):
    "Read the bias voltage in volts"
    read7888(3)
    v = read7888(3)
    if v != None:
        v = (v-2048) * 0.0048925
    return v

def readClockVoltage(**kwds):
    " read the clock current in microamps"
    read7888(5)
    v = read7888(5)
    if v != None:
        v = (v-2048) * 0.0048925
    return v

def readClockCurrent(**kwds):
    " read the clock voltage in volts"
    read7888(7)
    v = read7888(7)
    if v != None:
        v = (v-2048) * 0.24425
    return v

def adc2diode(n_adc):
    "Convert the hi res temp dac reading to a diode voltage"
    # (this func smarts of a more generic utility.)
    v_diode = 0.625 + n_adc * 0.0001526
    v_diode -= 0.005  # -5mV tweak, at 80K anyway.
    return v_diode

def vDiode(**kwds):
    "Read the diode voltage. currently just with the hi-res temp input"
    read7888(0) # prime the pump
    n_adc = read7888(0)# get a good reading
    if n_adc < 20: # clip 
        n_adc = 20
    if n_adc > 4092:
        n_adc = 4092
    return adc2diode(n_adc)    

def sciStepN(numsteps, wfile=sys.stdout, **kwds):
    """Send numsteps to the IM483 stepper motor controller 
    connected to the SCI port on the DSP. """
    try:
        dev.read()
        print("data in fifo!", file=wfile)
    except:
        pass
    dev.data24(numsteps) # 
    dev.command(26, 0) # command 26 tells dsp to move stepper motor.
    time.sleep(0.1 + abs(numsteps) * 0.008 ) # wait for motion to complete.
    try:
        dev.read()
    except:
        print("no stepper handshake.", file=wfile)

# DSP Thread.
# all access to the dsp must happen sequentially.
# We want other things to stay alive while communication
# to the dsp is still in process.
# Rather than allow any thread access to the dsp, and
# handling serialization by locking, we use a single thread
# and pass callables as messages into that thread.

import threading 
class DspThread(threading.Thread):
    """Dsp object. singleton. runs as thread.

    Inspired by page 284 of Python In a Nutshell."""

    def __init__(self): 
        import queue
        threading.Thread.__init__(self)
        self.workRequestQueue = queue.Queue(maxsize=1)
        self.semaphore = threading.Semaphore() # we WISH it was an rlock.
        self.callablename = ''
        self.then = time.time()
        self.last_hb = time.time()
        self.tc = None
        self.done = False
        self.interrupted = False 
        self.daemon = True # close thread after main dies
        self.start() # make thread active and run() separately.
        import atexit
        atexit.register(self.shutdown)
        
    def shutdown(self) :
        """
        shut down in an orderly fashion.
        """
        self.done = True

    def do_nowait(self, callable, *args, **kwds):
        """ make a request on the dsp thread.
            when request is accepted, return a unique event 
            that will allow caller to sleep until the task is done
            if request is rejected, raise StopIteration
        """
        self.semaphore.acquire(True)
        self.callablename = callable.__name__
        event = threading.Event()
        self.workRequestQueue.put((callable, event, args, kwds)) 
        return event

    def blocking_do_nowait(self, callable, *args, **kwds):
        """ make a request on the dsp thread.
            if request is accepted, 
            return a unique event 
            that will allow caller to sleep until the task is done
            if request is rejected, raise StopIteration
        """
        if not self.semaphore.acquire(False):
            print(callable.__name__, "was blocked by", self.callablename)
            sleep(1)
            return None

        self.callablename = callable.__name__
        event = threading.Event()
        self.workRequestQueue.put((callable, event, args, kwds)) 
        return event
    def do(self, callable, *args, **kwds):
        """execute callable on dspthread

        return its result, or raise its exception.""" 
        if threading.current_thread() is self.thread: 
            # if no thread switch, act
            return callable(*args, **kwds)
        else:
            event = self.do_nowait(callable, *args, **kwds)
            if not event:
                return None # error condition here.
            while True:
                # can't hang the main thread.
                # must be active to handle signals.
                event.wait(timeout = 0.1) # why not an infinite timeout?
                if event.isSet():
                    break
            if (isinstance(event.result, tuple) and
                        len(event.result) == 3 and
                        isinstance(event.result[1], Exception)) :
                import traceback
                traceback.print_exception(*event.result)
                raise event.result[1]
            else:
                return event.result

    def interrupt(self, source="unspecified"):
        "Tell the dsp thread to halt execution."
        # Need to exit when ociw is just a dummy
        self.interrupted = True
        print(source, "is interrupting")
        # dev.write(0)
        # time.sleep(2.0)
        # dev.clear_fifo()
        dev.write(0)
        time.sleep(2.0)
        dev.clear_fifo()
        print("fifo cleared")
        raise KeyboardInterrupt

    def run(self):
        """
        The main loop of the hardware serializer.
        do NOT call directly. This IS the dsp thread.
        """
        self.thread = threading.currentThread() # what thread are we on?
        import queue
        while True:
            # Heartbeat : this was a bad idea
            """
            now = time.time()
            if now - self.last_hb > 1:
                logger.debug("DSP Thred Heartbeat: {}".format(now))
                self.last_hb = now
            """

            # Event Handler
            if not self.workRequestQueue.empty():
            #try: 
                # if we have a command
                callable, event, args, kwds = self.workRequestQueue.get(False)
            else: #except Queue.Empty: (Lets not use errors as control flow - joe)
                # this is a bit of a hack.. (Ya think! - joe)
                # might be better to post temp control events to the queue!
                if self.done:
                    return
                time.sleep(0.001)
                if not self.tc : 
                    continue

                # If temperature control
                now = time.time()
                if now - self.then > 5 :
                    # ok, we are idle.
                    from det import dd
                    from tmptr import tdiode
                    try:
                        self.tc.current_temp = tdiode(vDiode())
                    except RuntimeError: # XXX automatic test that this works?
                        if self.interrupted : continue
                        logger.warning('unexpected data. clearing.')
                        dev.command(0, 0)
                        time.sleep(1)
                        dev.clear_fifo()
                        self.then = now
                        continue
                    dd.temp = "%7.3f" % self.tc.current_temp
                    self.tc.adjustCarrot()
                    watts = self.tc.watts()
                    volts = self.tc.volts(watts)
                    volts = int(volts*1000)
                    if 0000 < volts < 4700:
                        dd.heater = volts
                    else :
                        log.warning("excess volts!: {}".format(volts))
                        self.tc.goal = self.tc.current_temp
                    self.then = now
                continue
            # long callables check interrupted flag
            # and raise exception if interrupt happened.
            self.interrupted = False 
            try:
                event.result = callable(*args,**kwds)
            except:
                event.result = sys.exc_info()
            self.callablename = '' # 
            # release semaphore BEFORE setting event!
            # that way, a sleeping loop will not
            # wake up to a locked semaphore
            self.semaphore.release() # perhaps belongs elsewhere?
            event.set() # we are done with this

    def tc_in_acq(self, current_temp) :
        """
        Temperature control during acquisition.

        passes the current temp into the temp controller,
        adjusts the desired temperature (the carrot, ie goal we follow)
        determines the new heater power / voltage to send,
        and sends the heater command.
        """
        from det import dd
        from tmptr import tdiode
        if not self.tc:
            return
        self.tc.current_temp = current_temp
        dd.temp = "%7.3f" % self.tc.current_temp
        self.tc.adjustCarrot()
        watts = self.tc.watts()
        volts = self.tc.volts(watts)
        volts = int(volts*1000)
        if 0000 < volts < 4700: # XXX hardcoded heater max voltage.
            set_heater_by_tweak(volts)
        else :
            print("excess volts!", volts)
            self.tc.goal = self.tc.current_temp

    def threadable(self, func):
        """
        helper method to make a function threadable, OOP style.
        """
        def threaded(*args, **kwds):
            """
            Wrapped function. see source code for real function.
            """
            return self.do(func, *args, **kwds)
        if True: # setting the name of a function. 
                #Python Cookbook 2e, page 745
            #import new # this is actually deprecated
            import types
            f = types.FunctionType(threaded.__code__, threaded.__globals__, 
                func.__name__, threaded.__defaults__, threaded.__closure__)
            f.__doc__ = func.__doc__
            #f.func_code.co_filename = func.func_code.co_filename # arg! read only!
            return f
        else:
            # threaded.__name__ = func.__name__ # arrg! read only in 2.3!
            threaded.__doc__ = func.__doc__
            return threaded


def dspthreadable(func):
    """
    helper function to make a function threadable.

    This can make functions threadable without having the dsp thread alive.
    """
    # Q: should this be a method of DspThread?
    # it currently assumes a single global instance of DspThread
    # called dspthread.
    # A: well, perhaps for purity. 
    # but when would you have more than one instance
    # of the DspThread? Don't rule it out, I guess.
    def threaded(*args, **kwds):
        """
        Wrapped function. see source code for real function.
        """
        return dspthread.do(func, *args, **kwds)
    if True: # setting the name of a function. 
             #Python Cookbook 2e, page 745
        #import new # this is actually deprecated
        import types
        f = types.FunctionType(threaded.__code__, threaded.__globals__, 
            func.__name__, threaded.__defaults__, threaded.__closure__)
        f.__doc__ = func.__doc__
        #f.func_code.co_filename = func.func_code.co_filename # arg! read only!
        return f
    else:
        # threaded.__name__ = func.__name__ # arrg! read only in 2.3!
        threaded.__doc__ = func.__doc__
        return threaded

def initialize(hw_mode='pci'):
    """
    Sets up the hardware interface. Can choose between Simulated dsp (sim),
    usb mode (usb), and pci mode (pci)
    """
    
    global dev, dspthread
    dspthread = DspThread()

    if hw_mode == 'sim':
        import socket
        logger.debug("Not doing dummy mode and instead going to connect with sim.")
        try:
            dev = amcc.SimDevice()
        except socket.error:
            logger.warning("Can't connect to server starting in dummy mode")
            dummymode()
            dev= amcc.DummyDevice()
    elif hw_mode == 'usb':
        try:
            dev = amcc.USBDevice()
        except:
            logger.error("couldn't open usb, switching to dummy interface")
            dev = amcc.DummyDevice()
            #?dummymode()

    else: 
        try:
            dev = amcc.PCIDevice()
            dev.open()
            
            # dev = amcc.open()# "/etc/udev/devices/ociw0")
            if dev == None:
                logger.error("Device is loading as None!")
                # dummymode()
        except (AttributeError, SystemError) :
            logger.error("Error opening PCI card. need to insmod driver??")
            logger.error("setting dsp interface to dummy mode.")
            dummymode()

    return dev



# XXX convert following to function decorators when migrating to python 2.4!!
dev = initialize()
load_srec = dspthreadable(sload.sloader(dev).load_srec)
senddsp = dspthreadable(senddsp)
writedac = dspthreadable(writedac)
clockit = dspthreadable(clockit)
rnorm = dspthreadable(rnorm)
ron = dspthreadable(ron)
rrow = dspthreadable(rrow)
rglobal = dspthreadable(rglobal)
rampnext = dspthreadable(rampnext)
getFrameTime = dspthreadable(getFrameTime)
getPedTime = dspthreadable(getPedTime)
timer = dspthreadable(timer)
read7888 = dspthreadable(read7888)
readBiasCurrent = dspthreadable(readBiasCurrent)
readBiasVoltage = dspthreadable(readBiasVoltage)
readClockCurrent = dspthreadable(readClockCurrent)
readClockVoltage = dspthreadable(readClockVoltage)
vDiode = dspthreadable(vDiode)
sciStepN = dspthreadable(sciStepN)
_writeheadbits = dspthreadable(_writeheadbits)
writebias = dspthreadable(writebias)
seeclockpin = dspthreadable(_seeclockpin)
seebiaspin = dspthreadable(_seebiaspin)
