
"""
   filterII.py        Filter Routines. (Replaces FILTERWH.FTH)
   concrete implementation of abstract filter.
   This is style II.
"""

__version__ = """$Id: filterII.py 247 2004-10-23 18:52:25Z dsp $ """

__author__ = "$Author: dsp $"

__URL__ = "$URL: http://astro.pas.rochester.edu/svn/pydsp/trunk/pydsp/filterII.py $"

import filterBase # abstract filter

dialzero = 27.75  # in dial counts, not degrees.
cdszero = 960 # also in dial counts.

_cvfII = {
    "slope": -0.022877, # degree/nm, from ocli. corrected 2% to 5k 
    "start": 195.0,  # start position in degrees
    "range" : (4126.0,8060.0), # starting and ending wavelength, nanometers
    "res" : 0.116 # resolution in nanometers
}

_cvfIII = {
    "slope": -0.0130643, # degree/nm, from ocli. corrected 2% to 5k 
    "start": 90.0,  # start position in degrees
    "range" : (7703.0,14592.0), # starting and ending wavelength, nanometers
    "res" : 0.065 # resolution in nanometers
}

_fix = {
        "cds" : {"pos":335.0, "wvl":0.0,   "bnd":0.0}, # Cold Dark Slide. Black paper
        "vis" : {"pos":320.0, "wvl":500.0, "bnd":300.0}, # KG-5 colored glass Oriel 
        "j"   : {"pos":305.0, "wvl":1268.0,"bnd":300.0}, # Barr Assoc
        "h"   : {"pos":290.0, "wvl":1645.0,"bnd":350.0}, # Barr Assoc
        "k"   : {"pos":275.0, "wvl":2190.0,"bnd":409.0}, # Barr Assoc
        "3.3" : {"pos":260.0, "wvl":3269.0,"bnd":200.0}, # OFC 3.3+ND (OFC Ge 2.95)
        "l'"  : {"pos":245.0, "wvl":3745.0,"bnd":807.0}, # Dick Joyce l'+2xND (OD 2.2)
        "m'"  : {"pos":230.0, "wvl":4667.0,"bnd":352.0}, # OCLI M'
        "al1" : {"pos":215.0, "wvl":0.0,   "bnd":0.0}, # Al tape and black paper
}

# a filter wheel may have several cvfs.
cvfII = filterBase.cvFilter(**_cvfII)
cvfIII = filterBase.cvFilter(**_cvfIII)

cvfs = [cvfII, cvfIII ]

# it also has a palette of fixed filters.
fix = filterBase.fixedFilter(**_fix)

filtername = None
bandwidth = None
wavelength = None

def get_wavelength() :
    return wavelength

def get_bandwidth() :
    return bandwidth

def get_filtername() :
    return filtername

def set(value) :
    global filtername, bandwidth, wavelength
    """Position filterII.

    Will take a string or numeric argument. It first looks for a match in
    the fixed filter. Failing that, it tries to convert the argument
    to a float, and if successful, searchs for a match on cvfII, then cvfIII.
    XXX seems to be almost an abstract method.
    Iterate over cvfs, and put in filterBase???
    """
    if value in fix.__dict__ :
        print(value)
        fix.set(value)
        # this is hacky. should have get property of a filterwheel instance.
        filtername = value
        bandwidth = _fix[value]['bnd']
        wavelength = _fix[value]['wvl']
        return
    else :
        try : 
            fvalue = float(value)
        except:
            raise ValueError("can't find that on this filter wheel!")
    if cvfII.range[0] < fvalue < cvfII.range[1] :
        cvfII.set(fvalue)
        bandwidth = "cvfII"
    elif cvfIII.range[0] < fvalue < cvfIII.range[1] :
        cvfIII.set(fvalue)
        bandwidth = "cvfIII"
    else:
        raise ValueError("can't find that on this filter wheel!")
    filtername = value
    wavelength = value
    

__test__ = {
"usage" : """
>>> fix.set("cds")
moving wheel to 335.0 degrees
>>> fix.set("j")
moving wheel to 305.0 degrees
>>> fix.set("h")
moving wheel to 290.0 degrees
>>> fix.set("k")
moving wheel to 275.0 degrees
>>> fix.set("3.3")
moving wheel to 260.0 degrees
>>> fix.set("l'")
moving wheel to 245.0 degrees
>>> fix.set("m'")
moving wheel to 230.0 degrees
>>> fix.set("al1")
moving wheel to 215.0 degrees
>>> fix.set("foo")
Traceback (most recent call last):
  File "<stdin>", line 1, in ?
  File "filterBase.py", line 35, in set
    raise KeyError, "trouble finding filter %s"%name
KeyError: 'trouble finding filter foo'
>>> cvfII.set(3300)
Traceback (most recent call last):
  File "<stdin>", line 1, in ?
  File "filterBase.py", line 23, in set
    raise ValueError
ValueError
>>> cvfII.set(4400)
moving wheel to 188.7 degrees
>>> cvfII.set(7000)
moving wheel to 129.3 degrees
>>> cvfII.set(9000)
Traceback (most recent call last):
  File "<stdin>", line 1, in ?
  File "filterBase.py", line 23, in set
    raise ValueError
ValueError
>>> cvfIII.set(9000)
moving wheel to  73.1 degrees
>>> cvfIII.set(9.0)
moving wheel to  73.1 degrees
>>>
"""
}

def _test() :
    def move(degrees) :
        print("moving wheel to %5.1f degrees"%degrees)
    
    oldmove, filterBase.move = filterBase.move, move 
    try :
        import doctest, filterII
        return doctest.testmod(filterII)
    finally:
        # this WILL be executed before the return statement above.
        filterBase.move = oldmove # may need some other state restored?
        # (another problem with not having a wheel class. testability!)

if __name__ == "__main__" :
    _test()
