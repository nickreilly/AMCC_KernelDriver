#
# majornvs.py
#

"""
"""

rd['nrow'] = 512
rd['ncol'] = 512
rd['nrowskip'] = 0
rd['ncolskip'] = 0
# This is going to give us 52 images spaced by the 
# amount of itime shown below.
NSAMPS = 110
TEMPDEFAULT = 30.0
BIASDEFAULT = 0

testconfigbias = (0, 50, 100, 200)
import time
    
itimes=[2,2.5,3,3.5,4,5]

def vbias(bias) :
    """
    vbias has been moved to det dictionary.
    """
    dd.vbias = bias

# 4 images per loop.
def tradnvs(itimes) :
   rd['itime'], rd['nsamp'] = 2000, 1
   sscan()
   for itime in itimes : # list of itimes
         rd['itime'] = itime*1000
         for i in range (40) :  # 40 images each itime
             srun()
             print "Written image %d of itime %d seconds."%(i+1,itime)

def tradnvsbias(itimes) :
    rd['nsamp']=1
    for bias in testconfigbias :
        rd['object']="nvskfil_%dmV"%(bias)
        biassetup(bias)
        for itime in itimes :
            rd['itime'] = itime*1000
            for i in range (40) :  # 40 images each itime
                srun()
                print "Written image %d of itime %d seconds."%(i+1,itime)
    biassetup(BIASDEFAULT)

             
def takesutrdata(itime) :
    rd['nsamp']=1
    rd['itime']=5000
    sscan()
    rd['nsamp']=NSAMPS
    rd['itime']=itime
    sutr()

#def scansettle(secs = 600) :
#    "just let temp settle for a while. (10 min default)"
#    start = time.time()
#    rd['nsamp']=1
#    rd['itime']=8000
#    while time.time() - start < secs :
#        tc.do()
#        tmps()
#        sscan()

#def tempsetup(temp) :
#    if tc.carrot == temp : return
#    tc.time = time.time()
#    tc.goal = temp
#    rd['nsamp']=1
#    rd['itime']=10000
#    while tc.carrot != tc.goal :
#        tc.do()
#        tmps()
#        sscan()
#    start = time.time()
#    print "slew done. settling."
#    while time.time() - start < 600 :
#        tc.do()
#        tmps()
#        sscan()
        
def biassetup(bias) :
    if dd['dsub'] - dd['vreset'] == bias : return
    vbias(bias)
    rd['nsamp']=1
    rd['itime']=5000
    start = time.time()
    while time.time() - start < 120 :
        sscan()
    # rd['gc'] = 'Vbias = %d.'%(config)
        
#def satnlinearity() :
#    for temp,itime in testconfigtemp :
#        rd['object']="%dK_%ds"%(temp,itime)
#        tempsetup(temp)
#        takesutrdata(itime*1000)
#    tempsetup(TEMPDEFAULT)
        
def satnlinearity() :
    for bias in testconfigbias :
        itime = rd['itime']
        rd['object']="sat%dmV_%ds"%(bias,itime/1000)
        biassetup(bias)
        takesutrdata(itime)
    biassetup(BIASDEFAULT)

#def majornvs() :
#    scansettle(1000)
#    tc.degrees_K_per_minute=.1
#    for temp,itime in testconfigtemp :
#        tempsetup(temp)
#        for bias in testconfigbias :
#            rd['object']="%dK_%dmV_%ds"%(temp,bias,itime)
#            biassetup(bias)
#            takesutrdata(itime*1000)
#        biassetup(BIASDEFAULT)
#    tempsetup(TEMPDEFAULT)

