#
# biassetup.py
#

"""
"""

BIASDEFAULT = 0
import time

def vbias(bias) :
    """
    vbias has been moved to det dictionary.
    """
    dd.vbias = bias

def biassetup(bias) :
    if dd['dsub'] - dd['vreset'] == bias : return
    vbias(bias)
    rd['nsamp']=1
    rd['itime']=5000
    start = time.time()
    while time.time() - start < 120 :
        sscan()
