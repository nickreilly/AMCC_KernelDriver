
"""
   filterwheel4.py        Filter Routines. (Replaces FILTERWH.FTH)
   concrete implementation of abstract filter.
   This is filter wheel style IV.
"""

global filterwheelname
filterwheelname = 'filterwheel-4'
def get_wheelname() :
    return filterwheelname

# Cold aperture stop diameter in mm. USER MUST CHANGE THIS WHEN HARDWARE CHANGED
global lyotstop
lyotstop = 2.59 

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

# These are just place holders for now.  This filter wheel does not have CVFs.
_cvfII = {
    "slope": -0.06354722, # position/nm, from ocli. corrected 2% to 5k 
    "start": 541.50,  # start position
    "range" : (4126.0,8060.0), # starting and ending wavelength, nanometers
    "res" : 91.1, # spectral resolution for 0.82mm aperture
    "trans" : (43.0,38.0,43.0), # OCLI values at three wavelengths
    "comment" : 'Hey - no CVF here dummy'
}

_cvfIII = {
    "slope": -0.03628972, # position/nm, from ocli. corrected 2% to 5k 
    "start": 250.0,  # start position
    "range" : (7703.0,14592.0), # starting and ending wavelength, nanometers
    "res" : 60.3, # spectral resolution for 0.82mm aperture
    "trans" : (69.0,61.0,40.0), # OCLI values at three wavelengths
    "comment" : 'Hey - no CVF here dummy'
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
        "j"   : {"pos":888.75, "wvl":1255.0,  "bnd":298.0,  "trans":74.0,
                 "comment":"Barr, measured at 77K"},
        "h"   : {"pos":847.25, "wvl":1645.0,  "bnd":332.0,  "trans":78.0,
                 "comment":"Barr + 2mm PK-50 blocker, measured at 77K"},
        "k"   : {"pos":805.50, "wvl":2189.8,  "bnd":408.5,  "trans":73.0,
                 "comment":"Barr, measured at 77K"},
        "3.3" : {"pos":763.75, "wvl":3296.0,  "bnd":200.0,  "trans":87.0,
                 "comment":"OFC (From FW I) RoomTemp corr to 10K"},
        "l''" : {"pos":722.25, "wvl":3807.0,  "bnd":621.0,  "trans":70.0,
                 "comment":"OCLI L'', calib @ room temp, corr to 77K"},
        "m'"  : {"pos":680.50, "wvl":4666.5,  "bnd":175.9,  "trans":87.0,
                 "comment":"OCLI M' measured at 77K"},
        "mlong" : {"pos":638.75, "wvl":4892.0,  "bnd":422.0,  "trans":83.0,
                 "comment":"OCLI M' measured at 77K"},
        "vis" : {"pos":597.25, "wvl":550.0,   "bnd":300.0,  "trans":68.0,
                 "comment":"Oriel KG-5 **no calibration curve**"},
}


