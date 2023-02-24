"""
cmc_sb369.py - a script to run in pydsp.
to use, type: execuser cmc_sb369
"""

from autouser import AutoUser
from run import rd
from pydsp import cloop
import time


def sb369off(**kwds):
    # Set all voltages to zero 
    dd.pDetA1lo = 0
    dd.pDetA1hi = 0
    dd.pDetA2lo = 0
    dd.pDetA2hi = 0
    dd.pDetA3lo = 0
    dd.pDetA3hi = 0
    dd.pDetA4lo = 0
    dd.pDetA4hi = 0
    dd.pData_lo = 0
    dd.pData_hi = 0
    dd.pMC_lo = 0
    dd.pMC_hi = 0
    dd.sArOn1hi = 0
    dd.sArOn1lo = 0
    dd.sArOn2hi = 0
    dd.sArOn2lo = 0
    dd.vpa = 0
    dd.vna = 0
    dd.vpd = 0
    dd.vnd = 0
    dd.vpout = 0
    dd.vnout = 0
    dd.vclamp = 0
    dd.vreset = 0
    dd.vpr1 = 0
    dd.vnr1 = 0
    dd.vpr2 = 0
    dd.vnr2 = 0

"""
Start up sequence as recommended in Raytheon User's Manual, page 51.
"""
def sb369s1on(**kwds):
    # Turn of power to side 1 of array.  But first, make sure all were off,
    # especially vdetcom (aka vreset), since lastrun.run may have restored.
    sb369off()
    time.sleep(2)
    dd.vpr1 = +5500
    seebiaspin(5)
    time.sleep(1)
    dd.vpr2 = +5500
    seebiaspin(7)
    time.sleep(1)
    dd.vreset = +3700
    seebiaspin(12)
    time.sleep(1)
    dd.vclamp = +4700
    seebiaspin(9)
    time.sleep(1)
    dd.vpout = +5500
    seebiaspin(14)
    time.sleep(1)
    dd.vpd = +5500
    seebiaspin(3)
    time.sleep(1)
    dd.vpa = +5500
    seebiaspin(2)
    dd.sArOn1lo = +5500
    dd.sArOn1hi = +5500
    dd.pMC_hi = +5500
    dd.pData_hi = +5500
    dd.pDetA1hi = +5500
    dd.pDetA2hi = +5500
    dd.pDetA3hi = +5500
    dd.pDetA4hi = +5500


def sb369s2on(**kwds):
    # Turn of power to side 2 of array.  But first, make sure all were off,
    # especially vdetcom (aka vreset), since lastrun.run may have restored.
    sb369off()
    time.sleep(2)
    dd.vpr1 = +5500
    seebiaspin(5)
    time.sleep(1)
    dd.vpr2 = +5500
    seebiaspin(7)
    time.sleep(1)
    dd.vreset = +3700
    seebiaspin(12)
    time.sleep(1)
    dd.vclamp = +4700
    seebiaspin(9)
    time.sleep(1)
    dd.vpout = +5500
    seebiaspin(14)
    time.sleep(1)
    dd.vpd = +5500
    seebiaspin(3)
    time.sleep(1)
    dd.vpa = +5500
    seebiaspin(2)
    dd.sArOn2lo = +5500
    dd.sArOn2hi = +5500
    dd.pMC_hi = +5500
    dd.pData_hi = +5500
    dd.pDetA1hi = +5500
    dd.pDetA2hi = +5500
    dd.pDetA3hi = +5500
    dd.pDetA4hi = +5500


def sb369biason(**kwds):
    dd.vpr1 = +5500
    dd.vpr2 = +5500
    time.sleep(1)
    dd.vreset = +3700
    time.sleep(1)
    dd.vclamp = +4700
    time.sleep(1)
    dd.vpout = +5500
    time.sleep(1)
    dd.vpd = +5500
    time.sleep(1)
    dd.vpa = +5500
    dd.sArOn1lo = +5500
    dd.sArOn1hi = +5500

def sb369clockon(**kwds):
    dd.pMC_hi = +5500
    dd.pData_hi = +5500
    dd.pDetA1hi = +5500
    dd.pDetA2hi = +5500
    dd.pDetA3hi = +5500
    dd.pDetA4hi = +5500

""" The default Data Word is:
a9dd4ac3 = 1010 1001 1101 1101 0100 1010 1100 0011 """

def sb369dwLS(val,**kwds):
    """Set value of Least Significant half of the SB369 DataWord. 
    Syntax is:  sb369dwLS(0x4ac3)
    The DataWord is 32 bits.  This sets 16 bits on the right half (bits 17-32):
    Bit # 1  4    8    12   16   20   24   28   32
    Value xxxx xxxx xxxx xxxx 0101 0101 0101 0101"""
    dsp.senddsp(val,31) # hmmm...

def sb369dwMS(val,**kwds):
    """Set value of Most Significant half of the SB369 DataWord. 
    Syntax is:  sb369dwMS(0xa9dd)"""
    dsp.senddsp(val,32) # hmmm...

