# 
# bacnvs.py
#

def tradnvs() :
    rd['itime'], rd['nsamp'] = 2000, 1 # nsamp equals 1 for noise
    sscan()
    sscan()
    
    for itime in [2,3,4,5,6,7,8] : # list of itimes
        rd['itime'] = itime*1000
        for i in range (40) : # 40 images at each itime
            srun()
            print "Written image %d of itime %d seconds."%(i+1,itime)