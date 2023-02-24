from dsp import dspthreadable
import ociw
import det
from det import dd
import fits
import dv
import time
import ociw

def sscan2(**kwds) :
    """
    This acquires a fowler pedestal and signal and writes the normalized difference into a src buffer.

    NOTE: This command does not save the image to disk, a subsequent invocation of this command will 
    overwrite the buffer. Image will be displayed in the next available DV buffer.
    """
    src()
    scan()

sscan2 = dspthreadable(sscan)

def crun2(wfile=sys.stdout) :
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
                bufs.append(ociw.RawFrame(rd.nrow, rd.ncol, prepix=det.prepix, postpix=det.postpix))
            print>>wfile, "CRUN2 mode. type 'burst' to abort"
            while 1 :
                time.sleep(0.05)
                #dsp.dspthread.do(dsp.clockit, bufs=bufs)
                sscan()
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
    thread = threading.Thread(target=crunfunc)
    thread.setDaemon(True) # kills crun if we bail out.
    thread.start()
    time.sleep(0.1) # allow thread to start and print its stuff.

