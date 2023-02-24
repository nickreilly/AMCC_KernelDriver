"""
grz_pednoise_newclock.py - a script to run in pydsp.
    
    script to acquire a sequence of pedestal images
    with short integration times to measure noise.

to use, type: execuser grz_pednoise_newclock
"""


from autouser import AutoUser
from run import rd
from pydsp import cloop
import xdir
import pyfits
import time


def WaitAndBurst():
    time.sleep(waittime) # wait for temperature to stabilize
    burst()  # stop CRUN mode
    time.sleep(5)
    burst()  # sometimes, once is not enough
    time.sleep(5)



waittime=10


WaitAndBurst()
rd.object= 'pednoise_newclocksutr'
rd.itime = 11000
rd.nrow= 2048
rd.nrowskip=0
rd.nsamp=2
for i in range(0,100):
    sutr()

crun()
