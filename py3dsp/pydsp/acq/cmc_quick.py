"""
cmc_quick.py - a script to run in pydsp.
    
    Do something short and quick.  The function of this script
    changes all the time. This is a testing script.

to use, type: execuser cmc_quick
"""


from run import rd
from numpy import *
import xdir
import pyfits
import time
import filterBase
import ls332

#rd.nsamp = 1
#rd.nrow = 1024
#rd.ncol = 2048
#rd.ncol = 1024



print 'Which filter do you want to use for Test 1?'
FilterTest1 = str(raw_input())

#filterBase.set(FilterTest1)
filterBase.set(('cvfIII',int(FilterTest1)))
