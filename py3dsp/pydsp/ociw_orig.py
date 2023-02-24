"""
ociw.py: lowest-level interface to the device driver and the running DSP.

no real dependencies, but some of it assumes
a certain data protocol supported by the running dsp program.

It also assumes a single PCI card. A more generic approach would create
an object for each card. The driver would need a revisit in this
situation.

this module, and sload.py, replaced the ociw.so Python extension module
that was written in C. 
"""

__revision__ = """$Id:$"""


fd = None
dev = None


def open_ociw(devname="/dev/amcc0"): #""/etc/udev/devices/ociw0"):
    import io
    "Open the device. (perhaps a bad name, Python has a built-in named open.)"
    global fd
    global dev
    if not dev:
        try:
            dev = open(devname, mode="r+b", buffering=0)
                # + lets your write at the same time as read.
                # buffering must be False to open the device more than once.
                # which is a bad idea, actually.
            fd = dev.fileno()
            print("opened", devname)
        except:
            print("can't open device!")
            print("you may need to make the device: command is")
            print("mknod /etc/udev/devices/ociw0 c 125 0")
            print("change permissions accordingly.")
            dev = None
            fd = None
            raise(SystemError)    
    return dev

def write(i, dev=dev):
    "Write one 16 bit word to the device."
    import array
    a = array.array("H", [i&0xffff])
    dev.write(a)

def read(expected=True, dev=dev):
    "Read one 16 bit word from the device."
    import array
    s = dev.read(2)
    if len(s) == 2:
        return array.array('h', s)[0]
    else:
        raise SystemError("Data timeout")

def reset(dev=dev):
    "Reset the device."
    import fcntl
    if dev == None or fd == None:
        raise(SystemError)

    for i in range(100): # hack for old board.
        fcntl.ioctl(dev, 0x7a00) # ioctl for a reset operation
# ioctl = xxxx7axx
# 7a = hex(ord('z'))
# 'z' is the magic byte in amcc.h

# methods from here on out assume a running clock program.
# The clock program must be running a certain protocol to
# interpret the commands correctly.
SET_DATA_MS = 1
SET_DATA_NS = 2
SET_DATA_LS = 3
SET_ADDR_MS = 4
SET_ADDR_NS = 5
SET_ADDR_LS = 6

def command(cmd, addr, dev=dev):
    """Write 8 bit command and 24 bit address to the clock program.
    wasteful for commands with only 8 bits of data."""
    import array
    a = array.array("H", [
        ((addr>>8)&0xff00) | SET_ADDR_MS, 
        (addr & 0xff00) | SET_ADDR_NS,
        ((addr<<8)&0xff00) | cmd
         ])
    dev.write(a)

def data24(data, dev=dev):
    "Write a 24 bit data word to the clock program."
    import array
    a = array.array("H", [
        ((data&0x00ff0000)>>8) | SET_DATA_MS,
        (data & 0x00ff00) | SET_DATA_NS,
        ((data&0x00ff)<<8) | SET_DATA_LS
         ])
    dev.write(a)

def clear_fifo(dev=dev):
    "make sure nothing is in the fifo"
    import fcntl
    fcntl.ioctl(dev, 0x7a12); # 0x12 == 18 == AMCC_CLR_FIFO

def peek_fifo(dev=dev):
    "see how much is in the fifo, without reading it."
    import fcntl
    import array
    HEX80047A13 = -2147190253 
    a = array.array("L",[0])
    fcntl.ioctl(dev, HEX80047A13, a, True) # 0x13 == 19 = AMCC_GET_FIFO
    # 
    return int(a[0]) 

def peek_ptcr(dev=dev) :
    import fcntl
    import array
    HEX80047A07 = -2147190265 
    a = array.array("L",[4])
    fcntl.ioctl(dev, HEX80047A07, a, True) 
    return int(a[0]) 

def peek_rcr(dev=dev) :
    import fcntl
    import array
    HEX80047A05 = -2147190267 
    a = array.array("L",[0])
    fcntl.ioctl(dev, HEX80047A05, a, True) 
    return int(a[0]) 

def peek_nvram(addr) :
    pass
    # wait for rcr[31] to clear.
    # rcr[29:] = 100b
    # rcr[16:24] = lowaddr byte
    # rcr[29:] = 101b
    # rcr[16:24] = highaddr byte
    # rcr[29:] = 111b
    # wait for rcr[31] to clear.
    # return = rcr[16:24]

def poke_rcr(val, dev=dev) :
    import fcntl
    import array
    HEX00047A14 = 293396
    a = array.array("L",[0])
    a[0] = val
    fcntl.ioctl(dev, HEX00047A14, a, True) 

def interrupted() :
    """RawFrame objects call interrupted
    in all data acquisition loops
    to see if an interrupt has occurred.
    This is how acquisition is aborted.

    The default implementation does nothing.

    Override this function with something that actually works."""
    return False

