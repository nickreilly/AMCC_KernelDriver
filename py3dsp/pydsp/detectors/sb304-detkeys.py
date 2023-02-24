"""
# detkeys is a list of det dict things that should be saved with savesetup    
# a better place for this file?
# runtime/pydet directory? it is detector specific.

"""

__version__ = """$Id: hawaii.py 165 2004-06-07 20:16:00Z dsp $ """

__author__ = "$Author: dsp $"

__URL__ = "$URL: http://astro.pas.rochester.edu/svn/pydsp/trunk/pydsp/hawaii.py $"


detkeys = ["voffset", "heater", "vreset", "dsub" ]

namemap = {"vreset":"da0", "dsub":"da1", "heater":"ad0", "temp":"ad1"}

