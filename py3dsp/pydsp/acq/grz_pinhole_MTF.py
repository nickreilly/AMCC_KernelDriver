from run import rd
import time
import filterBase


def WaitAndBurst():
    time.sleep(waittime) # wait for temperature to stabilize
    burst()  # stop CRUN mode
    time.sleep(5)
    burst()  # sometimes, once is not enough
    time.sleep(5)

rd.object = "ESF_MTF_350mV"

#waittime in seconds
waittime = 60
WaitAndBurst()


#Here we take the subarray data for wavelengths in CVFII
wavelengths = [5000, 5500, 6000, 6500]


for i in range(len(wavelengths)):
    filterBase.set(("cvfII", wavelengths[i]))
    rd.itime = 60000
    rd.nrow = 900
    rd.ncol = 2048
    rd.nrowskip = 500
    rd.ncolskip = 0
    for blah in range(0, 16):
        srun()
    rd.itime = 30000
    for blah in range(0, 16):
        srun()
    rd.itime = 20000
    for blah in range(0, 16):
        srun()   
    rd.itime = 10000
    for blah in range(0, 16):
        srun()
    rd.itime = 5000
    for blah in range(0, 16):
        srun()


wavelengths = [7000, 7500]


for i in range(len(wavelengths)):
    filterBase.set(("cvfII", wavelengths[i]))
    rd.itime = 10000
    rd.nrow = 900
    rd.ncol = 2048
    rd.nrowskip = 500
    rd.ncolskip = 0
    for blah in range(0, 16):
        srun()
    rd.itime = 8000
    for blah in range(0, 16):
        srun()
    rd.itime = 6000
    for blah in range(0, 16):
        srun()   
    rd.itime = 4000
    rd.nrow = 720
    rd.nrowskip = 640
    for blah in range(0, 16):
        srun()
    rd.itime = 2000
    rd.nrow = 360
    rd.nrowskip = 1000
    for blah in range(0, 16):
        srun()


#Here we take the subarray data for wavelengths in CVFIII
wavelengths = [8500, 9000, 9500, 10000]

for i in range(len(wavelengths)):
    filterBase.set(("cvfIII", wavelengths[i]))
    rd.itime = 2500
    rd.nrow = 450
    rd.ncol = 2048
    rd.nrowskip = 550
    rd.ncolskip = 0
    for blah in range(0, 16):
        srun()
    rd.itime = 2000
    rd.nrow = 360
    for blah in range(0, 16):
        srun()
    rd.itime = 1500
    rd.nrow = 270
    for blah in range(0, 16):
        srun()
    rd.itime = 1000
    rd.nrow = 180
    for blah in range(0, 16):
        srun()
    rd.itime = 500
    rd.nrow = 90
    for blah in range(0, 16):
        srun()

#Here we take the subarray data for wavelengths in CVFIII
wavelengths = [8500, 9000, 9500, 10000]

for i in range(len(wavelengths)):
    filterBase.set(("cvfIII", wavelengths[i]))
    rd.itime = 2500
    rd.nrow = 450
    rd.ncol = 2048
    rd.nrowskip = 910
    rd.ncolskip = 0
    for blah in range(0, 16):
        srun()
    rd.itime = 2000
    rd.nrow = 360
    rd.nrowskip = 1000
    for blah in range(0, 16):
        srun()
    rd.itime = 1500
    rd.nrow = 270
    rd.nrowskip = 1090
    for blah in range(0, 16):
        srun()
    rd.itime = 1000
    rd.nrow = 180
    rd.nrowskip = 1180
    for blah in range(0, 16):
        srun()
    rd.itime = 500
    rd.nrow = 90
    rd.nrowskip = 1270
    for blah in range(0, 16):
        srun()

rd.itime = 11000
rd.nrow = 2048
rd.ncol = 2048
rd.nrowskip = 0
rd.ncolskip = 0
sscan()
crun()

