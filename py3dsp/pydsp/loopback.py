"""
Two loopback tests for testing the PCI card.

loopback() just uses write and read.
irqloopback uses the Half-Full flag interrupt
and the device driver procsssing
bootloop actually boots the dsp.

usage: cd to pydsp source directory, typically 'cd dsp/pydsp'

# python -i loopback.py
>>> bootloop() 

"""

import ociw
import time
import xdir

def loopone(i=0, n=100):
    """simple loopback test. write i to output, 
    read iback from fifo directly.
    do this n times. return nuber of errors."""
    while True:
        try:
            ociw.read(2)
        except:
            break
    print("starting test")
    errs = 0
    while n:
        n -= 1
        ociw.write(i)
        #time.sleep(0.01)
        try:
            iback = ociw.read(2)
            if iback&0x0000ffff != i& 0x0000ffff:
                errs += 1
            try:
                ociw.read(2)
            except:
                pass
        except:
            pass

    return print(f'{errs}')

def loopback():
    "simple loopback test. write to output, read from fifo directly."
    while True:
        try:
            ociw.read()
        except:
            break
    print("starting test")
    while True:
        print("writes")
        for i in range(-32768, 32768):
            ociw.write(i)
        print("reads")
        for i in range(-32768, 32768):
            assert i == ociw.read()
        print("end one pass")

import array
k = 0

def irqloopback(nrow=256, ncol=256, nsamp=2):
    """
    irqloopback(nrow, ncol, nsamp)

    set image parameters. Tell driver to expect video.
    then write the video directly out the HSS.
    loopback plug sends it back into fifo, and the driver reads it.
    go to the raw image buffers to test that video is actually coming back.
    A "loopbackdsp" object could be of use here, and simulate the imager.
    """
    global k
    k = (k + 1)&0xffff
    while True:
        try:
            ociw.read()
        except:
            break
    #ociw.set_rcs(nrow, ncol, nsamp)
    ociw.clear_fifo()
    time.sleep(0.1)
    # make sure no extra pixels.
    while True:
        try:
            ociw.read()
        except:
            break
    print("---")
    for j in range(nsamp*2):
        ociw.write(-1) # extra pixel
        ociw.write(-2) # extra pixel
        for i in range(nrow*ncol): # write out a frame
            ociw.write(((i+j+k)&0x00FFFF)-32768)
        print("---")
        time.sleep(0.1) # give device driver a chance to clean up.
    
    for j in range(nsamp*2):
        # need to actually check that enough pixels were received to
        # actually be a frame.
        # s = ociw.get_raw_buf(j, 256*256)[0:nrow*ncol*2]
        s = ociw.peek_fifo(j, 256*256)[0:nrow*ncol*2]
        # hey, slicing a buffer object returns a string object!
        # unless the slicing is perfect, then it returns a buffer object!
        # the array object cannot take a buffer object for an initializer!
        # why not?
        a = array.array("h", str(s))
        for i in range(nrow*ncol):
            if ((i+j+k)-32768)&0x00ffff != a[i]&0x00FFFF:
                print(i, j, a[i])
                raise RuntimeError

def progloopback():
    """
    Test the loopback mechanism by communicating with a running dsp program.

    write down all values to a DSP memory location, then read them back.
    runs the loop approximately every 4 -10 seconds.
    """
    runs = 0
    errs = 0
    xfers = 0
    import dsp
    try:
        while True:
            ociw.read()
    except SystemError:
        pass
    while True:
        for i in range(65536):
            """
            try:
                while 1:
                    ociw.read()
                    errs += 1 # should never get here!
            except SystemError: # expected.
            """
            xfers += 1
            ociw.data24(i) # send i down to the data location (3 out)
            ociw.command(7, 10) # write to X data location 10 (3 out)
            ociw.command(10, 10) # read X data, location 10 (3 out)
            iback = ociw.read()&0xffff # read back result (0 out)
            if iback != i:
                print(i, iback)
                errs += 1
        time.sleep(0.1)
        runs += 1
        print("%d runs %d errors %d xfers" % (runs, errs, xfers))
        if dsp.dspthread.interrupted:
            break

def bootloop():
    """
    Continually reboot the dsp and run an echo test.

    Alternates between echo and necho. necho echoes negated values.
    each echo test writes all values from 0 to 65535 out to the echoing 
    program. The values are then read back one at a time
    and compared to expected. If they dont match, call it a data error
    and move to the next reboot.
    secs per transfer is per roundtrip, and includes the writes as well
    as the reads.
    """
    import sload
    L = sload.sloader()
    boots = 0
    errs = 0
    dataerrs = 0 
    while True:
        import dsp
        import time
        try: 
            boots += 1
            if boots % 2:
                L.load_srec(xdir.clockpath+"/echo.s", dev)
            else:
                L.load_srec(xdir.clockpath+"/necho.s", dev)
            start = time.time()
            print("writes")
            for i in range(65536):
                ociw.write(i)
            print("reads")
            for i in range(65536):
                try: iback = ociw.read()
                except: iback = ociw.read()
                if not boots % 2:
                    iback = ~iback
                iback = iback & 0x0ffff
                if iback != i:
                    print("error on", hex(i), "got", hex(iback)) 
                    dataerrs += 1
                    break
            print((time.time() - start)/65536.0 , "seconds per transfer")
        except SystemError:
            import traceback
            traceback.print_exc()
            errs += 1
        print(boots, "boots", errs, "errs", dataerrs, "dataerrs")
        
if __name__ == '__main__':
    ociw.open_ociw()
