#!/usr/bin/python

"""
The top level file of the pydsp system.
all execution starts here.
A shell-bang could have been used, but we
used an alias instead.

Basically, start the DSP thread,
initialize the runtime hooks,
start the gui thread
Start some extra stuff like runline completion,
then bring the control system to life.
finally, dump into the command loop.
"""
__version__ = """$Id: start_pydsp.py 398 2006-06-03 22:21:52Z drew $ """

__author__ = "$Author: drew $"

__URL__ = "$URL: http://astro.pas.rochester.edu/svn/pydsp/trunk/pydsp/start_pydsp.py $"



def loadDetAndRun() :
    """
    Load the det (static) and run (volatile) dictionaries.

    This effectively fires up the system, for instance:
    setting the clock program attribute is a "smart" operation,
    and loads the clock program into the DSP as well.
    """
    import det
    det.init()

    import run
    run.init()

if __name__ == '__main__' :
    import dsp 
    dsp.dspthread = dsp.DspThread()
    dsp.dummymode()

    import det
    import run
    loadDetAndRun()
    import pydsp
    from pydsp import *
    from runrun import *

    pydsp.startpydsp()
    wfile = open("/tmp/words_dump.txt", "w")
    sys.stdout = wfile
    help("all", "input", wfile)
