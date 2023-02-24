"""
customization file for Raytheon SB-339 ROIC and IR detector array
"""

# should probably just execfile this, instead of import it.

# customization points:
# CLOCK PROGRAM
# How many pixels precede a data frame?
# How many pixels follow a data frame?
# what are the calibration constants for itime?

import run
import det
det.prepix = 4
det.postpix = 8

# what are the offsets and gains of all the clock dacs?
det.dac_mv_per_count = {
    0:1.000 , 1:1.000,
    12:1.000 , 13:1.000,
}

# HARDWARE:
# what are the offsets and gains of all the bias dacs?

class Bunch(object) :
    def __init__(self, **kwds) :
         self.__dict__ = kwds

# make a list of data bunches.
board = [
Bunch(dac=10, gain=2.0, offset=0.0),
]

# what are the offsets and gains of the video channels?

# RUNWORDS are there any special pseudobiases?

# DEWAR
# what are the conversion routines for reading temperature?
# what is the thermal model?

from det import dd

def get_vbias(*args) :
    """
    return the current bias voltage setting
    """
    return dd.dsub - dd.vreset

def set_vbias(value) :
    """
    set the bias voltage by changing dsub
    """
    dd.dsub = value + dd.vreset

def get_vreset(*args) :
    """
    return the current bias voltage setting
    """
    return dd._vreset # redirect to private vreset

def set_vreset(value, **kwds) :
    """
    Change the reset voltage. 
    Maintain current bias by changing dsub too.
    """
    # XXX validate BOTH new voltages first?:
    current_vreset = dd._vreset 
    current_vbias = dd.dsub - current_vreset
    if value > current_vreset :
        step = 2
    else:
        step = -2
    for tvalue in range(current_vreset, value, step):
        dd.dsub = current_vbias + tvalue 
        dd._vreset = tvalue 
    dd.dsub = current_vbias + value # change dsub.. and...
    dd._vreset = value # change private vreset too

from dsp import dspthreadable
from det import writebiasdacfunc

#write_pin14 = writebiasdacfunc(13, offset=0.010, gain=1.95, maxdac=2000, mindac=-1000)

set_vreset = dspthreadable(set_vreset)

dd.additem("vbias", 0, setfunc=set_vbias, getfunc=get_vbias, docstring="change dsub, vbias = dsub-vreset")
dd.additem("vreset", 0, setfunc=set_vreset, getfunc=get_vreset, docstring="change dsub and vreset, vbias = dsub-vreset")
#dd.additem("mybias", 0, setfunc=write_pin14)

dd.additem("voffset", 0, setfunc=writebiasdacfunc(dacs=(10,), gain=1.0))
# a property-based approach:
"""
def myget_vbias(self) :
    return self.dsub - self.vreset

def myset_vbias(self, value) :
    self.dsub = value + self.vreset

from det import DetDict
DetDict.myvbias = property(myget_vbias, myset_vbias)
"""
