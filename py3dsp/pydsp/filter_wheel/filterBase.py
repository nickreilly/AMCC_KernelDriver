
"""
filterBase.py    Base Filter Classes. (Replaces FILTERWH.FTH)

contains both fixed and variable filters.
they *could* both inherit from a common "filter" base class..
but at the current complexity level, it doesn't
really seem helpful to do so.

Filters support a simple interface...
You basically set the filter to some value.

CV filters are set by wavelength.
fixed filters are set by filter name.

The lowest level of this ultimately moves the wheel to some angle..
This module has a dummy move method that gets overridded with the 
real move method. A single wheel is assumed in the current implementation.
"""

__version__ = """$Id: filterBase.py 399 2006-06-04 20:02:17Z drew $"""
__URL__ = """$URL: http://astro.pas.rochester.edu/svn/pydsp/trunk/pydsp/filterBase.py $"""
__author__ = "$Author: drew $"

# This should be a symbolic link to one of many filter wheel files
try:
   import filters 
except ImportError:
   print("must add symbolic link 'filters.py' to real filter wheel file.")

# It appears that we need a fake move to keep PYDSP from moving the filter
# wheel at startup (usually FW is left in the correct spot when pydsp exits,
# so moving wheel puts it in the wrong spot).  This is caused by the line 
# "self.setfunc..." in DataDict.py -- see that for more info.  
# Move is later redefined (see fwpUpdate and rd.fwp in run.py and pydsp.py).
def move(position) :
    print("fake move of wheel to %5.1f position."%position)
    
class GotoFWP :
    """
    filter base class.
    """
    def __init__(self, **kw) :
        self.__dict__.update(kw)
    def set(self, position) :
        """
        Set the filter to the requested wavelength.
        """
        self.moveto(position)
    def moveto(self,position) :
        move(position)
        
filtername = 'blind'
bandwidth = 0.0
wavelength = 0.0
transmission = -2.0
filtercomment = 'not set yet'

def get_wavelength() :
    return wavelength

def get_bandwidth() :
    return bandwidth

def get_transmission() :
    return transmission

def get_filtername() :
    return filtername

def get_filtercomment() :
    return filtercomment

def set(value,nm=None) :
    global filtername, bandwidth, wavelength, transmission, filtercomment
    """Position filter wheel.

    Will take a string or numeric argument. It first looks for a match in
    the fixed filter. Failing that, it looks for a tuple and sets the 
    filtername to the first argument and the wavelength to the second arg. and
    then tries to match the filtername to a dictionary of CVFs in filters.py
    """
    if value in filters._fix :

        # this is hacky. should have get property of a filterwheel instance.
        filtername = value
        bandwidth = filters._fix[value]['bnd']
        wavelength = filters._fix[value]['wvl']
        transmission = filters._fix[value]['trans']
        filtercomment = filters._fix[value]['comment']
        position = filters._fix[value]['pos']
        fixedFilter = GotoFWP()
        fixedFilter.set(position)
        return
    else :
        if isinstance(value, tuple):
            nm = float(value[1])
            filtername = value[0]
            if filtername not in list(filters.cvfparam.keys()):
                raise ValueError("can't find that CVF on this filter wheel!")
        elif value not in list(filters.cvfparam.keys()):
            raise ValueError("can't find that on this filter wheel!")
        if nm == None:
            """ When the pydsp starts up, the DataDict.py sets ALL things in 
            rd & dd, which includes the filter.  On startup, pydsp does not 
            send the wavelength to the filter command for CVFs.  So, we need
            a way to check and then force the wavelength to be set for CVFs. """
            import sys
            blah = sys.modules["__main__"]
            if not blah.filterinitialize :
                filtername = value
                # We can't get the wavelength from rd (why???) but we can get
                # the FWP.  Calculate wavelength from fwp.
                nm = round(((blah.rd.fwp - filters.cvfparam[filtername]['start'])/filters.cvfparam[filtername]['slope'] + filters.cvfparam[filtername]['range'][0]), 1)
            else:
                raise ValueError("cvfs require a wavelength parameter in nm!")
        if nm < 100 : # is request in microns?
            nm *= 1000 # convert to nanometers
        # Find closest FWP (increments of 0.25 step) to requested wavelength.
        initposition = filters.cvfparam[filtername]['start'] + (nm-filters.cvfparam[filtername]['range'][0])*filters.cvfparam[filtername]['slope']
        nearfwp = float(round(initposition*4)/4.0)
        # Find the wavelength at the nearest FWP and round to 1 decimal.
        nearwavelength = round(((nearfwp - filters.cvfparam[filtername]['start'])/filters.cvfparam[filtername]['slope'] + filters.cvfparam[filtername]['range'][0]), 1)
        # Check if the wavelength is on the given CVF
        if (nearwavelength > filters.cvfparam[filtername]['range'][0] and nearwavelength < filters.cvfparam[filtername]['range'][1]) :
            print('Closest FWP to request is ' + str(nearfwp) + ' which corresponds to center wavelength=' + str(nearwavelength))
            wavelength = nearwavelength
            # Calculate the bandwidth using the spectral resolution, and round
            # it to 2 decimal places.  
            # WARNING: CVF BANDWIDTH IS APPROXIMATE - user should do full calc.
            bandwidth = round((wavelength / filters.cvfparam[filtername]['res']), 2)
            # Calculate the transmission (OCLI CFVs only have 3 trans points).
            # Do a linear interpolation between those three trans points.
            cvfshort = filters.cvfparam[filtername]['range'][0]
            cvfmidpoint = (filters.cvfparam[filtername]['range'][1] + filters.cvfparam[filtername]['range'][0])/2
            cvflong = filters.cvfparam[filtername]['range'][1]
            translo = filters.cvfparam[filtername]['trans'][0]
            transmid = filters.cvfparam[filtername]['trans'][1]
            transhi = filters.cvfparam[filtername]['trans'][2] 
            if wavelength < cvfmidpoint :
                transmission = transmid + ((cvfmidpoint - wavelength)/(cvfmidpoint - cvfshort))*(translo - transmid)
            else: 
                transmission = transmid - ((wavelength - cvfmidpoint)/(cvflong - cvfmidpoint))*(transmid - transhi)
            # Round off the transmission to two decimal places.
            transmission = round(transmission,2)
            # Set the text string for the filter comment in FITS header
            filtercomment = filters.cvfparam[filtername]['comment']
            # Finally, move to the requested filter.
            cvFilter = GotoFWP()
            cvFilter.set(nearfwp)
        else :
            raise ValueError("That wavelength is not on the given CVF.")
    
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
"""
def _test() :
    def move(degrees) :
        print "moving wheel to %5.1f degrees"%degrees
    
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
"""
