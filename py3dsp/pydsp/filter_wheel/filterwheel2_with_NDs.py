
"""
   filters.py        Filter Routines. (Replaces FILTERWH.FTH)
   concrete implementation of abstract filter.
   This is filter wheel style II.
"""

__version__ = """$Id: filterII.py 247 2004-10-23 18:52:25Z dsp $ """

__author__ = "$Author: dsp $"

__URL__ = "$URL: http://astro.pas.rochester.edu/svn/pydsp/trunk/pydsp/filterII.py $"

global filterwheelname
filterwheelname = 'filterwheel-2'
def get_wheelname() :
    return filterwheelname

# Cold aperture stop diameter in mm. USER MUST CHANGE THIS WHEN HARDWARE CHANGED
global lyotstop
lyotstop = 0.050
#lyotstop = 0.82 
#lyotstop = 1.16

global dist2lyot  # distance in mm between Detector and Lyot Stop.
dist2lyot = 43.76  #  USER MUST UPDATE!

def get_lyotstop() :
    return lyotstop

def get_dist2lyot() :
    return dist2lyot


# Set up dictionary for each CVF of the actual CVF parameters
# Keep "comment" for filters (CVF & fixed) to < 45 characters for FITS header
# Eventually, when we upgrade the OS & python (to >2.4.x) we can use numpy to
# calculate the actual bandwidth for each position on CVF.  Until then, you
# MUST always recalculate the spectral resolution for each change in aperture.
_cvfII = {
    "slope": -0.06354722, # position/nm, from ocli. corrected 2% to 5k 
    "start": 541.50,  # start position
    "range" : (4126.0,8060.0), # starting and ending wavelength, nanometers
    "res" : 91.1, # spectral resolution for 0.82mm aperture
    "trans" : (43.0,38.0,43.0), # OCLI values at three wavelengths
    "comment" : 'OCLI CVF II 4126-8060nm'
}

_cvfIII = {
    "slope": -0.03628972, # position/nm, from ocli. corrected 2% to 5k 
    "start": 250.0,  # start position
    "range" : (7703.0,14592.0), # starting and ending wavelength, nanometers
    "res" : 60.3, # spectral resolution for 0.82mm aperture
    "trans" : (69.0,61.0,40.0), # OCLI values at three wavelengths
    "comment" : 'OCLI CVF III 7703-14592nm'
}

# Set up a dictionary to select which dictionary of CVF parameters to use
cvfparam = {'cvfII': _cvfII ,'cvfIII': _cvfIII}

# Dictionary of the all the fixed filters.
# Keep "comment" for filters (CVF & fixed) to < 45 characters for FITS header
# The eight fixed filters are separated by 15 deg = 1000.0 * 15/360 = 41.667 fw
# CDS is 20 degrees from either the first fixed filter or the CVF slot.
_fix = {
        "cds" : {"pos":944.50, "wvl":0.0,     "bnd":0.0,    "trans":0.0, 
                 "comment":"Cold Dark Slide"},
        "vis" : {"pos":888.75, "wvl":500.0,   "bnd":300.0,  "trans":80.0,
                 "comment":"Oriel KG-5 **no calibration curve**"},
        "k"   : {"pos":847.25, "wvl":2189.8,  "bnd":408.5,  "trans":76.0,
                 "comment":"Barr, measured at 77K"},
        "3.3" : {"pos":805.50, "wvl":3269.0,  "bnd":200.0,  "trans":87.0,
                 "comment":"OFC (From FW I) corr to 10K"},
        "l'"  : {"pos":763.75, "wvl":3745.0,  "bnd":807.0,  "trans":0.0,
                 "comment":"Dick Joyce l' + ND (OD 1.5)"},
        "m'"  : {"pos":722.25, "wvl":4666.5,  "bnd":175.9,  "trans":87.0,
                 "comment":"OCLI M' measured at 77K + ND (OD 2.2)"},
        "5.8" : {"pos":680.50, "wvl":5804.0,  "bnd":1399.0, "trans":69.1,
                 "comment":"OCLI OF3-W6 IRAC corr to 4K + ND (OD 2.95)"},
        "8.6" : {"pos":638.75, "wvl":8622.0,  "bnd":968.0,  "trans":82.0,
                 "comment":"OCLI W08700-9 corr to 4K + ND (OD 2.95)"}, 
        "11.6": {"pos":597.25, "wvl":11590.0, "bnd":2190.0, "trans":81.0,
                 "comment":"OCLI W11780-9 corr to 4K + ND (OD 2.95)"}, 
}


