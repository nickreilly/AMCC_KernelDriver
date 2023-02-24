"""
The main unit test suite for pydsp.

Somewhat of a summary of the different modules as well.
run at the Pydsp command, probably WITHOUT a device connected.
"""

__version__ = """$Id: tests.py 399 2006-06-04 20:02:17Z drew $ """

__author__ = "$Author: drew $"

__URL__ = "$URL: http://astro.pas.rochester.edu/svn/pydsp/trunk/pydsp/tests.py $"

from run import rd
from det import dd
from autouser import AutoUser
from pydsp import cloop
import dsp

def test_DataDict() :
    import DataDict
    print("Testing DataDict..")
    DataDict._test()
    print("DataDict test done")

def test_det() :
    print("need to write code to test the det module.")
    # core det code: the det dictionary.
    # setclockpgm, clockpath. clockprogname
    # init. adds stuff to detdict.
    # dacgains. dac clipping
    # writeclockdac; needs dsp, 
    # setbiases.. uses biasrunfile.
    # showbiases. bias map file.
    # loaddet and savedet.
    pass

def test_run() :
    print("need to write code to test the run module")
    pass

def test_xdir() :
  import xdir
  import os
  # remember the old xdir path variables
  print("testing the xdir module.")
  olddatapath = xdir.datapath
  olddetpath = xdir.detpath
  assert olddatapath
  assert olddetpath
  try :
    # make a new test data directory in dsphome.
    newdatapath = xdir.dsphome+"/testdata"
    assert newdatapath != olddatapath # make sure it is new
    assert not os.access(newdatapath,4)
    # make a new test det directory in dsphome.
    newdetpath = xdir.dsphome+"/testdet" # make sure it is new
    assert newdetpath != olddetpath
    assert not os.access(newdetpath,4)
    # test that setdatapath fails if bad directory
    xdir.setdatapath(newdatapath) # should fail
    assert xdir.datapath == olddatapath # make sure it does
    # test that setdetpath fails if bad directory
    xdir.setdetpath(newdetpath) # should fail
    assert xdir.detpath == olddetpath # make sure it does
    # finally, create the new directories
    os.mkdir(newdatapath)
    os.mkdir(newdetpath)
    # set datapath to the test data direcory
    # test that setdatapath works if good directory.
    xdir.setdatapath(newdatapath) # should succeed 
    assert xdir.datapath == newdatapath # make sure it does
    # set detpath to the test det directory
    # test that setdetpath works if good directory.
    xdir.setdetpath(newdetpath) # should succeed 
    assert xdir.detpath == newdetpath # make sure it does
    # set the clockpath to the test clocking code directory
    # the det directory and the clocking code directory are similar!
    # they both know a bit about the current system setup.
    # test that setdatapath clears the night
    # test that setdatapath clears the object
    # test that setdetpath is silent on first set.
    # test that setdetpath is silent if no change in directory.
    # test that setdetpath notifies user if detpath changes.
    # test that night fails if datapath is bad
    # test that night fails if nightname argument is bad
    # test that night creates a new night path if all else good.
    # test that night reuses an old night path
    # test that night makes a default object (or clears the object.)
    # test that setting an object fails if no datapath or night
    # test that setting an object fails if the object name is bad.
    # test that setting an object makes a new directory if need be.
    # test that setting an object reuses an old directory if 
    # test that nextobject advances properly 
  finally :
    xdir.setdetpath(olddetpath)
    xdir.setdatapath(olddatapath)
    os.rmdir(newdatapath)
    os.rmdir(newdetpath)
    print("done testing xdir.")

def test_dsp() :
    print("need to write code to test the dsp module.")
    # This is a little tricky. dsp is semantically 'clock program'
    # srecord load.
    # command primitives.
    # higher prims (setrcs)
    # write dacs
    # _clockit
    # hmm. some thread sharing issues.
    # other clock program specific stuff.
    # getFrameTime.
    # getPedTime
    # timer
    # 7888 ADC
    # bias and clock currents and voltages. (header board)
    # stepper motor stuff.
    # dspthread.
    pass

