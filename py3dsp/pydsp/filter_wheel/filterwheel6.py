
"""
   filters.py        Filter Routines. (Replaces FILTERWH.FTH)
   concrete implementation of abstract filter.
   This is filter wheel style VI.
"""

__version__ = """$Id: filterII.py 247 2004-10-23 18:52:25Z dsp $ """

__author__ = "$Author: dsp $"

__URL__ = "$URL: http://astro.pas.rochester.edu/svn/pydsp/trunk/pydsp/filterII.py $"

global filterwheelname
filterwheelname = 'filterwheel-6'
def get_wheelname() :
    return filterwheelname

# Cold aperture stop diameter in mm. USER MUST CHANGE THIS WHEN HARDWARE CHANGED
global lyotstop
lyotstop = 2.00 

global dist2lyot  # distance in mm between Detector and Lyot Stop.
dist2lyot = 43.76  #  USER MUST UPDATE!

def get_lyotstop() :
    return lyotstop

def get_dist2lyot() :
    return dist2lyot


# Keep "comment" for filters (CVF & fixed) to < 45 characters for FITS header

# Set up a dictionary to select which dictionary of CVF parameters to use
#cvfparam = {'cvfII': None ,'cvfIII': None}
cvfparam = {}

# Dictionary of the all the fixed filters.
# Keep "comment" for filters (CVF & fixed) to < 45 characters for FITS header
# The eight fixed filters are separated by 15 deg = 1000.0 * 15/360 = 41.667 fw
# CDS is 20 degrees from either the first fixed filter or the CVF slot.
#
# No filters in 1/4 inch square slots yet. Dictionary entries are placeholders.
_fix = {
        "cds"   : {"pos":55.50, "wvl":0.0,     "bnd":0.0,    "trans":0.0, 
                 "comment":"Cold Dark Slide"},
        "open1" : {"pos":958.25, "wvl":500.0,   "bnd":300.0,  "trans":80.0,
                 "comment":"Oriel KG-5 **no calibration curve**"},
        "open2" : {"pos":917.75, "wvl":2189.8,  "bnd":408.5,  "trans":76.0,
                 "comment":"Barr, measured at 77K"},
        "open3" : {"pos":875.00, "wvl":3269.0,  "bnd":200.0,  "trans":87.0,
                 "comment":"OFC (From FW I) corr to 10K"},
        "open4" : {"pos":833.25, "wvl":3745.0,  "bnd":807.0,  "trans":0.0,
                 "comment":"Dick Joyce l' + ND (OD 1.5)"},
        "open5" : {"pos":791.75, "wvl":4666.5,  "bnd":175.9,  "trans":87.0,
                 "comment":"OCLI M' measured at 77K + ND (OD 2.2)"},
        "open6" : {"pos":750.00, "wvl":5804.0,  "bnd":1399.0, "trans":69.1,
                 "comment":"OCLI OF3-W6 IRAC corr to 4K + ND (OD 2.95)"},
        "open7" : {"pos":708.25, "wvl":8622.0,  "bnd":968.0,  "trans":82.0,
                 "comment":"OCLI W08700-9 corr to 4K + ND (OD 2.95)"}, 
        "open8" : {"pos":666.75, "wvl":11590.0, "bnd":2190.0, "trans":81.0,
                 "comment":"OCLI W11780-9 corr to 4K + ND (OD 2.95)"}, 
        "openA" : {"pos":611.00, "wvl":11590.0, "bnd":2190.0, "trans":81.0,
                 "comment":"OCLI W11780-9 corr to 4K + ND (OD 2.95)"}, 
        "openB" : {"pos":555.50, "wvl":11590.0, "bnd":2190.0, "trans":81.0,
                 "comment":"OCLI W11780-9 corr to 4K + ND (OD 2.95)"}, 
        "openC" : {"pos":500.00, "wvl":11590.0, "bnd":2190.0, "trans":81.0,
                 "comment":"OCLI W11780-9 corr to 4K + ND (OD 2.95)"}, 
        "13.3"  : {"pos":402.75, "wvl":13310.0, "bnd":550.0, "trans":73.0,
                 "comment":"ABI Barr filter + JDSU Blocker, 60K calib"}, 
}


