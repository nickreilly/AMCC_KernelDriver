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
  # put comment in header
  rd.gc = 'sub-array reads'
  for rowskip in range(0, 738+1, 246):
    rd.nrowskip = rowskip
    sscan() # Take an image, but do not save
    sscan() # do two because we want a better reset than just once.
    #here we take 16 images at each of the rowskips
    for blah in range(0, 16):
        srun()

rd.object = "pinhole_array"

#waittime in seconds
waittime = 600
WaitAndBurst()

##rd.ncol = 512

#Here we take the subarray data for wavelengths in CVFIII
wavelengths = [8250, 8500, 8750, 9000, 9250, 9500, 9750]
#subitimes = [5000, 5000, 5000, 5000, 5500, 6000, 8000]
#rd.nrowskip = 64
#rd.nrow = 256

for i in range(len(wavelengths)):
    filterBase.set(("cvfIII", wavelengths[i]))
    rd.itime = 30000
    #get_mosaic_img()  # note: itime, nrow set in sub-routine
    for blah in range(0, 16):
        srun()
    rd.itime = 60000
    for blah in range(0, 16):
        srun()


#rd.nrowskip = 0
#rd.nrow = 1024

#Here we take the full array data for wavelengths in CVFIII
#wavelengths = [10000, 10250]
#fullitimes = [18000, 120000]
#wavelengths = [8250, 8500, 8750, 9000, 9250, 9500, 9750, 10000, 10250]
#fullitimes = [6000, 6000, 6000, 6000, 6000, 6000, 6000, 18000, 120000]

#for i in range(len(wavelengths)):
#    rd.itime = fullitimes[i]
#    filterBase.set(("cvfIII", wavelengths[i]))
#    for blah in range(0, 16):
#        srun()

#Take data for 6000 nm in CVFII
rd.itime = 180000
filterBase.set(("cvfII", 6000))
for blah in range(0, 16):
    srun()

#Take data for l' filter
#rd.itime = 350000
#filterBase.set("l'")
#for blah in range(0, 16):
#    srun()


#Take data for 3300 nm filter
#rd.itime = 350000
#filterBase.set("3.3")
#for blah in range(0, 16):
#    srun()


#Here we take the full array data on CDS
rd.itime = 11000
filterBase.set("cds")
for blah in range(0, 16):
    srun()

#rd.nrowskip = 0
#rd.nrow = 1024

