#
# darkcur2.py
#

"""
"""

def vbias(bias) :
    """
    vbias has been moved to det dictionary.
    """
    dd.vbias = bias

rd['object'] = 'darkcur2'
rd['nrow'] = 512
rd['ncol'] = 512
rd['nrowskip'] = 0
rd['ncolskip'] = 0
rd['gc'] = 'more dark current testing'
import time
import dsp 

# 10 images per loop. (8 saved)
def dodarks2() :
  for v_bias in range( 0, 102, 25) : # 4 different biases
    for vreset in range (-50, 52, 50) :  # 3 reset levels
        dd['vreset'] = vreset
        vbias(v_bias)
        dd['voffset'] =  - (2000 + vreset)
        rd['nsamp'] = 4
        rd['itime'] = 30000
        sscan()
        sscan()
        rd['lc'] = 'dark current testing, pedestal'
        pedrun()
        pedrun()
        rd['lc'] = 'dark current testing, short image'
        srun()
        srun()
        rd['nsamp'] = 32
        rd['itime'] = 600000
        srun()
        srun()
        rd['nsamp'] = 128
        rd['itime'] = 2000000 
        srun()
        srun()
        print "did one loop! yay!"

rd['nsamp'] = 32
rd['itime'] = 300000 

print 'darkcur 2 is going to run soon!'

for i in range(10) : 
    sscan() # clock the array and wait for temp to settle somewhat.

dodarks2() # run the test

rd['nsamp'] = 1
rd['itime'] = 1000 
sscan()

# change the heater back, conserve helium
dd['heater'] = 3400

# for next five hours, print out the temperature once per minute.

for i in range(300) :
    print dsp.vDiode()
    time.sleep(60)

