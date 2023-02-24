"""
cmc_sb339.py - a script to run in pydsp.
to use, type: execuser cmc_sb339
"""

from autouser import AutoUser
from run import rd
from pydsp import cloop
import time

""" The default Data Word is:
 0x8807e1 = 1000 1000 0000 0111 1110 0001  Most Significant, bits 0-23
 0xa60001 = 1010 0110 0000 0000 0000 0001  Least Significant, bits 24-47

 0xa807e1 = 1010 1000 0000 0111 1110 0001  Most Sig. Run in 8 output mode

"""
def sb339dwLS(val,**kwds):
    """Set value of Least Significant half of the SB339 DataWord. 
    Syntax is:  sb339dwLS(0xa60001)
    The DataWord is 24 bits. 
    Bit # 1  4    8    12   16   20   24   28   32
    Value xxxx xxxx xxxx xxxx 0101 0101 0101 0101"""
    dsp.senddsp(val,31) # hmmm...

def sb339dwMS(val,**kwds):
    """Set value of Most Significant half of the SB339 DataWord. 
    Syntax is:  sb339dwMS(0xa807e1)"""
    dsp.senddsp(val,32) # hmmm...

def cleardet():
    """Take a quick frame to clear out (reset) the detector array """
    rd.nsamp = 1
    rd.itime = 1700
    pedscan()

def darkcurr():
    cleardet()
    rd.itime = 1700
    srun()
    for inttime in range(20000, 50000, 5000):
        rd.itime = inttime
        srun()
        cleardet()

def readnoise():
    cleardet()
    for zz in range(0,64):
        srun()

def CTIAglow():
    cleardet()
    rd.itime = 1700
    for HighGainCurr in range(0,8):
        # change D20, D21, D22 which are bits 21-23 (start is bit 0)
        dataword = 0xa807e0 + HighGainCurr
        sb339dwMS(dataword)
        pedrun()
        srun()
    rd.itime = 10000
    for HighGainCurr in range(0,8):
        dataword = 0xa807e0 + HighGainCurr
        sb339dwMS(dataword)
        pedrun()
        srun()
    sb339dwMS(0xa807e1) # back to default
    

