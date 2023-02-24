from run import rd
import numpy
import xdir
import pyfits
import filterBase
import time
import ls332

detbiases = [250,350]

subarrayitime = 3000
skip=4
rowend = 64
fullrowend=2048
filterBase.set("cds")

time.sleep(5)
burst()
time.sleep(5)
burst()
time.sleep(5)
burst()

tmps()
CurrTemp = int(round(ls332.readTemp())) # 

for detbias in detbiases:
    # The applied reverse detector bias = (dsub-vreset), invert to get dsub
    detsub = dd.vreset + detbias
    dd.dsub = detsub

    # Set the object name for this data with the bias in the name.
    rd.object = "Pedscans_%dK_%dmV"%(CurrTemp, detbias)

    rd.nsamp = 1
    rd.nrowskip = 0 
    rd.nrow = 2048
    rd.itime = 11000
    for blah in range(20):
        sscan() # just take some throw-away data to help new bias sett

    
    rd.nrow = rowend
    rd.nrowskip = skip
    rd.itime = 400
    for blah in range(100):
        pedrun()

rd.nrowskip = 0
rd.nrow = fullrowend
rd.itime = 11000
crun2()


