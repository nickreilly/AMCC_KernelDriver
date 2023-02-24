
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

def move(degrees) :
    print("fake move of wheel to %5.1f degrees."%degrees)
    
class cvFilter :
    """
    Continuously variable filter base class.
    """
    def __init__(self, **kw) :
        self.__dict__.update(kw)
    def set(self, nm) :
        """
        Set the filter to the requested wavelength.
        """
        if nm < 100 : # is request in microns?
            nm *= 1000 # convert to nanometers
        # set cvf to request nanometers.
        if self.range[0] < nm < self.range[1] :
            degrees = self.start + (nm-self.range[0])*self.slope
            self.moveto(degrees)
        else :
            raise ValueError
    def moveto(self,degrees) :
        move(degrees)
        
class fixedFilter :
    """
    Fixed filter base class.

    fixed filters have named filters that know their positions.
    a move method is mixed in, so they can accept move requests.
    """
    def __init__(self,**kw) :
        self.__dict__.update(kw)
    def set(self, name) :
        "set filter to named position by finding the corresponding angle and moving there."
        try :
            angle = self.__dict__[name]["pos"]
        except KeyError :
            raise KeyError("trouble finding filter %s"%name)
        self.moveto(angle) # if this raises an error, pass it on!

    def moveto(self,degrees) :
        ""
        move(degrees)
        