class AbstractFrame:
    """
    unused design concept.
    left here as food for thought.
    """
    # any abstractions here?
    # abstract creation behavior?
    # create a frame object, tell it its metadata.
    # grab should do the physical acquisition.
    # grab will almost certainly use metadata
    # and may also update metadata.
    # the grab may fail.
    # Is the frame always two dimensional image data?
    # It might be audio?
    # It might be an image cube?
    # It might be ???
    # grab may want to know when to expect data to start
    # grab may want to know when to expect data to stop.
    # grab may want to know when to give up if no data arrives.
    # grab may need to give up even if data does arrive.
    # it is almost like TCP?? seems to be a packet protocol type thing
    # we connect, get the chunk of data
    # there seems to be very little abstract behavior here.
    def __init__(self, **kwds):
        self.__dict__.merge(kwds)

    def grab(self):
        raise NotImplementedError

import time
import sys
class RawFrame:
    """Image data acquisition object. Acquires a single frame.
    Instantiate it with nrows, ncols, prepix, and postpix.
    possibly add special funcs to the base class.
    have the object acquire by calling its grab function. 
    it returns the image data in a string.
    tag it with metadata.
    The public interface is the grab method.
    Interested objects can subscribe to events before and
    after the data has come in.
    """

    prefuncs = [] # these funcs are called EVERY instance
    funcs = [] # likewise

    def __init__(self, nrows, ncols, prepix=0, postpix=0):
        """
        create a new acquisition object.
        each frame has a size expressed in rows and columns,
        and a few pixels that always precede or follow the main frame.

        Each instance makes a copy of the base class' list of subscribers
        and allows the user to add new functions to this list
        or perhaps delete the ones that are there.
        """
        self.nrows = nrows
        self.ncols = ncols
        self.nbytes = nrows * ncols * 2
        self.prebytes = prepix * 2
        self.postbytes = postpix * 2
        self.bytelist = []
        self.read = dev.read
        self.funcs = RawFrame.funcs[:] # instance observers
        self.prefuncs = RawFrame.prefuncs[:] # instance observers

    def preframeAlert(self):
        """Publish-subscribe hook. Called BEFORE frame is started."""
        for func in self.prefuncs:
            func(self) # call instance observers

    def frameAlert(self):
        """Publish-subscribe hook. Called AFTER frame is successfully in."""
        for func in self.funcs:
            func(self) # call instance observers

    def grab_predata(self):
        """
        Acquire the preceding pixels for the image frame.
        """
        # if we aren't expecting data for a while, sleep??
        # should hang here during itime.
        predata = self.read(self.prebytes)
        bytesleft = self.prebytes - len(predata)
        #print bytesleft
        while bytesleft > 0 and not interrupted():
            #print bytesleft
            time.sleep(0.1)
            # print "\r",
            # sys.stdout.flush()
            predata += self.read(bytesleft)
            bytesleft = self.prebytes - len(predata)
        import array
        self.predata = array.array('h', predata)[:]

    def grab_data(self):
        """
        Acquire the main data for the frame.
        """
        import numpy
        self.bytelist = [] # 
        for row in range(self.nrows):
            data = self.read(self.ncols*2)
            if data: self.bytelist.append(data)
            bytesleft = self.ncols*2 - len(data)
            #print row, bytesleft
            while bytesleft > 0 and not interrupted():
                #print row, bytesleft
                time.sleep(0.1)
                data = self.read(bytesleft)
                if data:
                    self.bytelist.append(data)
                bytesleft = self.ncols*2 - len(data)
            if interrupted():
                self.data = None
                break
        else:
            self.data = numpy.ndarray(shape=(self.nrows, self.ncols), dtype=numpy.int16, buffer="".join(self.bytelist))
        del self.bytelist

    def grab_postdata(self):
        """
        Acquire the trailing pixels for the image frame.
        """
        postdata = self.read(self.postbytes)
        bytesleft = self.postbytes - len(postdata)
        #print bytesleft
        while bytesleft > 0 and not interrupted():
            time.sleep(0.1)
            postdata += self.read(bytesleft)
            bytesleft = self.postbytes - len(postdata)
            #print bytesleft
        import array
        self.postdata = array.array('h', postdata)[:]

    def grab(self):
        """
        Acquire the complete frame.
        Most users should just call this directly.
        """
        import numpy
        self.preframeAlert()
        #print 'preframeAlert done'
        self.grab_predata()
        #print 'predata done'
        self.grab_data()
        #print 'data done'
        self.grab_postdata()
        #print 'postdata done'
        if not interrupted():
            self.checksum = (numpy.sum(self.data.flat) + sum(self.predata))
            if self.checksum&0x0ffff != self.postdata[-1]&0x0ffff:
                raise RuntimeError("checksum error")
            self.frameAlert() # tell observers that a frame is done.
