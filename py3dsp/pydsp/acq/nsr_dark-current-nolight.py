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
import filterBase
import ls332

print "Sleeping..."

object_base = "SUTR18_250mv_ar_"
object_append = ""


#sleep(600)
burst()
sleep(5)
burst()
sleep(5)
burst()



print "Taking garbage data!"
for blah in range(20):
	sscan()
numsub=8
subheight=int(2048/numsub)
sample_size = 18

#subarray- moving up by half subarray distance
#rd.object = object_base + "overwrite" + object_append + "_pedscan"
'''
#do with ped scan, NO garbage data
for subarray in range(0, 2*numsub-1):
	rd.nsamp = 1
	rd.nrow = 2048
	rd.nrowskip = 0
	rd.itime = 11000
	pedscan()	
	rd.nrowskip = subarray*(2048/(2*numsub))
	rd.nrow = subheight
	rd.itime = 1500
	rd.nsamp = sample_size
	sutr()
'''
#Take garbage data of each subarrays before taking them
num_garbage = 10
rd.object = object_base 
for subarray in range(0, 2*numsub-1):
	rd.nrowskip = subarray*(2048/(2*numsub))
	rd.nrow = subheight
	rd.itime = 1500
	rd.nsamp = 1
	for garbage in range(num_garbage):
		sscan()
	rd.nsamp = sample_size
	rd.gc = 'Taking interleaving subarrays'
	sutr()
'''
#Take a full pedscan AND garbage data of each subarrays before taking them
num_garbage = 20
rd.object = object_base + "overwrite" + object_append + "_both"
for subarray in range(0, 2*numsub-1):
	rd.nsamp = 1
	rd.nrow = 2048
	rd.nrowskip = 0
	rd.itime = 11000
	pedscan()
	rd.nrowskip = subarray*(2048/(2*numsub))
	rd.nrow = subheight
	rd.itime = 1500
	for garbage in range(num_garbage):
		sscan()
	rd.nsamp = sample_size
	sutr()
'''
#Take the full array!  
#rd.nrow=2048
#rd.nrowskip=0
#rd.object = object_base + "fullframe" + object_append
#rd.itime = 11000

#rd.nsamp = 1
#sscan()
#rd.nsamp = sample_size
#sutr()

print "Done taking your data!"
rd.nsamp = 1
rd.nrow = 2048
rd.nrowskip = 0
rd.itime = 11000
crun2()



