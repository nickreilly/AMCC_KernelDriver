"""
cmc_dark+light.py - a script to run in pydsp.
    
  1) aquires images in SUTR mode with the first set in the dark, 
  2) moves the filter wheel to filter with high background, 
  3) sets the reset clock to zero (off),
  4) takes more SUTR images in the light
  5) moves the filter wheel back to cds (dark)
  6) takes more SUTR data in the dark to allow diodes to debias to zero.
  7) sets the reset clock back to default.

to use, type: execuser filename
where filename is this file's name, but WITHOUT the .py
"""

from run import rd
#import numpy
#import xdir
#import pyfits
import time
#import filterBase
#import ls332


object_base = "firstframe_test"
rd.object= object_base

print "Taking Garbage Data"
for i in range(100):
	sscan()
numsub=8
subheight=2048/numsub
pedrun()
for subarray in range(0, 2*numsub-1):
	rd.nrow = subheight
	rd.itime = 1500
	rd.nsamp = 50
	rd.nrowskip = subarray*(2048/(2*numsub))
	sutr()

	rd.nrowskip = 0
	rd.nrow = 2048
	rd.itime = 11000
	rd.nsamp = 1
	dd.resetnhi = 0
	pedrun()
	dd.resetnhi = 3300
	
print "Done taking your data!"
rd.nrowskip = 0
rd.nrow = 2048
rd.itime = 11000
rd.nsamp = 1
crun()



