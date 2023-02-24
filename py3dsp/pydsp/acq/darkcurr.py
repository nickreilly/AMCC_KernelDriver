#
# darkcurr.py
#

"""
"""

rd['object'] = 'darkcurr'
rd['nrow'] = 512
rd['ncol'] = 512
rd['nrowskip'] = 0
rd['ncolskip'] = 0
rd['gc'] = 'dark current testing'
import time
def vbias(bias):
    dd.vbias = bias

# 10 images per, 
def dodarks() :
  for v_bias in range( 0, 202, 25) : # 8 different biases
    for vreset in range (-50, 152, 25) :  # 6 reset levels
        dd['vreset'] = vreset
        vbias(v_bias)
        dd['voffset'] =  - (2000 + vreset)
        rd['nsamp'] = 4
        rd['itime'] = 10000
        sscan()
        sscan()
        rd['lc'] = 'dark current testing, pedestal'
        pedrun()
        pedrun()
        rd['lc'] = 'dark current testing, short image'
        srun()
        srun()
        rd['nsamp'] = 32
        rd['itime'] = 100000
        srun()
        srun()
        rd['nsamp'] = 64
        rd['itime'] = 300000 
        srun()
        srun()
        print "did one loop! yay!"
