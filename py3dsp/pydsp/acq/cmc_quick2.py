"""
cmc_quick2.py - a script to run in pydsp.
    
    Do something short and quick.  The function of this script
    changes all the time. This is a testing script.

to use, type: execuser cmc_quick
"""

from autouser import AutoUser
from run import rd
from pydsp import cloop
from numarray import *
import xdir
import pyfits
import time
import filterBase

#rd.nsamp = 1
#rd.nrow = 1024
#rd.ncol = 2048
#rd.ncol = 1024

burst()
rd.itime=3000
for inttime in range(1, 3, 1):
    #rd.itime=inttime
    #time.sleep(3)
    sscan()
rd.itime=3000
rd.nsamp = 400
sutr()


