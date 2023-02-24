from run import rd
#import numpy
#import xdir
#import pyfits
import time
import filterBase


def WaitAndBurst():
    time.sleep(waittime) # wait for temperature to stabilize
    burst()  # stop CRUN mode
    time.sleep(5)
    burst()  # sometimes, once is not enough
    time.sleep(5)


def get_mosaic_img():
  # Put comment in header
  rd.gc = 'sub-array reads'
  # Change the number of rows
  rd.nrow = 16
  for rowskip in range(0, 2032, 16):
    rd.nrowskip = rowskip
    sscan() # Take an image, but do not save
    sscan() # do two because we want a better reset than just once.
    # Take 4 images at each of the rowskips
    for blah in range(0, 4):
        srun()

rd.object = "LWIR_subarrays"
rd.ncol = 2048

#waittime in seconds
waittime = 600
WaitAndBurst()

#Here we take the subarray data for wavelengths in CVFIII
wavelengths = [8250, 8500, 8750, 9000, 9250, 9500, 9750]

for i in range(len(wavelengths)):
    filterBase.set(("cvfIII", wavelengths[i]))
    rd.itime = 400
    get_mosaic_img()  # note: itime, nrow set in sub-routine

rd.nrowskip = 0
rd.nrow = 2048

#Here we take the full array data on CDS
rd.itime = 11000
filterBase.set("cds")
for blah in range(0, 2):
    sscan()