def test_runrun() :
    "test the functionality of the runrun module"
    import runrun
    from run import rd
    print("testing the runrun module")
    # save current state, and
    nstuff =  rd.nrow, rd.ncol, rd.nsamp, rd.nrowskip, rd.ncolskip 
    # change the calconstants, 
    # see if minItime returns the right number of millisecs.
    calconst, runrun.calconst = runrun.calconst, [0.001,0.002,0.003,0.004,0.005]
    rd.nrow, rd.ncol, rd.nsamp, rd.nrowskip, rd.ncolskip = (200,200,2,0,0)
    assert runrun.minItime() == 80
    rd.nrow, rd.ncol, rd.nsamp, rd.nrowskip, rd.ncolskip = (200,200,2,200,200)
    assert runrun.minItime() == 402
    # restore modified values.
    rd.nrow, rd.ncol, rd.nsamp, rd.nrowskip, rd.ncolskip = nstuff
    runrun.calconst = calconst

    # now check that these functions change the buffer flags.
    runrun.src()
    assert rd.bufferflag == 'src'
    runrun.bkg()
    assert rd.bufferflag == 'bkg'
    runrun.ped()
    assert rd.bufferflag == 'ped'
    runrun.sig()
    assert rd.bufferflag == 'sig'
    # test preCheckItime?
    minItime, runrun.minItime = runrun.minItime, lambda : 1234
    itime, rd.itime = rd.itime, 2000
    # interactive, needs test user. Check normal pass.
    runrun.preCheckItime(AutoUser())
    assert rd.itime == 2000
    rd.itime = 1000
    # check blank response.
    runrun.preCheckItime(AutoUser(""))
    assert rd.itime == 1000
    # check number confirm too low.
    runrun.preCheckItime(AutoUser("1100",""))
    assert rd.itime == 1000
    # check Y response.
    runrun.preCheckItime(AutoUser("Y"))
    assert rd.itime == 1234
    rd.itime = 1000
    # check Number confirm OK
    runrun.preCheckItime(AutoUser("3000"))
    assert rd.itime == 3000
    # restore stuff.
    rd.itime = itime
    runrun.minItime = minItime
    print("done testing runrun")
    
def test_dv() :
    print("need to write code to test the dv module")

def test_filter () :
    import filterwh
    print("Testing the filterwheel.")
    filterwh._test()
    print("done testing the filter wheel")

def test_scans() :
    """Automatically test the system. The actual system.

    Puts in known commands, and checks to see that proper responses
    are obtained. This suite can be run after code is changed to
    make sure that things were not broken."""

    print("testing scanning")
    # ok, first set itime, nrow, ncol, etc....
    user = AutoUser("itime 0", "nrow 512", "ncol 512", "nsamp 1")
    cloop(input=user)

    # test that they actually got set.
    assert rd.itime == 0
    assert rd.nrow == 512
    assert rd.ncol == 512
    assert rd.nsamp == 1

    # now run a real scan
    # since itime is too short, it will ask for a longer value.
    import time
    start = time.time()
    user = AutoUser("sscan","5000")
    cloop(input=user)
    
    # itime should have changed to the new value.
    assert rd.itime == 5000
    # scan should have actually taken longer than the itime
    assert time.time() - start > 5.0 # should take longer than itime.
    user = AutoUser("itime 6000","sscan")
    cloop(input=user)
    assert rd.itime == 6000
    def interrupter(sig,frame) : # something to interrupt a scan
        print("bang!")
        dsp.dspthread.interrupt()
    import signal
    old = signal.signal(signal.SIGALRM, interrupter)
    try :
        signal.alarm(2)
        start = time.time()
        user = AutoUser("sscan")
        try :
            cloop(input=user)
        except KeyboardInterrupt :
            pass
        import runrun
        assert time.time() - start < runrun.scantime() # scan should have been cut short.
        assert dsp.dspthread.interrupted # interrupted flag should be set.
    finally :
        signal.signal(signal.SIGALRM,old) # clean up when we are done
        
def test_autouser() :
    print("testing the test user")
    import autouser
    autouser._test()
    print("done testing the test user")

def test_pydsp() :
    print("need to write code to test the pydsp module.")

def test_start_pydsp() :
    print("possible to write code to test start_pydsp? (it is a script).")

def testall() :
    test_DataDict()
    test_det()
    test_run()
    test_xdir()
    test_dsp()
    test_runrun()
    test_dv()
    test_filter()
    test_scans()
    test_autouser()
    test_pydsp()

if __name__ == "__main__" :
    testall()
