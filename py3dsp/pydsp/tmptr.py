"""
tmptr_curve10.py  TS temperatures for fanout+work surface 

Reads lake shore diode voltage-temperature characteristic into
dictionary (vcurv, tcurv).  the temperatures are calculated via 
linear regression. If the voltage indicates a temperature
that is either above or below the characteristic, None is returned.

"""

__version__ = """$Id: tmptr_curve10.py 400 2006-06-19 22:39:30Z drew $ """

__author__ = "$Author: drew $"

__URL__ = "$URL: http://astro.pas.rochester.edu/svn/pydsp/trunk/pydsp/tmptr_curve10.py $"

# order in increasing voltage.
curve10 = [
(320.,   .47069),
(310.,   .49484),
(300.,   .51892),
(290.,   .54294),
(280.,   .56690),
(270.,   .59080),
(260.,   .61465),
(250.,   .63841),
(240.,   .66208),
(230.,   .68564),
(220.,   .70908),
(210.,   .73238),
(200.,   .75554),
(190.,   .77855),
(180.,   .80138),
(170.,   .82404),
(160.,   .84650),
(150.,   .86873),
(140.,   .89072),
(130.,   .91243),
(120.,   .93383),
(110.,   .95487),
(100.,   .97550),
(95.,    .98564),
(90.,    .99565),
(85.,   1.00552),
(80.,   1.01525),
(75.,   1.02482),
(70.,   1.03425),
(65.,   1.04353),
(60.,   1.05267),
(58.,   1.05629),
(56.,   1.05988),
(54.,   1.06346),
(52.,   1.06700),
(50.,   1.07053),
(48.,   1.07402),
(46.,   1.07748),
(44.,   1.08093),
(42.,   1.08436),
(40.,   1.08781),
(39.,   1.08972),
(38.,   1.09131),
(37.,   1.09324),
(36.,   1.09490),
(35.,   1.09689),
(34.,   1.09864),
(33.,   1.10071),
(32.,   1.10263),
(31.,   1.10480),
(30.,   1.10702),
(29.,   1.10945),
(28.,   1.11212),
(27.,   1.11517),
(26.,   1.11896),
(25.,   1.12463),
(24.,   1.13598),
(23.,   1.15558),
(22.,   1.17705),
(21.,   1.19645),
(20.,   1.21440),
(19.5,  1.22314),
(19.,   1.23184),
(18.5,  1.24053),
(18.,   1.24928),
(17.5,  1.25810),
(17.,   1.26702),
(16.5,  1.27607),
(16.,   1.28527),
(15.5,  1.29464),
(15.,   1.30422),
(14.5,  1.31403),
(14.,   1.32412),
(13.5,  1.33453),
(13.,   1.34530),
(12.5,  1.35647),
(12.,   1.36809),
(11.5,  1.38021),
(11.,   1.39287),
(10.5,  1.40615),
(10.,   1.42013),
(9.5,   1.43488),
(9.,    1.45048),
(8.5,   1.46700),
(8.,    1.48443),
(7.5,   1.50272),
(7.,    1.52166),
(6.5,   1.54097),
(6.,    1.56027),
(5.5,   1.57928),
(5.,    1.59782),
(4.8,   1.60506),
(4.6,   1.61220),
(4.4,   1.61920),
(4.2,   1.62602),
(4.,    1.63263),
(3.8,   1.63905),
(3.6,   1.64529),
(3.4,   1.65134),
(3.2,   1.65721),
(3.,    1.66292),
(2.8,   1.66845),
(2.6,   1.67376),
(2.4,   1.67880),
(2.2,   1.68352),
(2.,    1.68786),
(1.8,   1.69177),
(1.6,   1.69521),
(1.4,   1.69812)
]


# basic algorithm:
# find n entries in dictionary that are closest to 
# actual voltage reading.
# if entries are not found on both sides of the 
# reading, something is pretty wrong. 
# else...
# apply some order of estimator to this
# small set of nearby calibration data points.
# linear or second order is probably sensible.

_lastentry = None
def tdiode(v, table = curve10) :
    """
    Convert diode voltage to Kelvin using table (curve10 default).
    """
    global _lastentry
    if _lastentry and table[_lastentry][1] <= v <= table[_lastentry+1][1] :
        ix = _lastentry
    else :
        for i in range(len(table) - 1) :
            if table[i][1] <= v <= table[i+1][1] :
                ix = i
                break
        else : # if the for loop terminates normally --
            # that would be abnormal.
            _lastentry = None
            return 0.0 # (or raise exception)
    #print table[ix]
    alpha = (v - table[ix][1])/(table[ix+1][1]-table[ix][1])    
    #print alpha
    temp = table[ix][0] + alpha * ( table[ix+1][0]-table[ix][0])
    _lastentry = ix
    return temp

# then, return the result of the estimator.

# other temp checking:
# is the temp too warm? email somebody?
# temp control could be nice too.
# a step change in heater power
# causes a long exponential settling
# into a new temperature.
# If the settle characteristic is known,
# how much feed forward can be applied?
