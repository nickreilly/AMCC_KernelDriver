"""
amcc.py is a remake of ociw.py: lowest-level interface to the device driver and the running DSP.
- I'm formulating this file as an interface for the different ways I want
to communicate (pci, dummy, usb)

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

# methods from here on out assume a running clock program.
# The clock program must be running a certain protocol to
# interpret the commands correctly.
SET_DATA_MS = 1
SET_DATA_NS = 2
SET_DATA_LS = 3
SET_ADDR_MS = 4
SET_ADDR_NS = 5
SET_ADDR_LS = 6

import logging as log

logger = log.getLogger("pydsp.amcc")

class PCIDevice:
    def __init__(self):
        # self.open()
        self.fd = None
        self.dev = None
        self.devname = '/dev/amcc0'

    def open(self):
        try:
            self.dev = open(self.devname, mode="rb+", buffering=0)
                # + lets your write at the same time as read.
                # buffering must be False (0) to open the device more than once.
                # which is a bad idea, actually.
            self.fd = self.dev.fileno()
            if self.fd is None:
                raise SystemError
            print(f"opened {self.devname}, fd: {self.fd}")
        except:
            print("can't open device!")
            print("you may need to make the device: command is")
            print("mknod /dev/amcc0 c 125 0")
            print("change permissions accordingly.")
            self.dev = None
            self.fd = None
            raise(SystemError)


    def write(self, i):
        "Write one 16 bit word to the device."
        # global dev
        import array
        a = array.array("H", [i & 0xffff])
        self.dev.write(a)

    def read(self, expected=True):
        "Read one 16 bit word from the device."
        import array

        s = self.dev.read(2)
        # return s
        # print(s)
        if len(s) == 2:
            # print(len(s))#.shape)
            return array.array('h', s)[0]
        else:
            raise SystemError("Data timeout")
            # return s

    def reset(self):
        "Reset the device."
        import fcntl
        if self.dev == None or self.fd == None:
            raise(SystemError)

        for i in range(100): # hack for old board.
            fcntl.ioctl(self.dev, 0x7a00) # ioctl for a reset operation
    # ioctl = xxxx7axx
    # 7a = hex(ord('z'))
    # 'z' is the magic byte in amcc.h

    def command(self, cmd, addr):
        """Write 8 bit command and 24 bit address to the clock program.
        wasteful for commands with only 8 bits of data."""
        import array
        a = array.array("H", [
            ((addr>>8)&0xff00) | SET_ADDR_MS, 
            (addr & 0xff00) | SET_ADDR_NS,
            ((addr<<8)&0xff00) | cmd
             ])
        self.dev.write(a)

    def data24(self, data):
        "Write a 24 bit data word to the clock program."
        import array
        a = array.array("H", [
            ((data & 0x00ff0000)>>8) | SET_DATA_MS,
            (data & 0x00ff00) | SET_DATA_NS,
            ((data & 0x00ff)<<8) | SET_DATA_LS
             ])
        self.dev.write(a)

    def clear_fifo(self):
        "make sure nothing is in the fifo"
        import fcntl
        fcntl.ioctl(self.dev, 0x7a12)  # 0x12 == 18 == AMCC_CLR_FIFO

    def peek_fifo(self, val=0):
        "see how much is in the fifo, without reading it."
        import fcntl
        import array
        HEX80047A13 = -2147190253
        a = array.array("L", [val])
        fcntl.ioctl(self.dev, HEX80047A13, a, True)  # 0x13 == 19 = AMCC_GET_FIFO
        return int(a[0])

    def peek_ptcr(self):
        import fcntl
        import array
        HEX80047A07 = -2147190265
        a = array.array("L", [4])
        fcntl.ioctl(self.dev, HEX80047A07, a, True)
        return int(a[0])

    def peek_rcr(self) :
        import fcntl
        import array
        HEX80047A05 = -2147190267
        a = array.array("L", [0])
        fcntl.ioctl(self.dev, HEX80047A05, a, True)
        return int(a[0]) 

    def peek_nvram(self, addr):
        pass
        # wait for rcr[31] to clear.
        # rcr[29:] = 100b
        # rcr[16:24] = lowaddr byte
        # rcr[29:] = 101b
        # rcr[16:24] = highaddr byte
        # rcr[29:] = 111b
        # wait for rcr[31] to clear.
        # return = rcr[16:24]

    def poke_rcr(self, val):
        import fcntl
        import array
        HEX00047A14 = 293396
        a = array.array("L", [0])
        a[0] = val
        fcntl.ioctl(self.dev, HEX00047A14, a, True) 

    def interrupted(self):
        """RawFrame objects call interrupted
        in all data acquisition loops
        to see if an interrupt has occurred.
        This is how acquisition is aborted.

        The default implementation does nothing.

        Override this function with something that actually works."""
        return False

class DummyDevice:
    def __init__(self):
        logger.info("Initing a dummy device")
        self.dev = None

    def open(self):
        logger.debug("dummy open")
        pass

    def write(self, i):
        logger.debug("dummy write: {}".format(i))
        pass

    def read(self, expected=True):
        logger.debug("dummy read")
        pass

    def reset(self):
        logger.debug("dummy reset")
        pass

    def command(self, cmd, addr):
        logger.debug("dummy command")
        pass

    def data24(self, data):
        logger.debug("dummy data24")
        pass

    def clear_fifo(self ):
        logger.debug("dummy clear_fifo")
        pass

    def peek_fifo(self ):
        logger.debug("dummy peek_fifo")
        return 0

    def peek_ptcr(self ) :
        logger.debug("dummy peek_ptcr")
        return 0

    def peek_rcr(self ) :
        logger.debug("dummy peek_rcr")
        return 0

    def peek_nvram(self, addr) :
        logger.debug("dummy peek_nvram")
        return 0
        
    def poke_rcr(self, val) :
        log.debug("dummy poke_rcr")
        pass

    def interrupted(self ) :
        log.debug("dummy interrupted")
        return False

class SimDevice:
    def __init__(self):
        logger.info("Initing simulation connection")
        import socket
        serv_addr = "../dev_sim/sock_sim.sock"
        self.s = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        logger.debug("Attempting to connect to sim server")
        self.s.connect(serv_addr)
        logger.debug("Connection sucessful")
        self.s.send("Test: The mangy red fox")

    def open(self):
        logger.debug("SimDevice: open")

    def write(self, i):
        "Write one 16 bit word to the device."
        logger.debug("SimDevice: write: {}".format(i))
        self.s.send(str(i))

    def read(self, expected=True):
        logger.debug("SimDevice:read")
        return 0

    def reset(self):
        logger.debug("SimDevice:reset")
        pass

    def command(self, cmd, addr):
        logger.debug("SimDevice: command")
        pass

    def data24(self, data):
        logger.debug("SimDevice: date24")
        pass

    def clear_fifo(self ):
        logger.debug("SimDevice: clear fifo")
        pass

    def peek_fifo(self ):
        logger.debug("SimDevice: peed fifo")
        return 0

    def peek_ptcr(self ) :
        logger.debug("SimDevice: print ptcr")
        return 0

    def peek_rcr(self ) :
        logger.debug("SimDevice: peek rcr")
        return 0

    def peek_nvram(self, addr) :
        logger.debug("SimDevice: peek nvram")
        return 0
        
    def poke_rcr(self, val) :
        logger.debug("SimDevice: poke rcr")
        pass

    def interrupted(self ) :
        logger.debug("is Sim device interrupted?")
        return False

class USBDevice:
    def __init__(self):
        self.dev = None
        self.open()

    def open(self):
        import usb.core, usb.util
        # Default USB IDs for device
        VID = 0x04b4 # Cypress Semiconductor
        PID = 0x8051 # The CY8-blahblah chip
        print("finding usb")
        self.dev = usb.core.find(idVendor=VID, idProduct=PID)

        if self.dev is None:
            print("Couldn't find USB device")
            raise SystemError
        else:
            print("found usb")


    def write(self, i):
        pass

    def read(self, expected=True):
        pass

    def reset(self):
        pass

    def command(self, cmd, addr):
        pass

    def data24(self, data):
        pass

    def clear_fifo(self ):
        pass

    def peek_fifo(self ):
        return 0

    def peek_ptcr(self ) :
        return 0

    def peek_rcr(self ) :
        return 0

    def peek_nvram(self, addr) :
        return 0
        
    def poke_rcr(self, val) :
        pass

    def interrupted(self ) :
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
    # global dev

    prefuncs = [] # these funcs are called EVERY instance
    funcs = [] # likewise

    def __init__(self, dev, nrows, ncols, prepix=0, postpix=0):
        """
        create a new acquisition object.
        each frame has a size expressed in rows and columns,
        and a few pixels that always precede or follow the main frame.

        Each instance makes a copy of the base class' list of subscribers
        and allows the user to add new functions to this list
        or perhaps delete the ones that are there.
        """
        # global dev
        self.nrows = nrows
        self.ncols = ncols
        self.nbytes = nrows * ncols * 2
        self.prebytes = prepix * 2
        self.postbytes = postpix * 2
        self.bytelist = []
        self.read = dev.read  # This is going to need to change
        self.funcs = RawFrame.funcs[:]  # instance observers
        self.prefuncs = RawFrame.prefuncs[:]  # instance observers

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
        # predata = dev.read(self.prebytes)
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


if __name__ == '__main__':
    import matplotlib.pyplot as plt
    dev = PCIDevice()
    dev.open()
    dev.reset()
    # dev.write(2)

    raw = RawFrame(nrows=2048, ncols=2048, prepix=0, postpix=0)
    raw.grab()

    plt.figure()
    plt.imshow(raw.data)

