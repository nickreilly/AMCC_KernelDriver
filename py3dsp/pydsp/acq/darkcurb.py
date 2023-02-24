#
# darkcur2.py
#

"""
"""

# rd['object'] = 'darkcur2'
rd['nrow'] = 512
rd['ncol'] = 512
rd['nrowskip'] = 0
rd['ncolskip'] = 0
rd['gc'] = 'Original Method Dark Current Testing'

# 4 images per loop.
def traddarkl() :
  rd['itime'], rd['nsamp'] = 2000, 1
  sscan()
  rd['nsamp'] = 16      
  for itime in [15,60,200,500,1000,2000,3000] : # list of itimes
        rd['itime'] = itime*1000
        for i in range (4) :  # 4 images each itime
            srun()
            print "Written image %d of itime %d seconds."%(i+1,itime)
