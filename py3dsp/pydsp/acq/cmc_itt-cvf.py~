"""
cmc_nvs.py - a script to run in pydsp.
    
    aquires images for a series of integration times, and calculates the 
    noise squared and signal for each image pair.

to use, type: execuser cmc_nvs
"""

from autouser import AutoUser
from run import rd
from pydsp import cloop
from numarray import *
import xdir
import pyfits
import time

# To move the filter wheel outside of AutoUser() is:
# filterBase.set("k")
# OR for CVF, put name and wavelength in tuple --> extra ():
# filterBase.set(("cvfL", 3200))

# 
user = AutoUser(
    "itime 3000",
    "filter cvfL 4400", 
    "srun",
    "srun",
    "filter cvfL 4000", 
    "srun",
    "srun",
    "filter cvfL 3800", 
    "srun",
    "srun",
    "filter cvfL 3600", 
    "srun",
    "srun",
    "filter cvfL 3400", 
    "srun",
    "srun",
    "itime 3000",
    "filter cvfL 3200",
    "srun",
    "srun",
    "filter cvfK 2200",
    "itime 3000",
    "sscan",
    "itime 30000",
    "srun",
    "srun",
    "itime 3000",
    "filter k",
    "itime 6000",
    "srun",
    "srun",
)
cloop(user)
print 'Finished taking data'

