"""
Some testing that was written while migrating to the Linux 2.6 kernel.

The device driver was greatly simplified during this migration.
"""

# import ociw_cpy as ociw
# import ociw
import amcc
import dsp

import sload
import time
import numpy as np # numarray



if __name__ == '__main__':
    try :
        amcc.close()
    except :
        pass # probably wasn't open anyway.
    try :
        # ociw.open("/etc/udev/devices/ociw0")
        dev = dsp.initialize('pci')
        # dev.open()
        L = sload.sloader(dev)
        L.load_srec("/usr/dsp/runtime/56300/mkimage.s")
    except MemoryError :
        pass #  mmap does not work yet

# mkimage is special program that sends back n..0 if you write out N

# Tuple defining array dimensions we want.
shape = (2048,2048) # FIFO is 8K. 128 by 128 image is 2x bigger than that.

# Create a new image array
def testacq() :
    global image
    toofasts = 0
    #image = numarray.array(shape=shape,type=numarray.Int16)
    time.sleep(0.01) # let the other processes get a slice.
    start = time.time()
    npix = shape[1]*shape[0]
    expected = npix - 1
    dev.write((npix/256)-1) # if you make it -2, last pix fails.
    s = dev.read(npix*2)
    while len(s) != npix*2 :
        try :
            s += dev.read(npix*2 - len(s))
            toofasts += 1
        except KeyboardInterrupt :
            print(len(s), "expecting", npix*2)
            raise
    end = time.time()
    print("Total time: ", (end - start), "seconds") 
    print((end - start) / npix , "sec per pixel", end=' ') 
    print(npix / (end - start), "pixel/sec")
    start = time.time()
    image=np.fromstring(s,dtype=np.int16,shape=shape) 
    end = time.time()
    print("numpy took", end - start)
    print("too fasts:", toofasts)
    expected = ((shape[1]*shape[0])/256)-1
    assert image[0,0]&0xffff == expected, ("%s,%s"%(image[0,0]&0xffff,expected))
    assert image[-1,-1]&0xffff == 0, ("%s,%s"%(image[-1,-1]&0xffff,0))
            #assert image[i,j]&0xffff == expected, ("%s,%s"%(image[i,j]&0xffff,expected))
            #assert image[i,j]&0xffff == expected, ("%s,%s"%(image[i,j]&0xffff,expected))
            #expected -= 1
    try :
        time.sleep(0.01)
        dev.read()
        print("unexpected data")
    except SystemError :
        print("success! got exactly the right amount of data")
        pass

if __name__ == '__main__':
    testacq()
