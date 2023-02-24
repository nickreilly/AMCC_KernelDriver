import det
det.prepix = 2
det.postpix = 8

# what are the offsets and gains of all the clock dacs?
# This is set using det.writeclockdac 
det.dac_mv_per_count = {
    0:1.000 , 1:1.000,
    12:1.000 , 13:1.000,
}

from det import dd

def get_vbias(*args) :
    """
    Return the current bias voltage setting.
    """
    return dd.dsub - dd.vreset

def set_vbias(value) :
    """
    Set the bias voltage by changing dsub.
    """
    dd.dsub = value + dd.vreset

def get_vreset(*args) :
    """
    Return the current bias voltage setting.
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

set_vreset = dspthreadable(set_vreset)

dd.additem("vbias", 0, setfunc=set_vbias, getfunc=get_vbias, docstring="change dsub, vbias = dsub-vreset")
dd.additem("vreset", 0, setfunc=set_vreset, getfunc=get_vreset, docstring="change dsub and vreset, vbias = dsub-vreset")
