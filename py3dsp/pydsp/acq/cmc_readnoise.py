"""
cmc_readnoise.py - a script to run in pydsp.
    
    script to acquire a sequence of images at short integration times
    while changing the number of samples, i.e. noise versus fowler-N. 

to use, type: execuser cmc_readnoise
"""

from autouser import AutoUser
from run import rd
from pydsp import cloop
import xdir
import pyfits
import time


print 'taking read noise data.'
#rd.object = 'readnoise77K-1'
#rd.itime = 100000
#rd.lc = ''
#rd.nsamp = 1
#rd.nrow = 2048
#rd.ncol = 2048
#rd.nrowskip = 0
#rd.ncolskip = 0

burst()
time.sleep(5)
burst()
time.sleep(5)
burst()

# do a couple images and throw away -- stability.
sscan()
sscan()
for numsamp in range( 1, 2, 1) :
    rd.nsamp = numsamp
    for kimages in range(0, 2000, 1):
        srun()

print 'Finished taking data'
crun()

