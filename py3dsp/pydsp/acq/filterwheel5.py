
"""
   filters.py        Filter Routines. (Replaces FILTERWH.FTH)
   concrete implementation of abstract filter.
   This is filter wheel style V.
"""

__version__ = """$Id: filterII.py 247 2004-10-23 18:52:25Z dsp $ """

__author__ = "$Author: dsp $"

__URL__ = "$URL: http://astro.pas.rochester.edu/svn/pydsp/trunk/pydsp/filterII.py $"

global filterwheelname
filterwheelname = 'filterwheel-5'
def get_wheelname() :
    return filterwheelname

# Cold aperture stop diameter in mm. USER MUST CHANGE THIS WHEN HARDWARE CHANGED
global lyotstop
lyotstop = 3.00 

global dist2lyot  # distance in mm between Detector and Lyot Stop.
dist2lyot = 40.45  #  USER MUST UPDATE!

def get_lyotstop() :
    return lyotstop

def get_dist2lyot() :
    return dist2lyot


# Set up dictionary for each CVF of the actual CVF parameters
# Keep "comment" for filters (CVF & fixed) to < 45 characters for FITS header
# Eventually, when we upgrade the OS & python (to >2.4.x) we can use numpy to
# calculate the actual bandwidth for each position on CVF.  Until then, you
# MUST always recalculate the spectral resolution for each change in aperture.

# This cvf was broken near 1.25 microns
_cvfJH = {
    "slope": -0.21607606, # position/nm, from ocli. corrected 2% to 5k 
    "start": 808.25,  # start position
    "range" : (1102.0,2259.0), # starting and ending wavelength, nanometers
    "res" : 50.0, # spectral resolution for 0.82mm aperture
    "trans" : (25.0,70.9,83.6), # OCLI values at three wavelengths
    "comment" : 'OCLI CVF JH Band p/n 1.2/2.2-6PS 1102-2259nm'
}

# The CVF 1.9/3.6-6B is a half disc covering 1.9 - 3.6.  This is that disc
# cut in half.
_cvfK = {
    "slope": -0.26539278, # position/nm, from ocli. corrected 2% to 5k 
    "start": 530.50,  # start position
    "range" : (1848.0,2790.0), # starting and ending wavelength, nanometers
    "res" : 60.3, # spectral resolution for 0.82mm aperture
    "trans" : (28.0,54.0,72.0), # OCLI values at three wavelengths
    "comment" : 'OCLI CVF K Band p/n 1.9/3.6-6B 1848-2790nm'
}

# 17-May-95 calibrated this CVF using Br beta line.
_cvfL = {
    "slope": -0.11091393, # position/nm, from ocli. corrected 2% to 5k 
    "start": 252.75,  # start position
    "range" : (2318.0,4572.0), # starting and ending wavelength, nanometers
    "res" : 60.3, # spectral resolution for 0.82mm aperture
    "trans" : (60.0,71.0,50.0), # OCLI values at three wavelengths
    "comment" : 'OCLI CVF 563-097/101 Segment 1 2318-4572nm'
}
# Set up a dictionary to select which dictionary of CVF parameters to use
cvfparam = {'cvfJH':_cvfJH, 'cvfK': _cvfK ,'cvfL': _cvfL}

# Dictionary of the all the fixed filters.
# Keep "comment" for filters (CVF & fixed) to < 45 characters for FITS header
# Zero fw occurs 1 degree = 1000/360 = 2.778 fw past the L-band CVF.
# The six fixed filters are separated by 10 deg = 1000.0 * 10/360 = 27.778 fw
# CDS is halfway between JH-band CVF and K-band CVF (largest blank section).
_fix = {
        "cds" : {"pos":544.50, "wvl":0.0,     "bnd":0.0,    "trans":0.0, 
                 "comment":"Cold Dark Slide"},
        "cd2" : {"pos":989.00, "wvl":0.0,     "bnd":0.0,    "trans":0.0, 
                 "comment":"Cold Dark Slide 2 - not as dark"},
        "j"   : {"pos":975.00, "wvl":1250.0,   "bnd":230.0,  "trans":66.0,
                 "comment":"From Barbara Jones UCSD, calib temp unknown"},
        "h"   : {"pos":947.25, "wvl":1652.0,   "bnd":318.0,  "trans":76.0,
                 "comment":"From Dick Joyce, OCLI cal at 77K "},
        "k"   : {"pos":919.50, "wvl":2227.0,  "bnd":405.0,  "trans":85.0,
                 "comment":"From Dick Joyce, OCLI cal at 77K"},
        "3.3" : {"pos":891.75, "wvl":3255.0,  "bnd":230.0,  "trans":83.0,
                 "comment":"From OCLI via C. Woodward, calibrated at 77K"},
        "l''"  : {"pos":864.00, "wvl":3807.0,  "bnd":621.0,  "trans":70.0,
                 "comment":"From OCLI-LTD, cal at room corrected to 77K"},
        "m'"  : {"pos":836.00, "wvl":4667.0,  "bnd":176.0,  "trans":88.0,
                 "comment":"OCLI M' calibrated at 77K"},
}


