#
# majordark.py
#

"""
"""

def vbias(bias) :
    """
    vbias has been moved to det dictionary.
    """
    dd.vbias = bias

# rd['object'] = 'darkcur2'
rd['nrow'] = 512
rd['ncol'] = 512
rd['nrowskip'] = 0
rd['ncolskip'] = 0
# This is going to give us 52 images spaced by the 
# amount of itime shown below.
rd['nsamp'] = 52

# heater voltage and itime for sutr spacing
testconfigtemp = (
    (3000, 60),
    (3300, 60),
    (3600, 50),
    (3900, 40),
    (4200, 30),
    (4500, 10),
    (4800, 8),
    (5100, 5),
    (5400, 3),
)

testconfigbias = (0, 50, 100, 150, 200, 250)
    
# 4 images per loop.
# def traddarkl(itimes) :
#   rd['itime'], rd['nsamp'] = 2000, 1
#   sscan()
#   rd['nsamp'] = 16      
#   for itime in itimes : # list of itimes
#         rd['itime'] = itime*1000
#         for i in range (4) :  # 4 images each itime
#             srun()
#             print "Written image %d of itime %d seconds."%(i+1,itime)

def takesutrdata() :
    sscan()
    sutr()

def tempsetup(heater) :
    dd['heater'] = heater
    start = time.time()
    while time.time() - start < 7000 :
        sscan()
    # rd['gc'] = 'Heater voltage %d.'%(heater)
    
def biassetup(bias) :
    vbias(bias)
    start = time.time()
    while time.time() - start < 900 :
        sscan()
    # rd['gc'] = 'Vbias = %d.'%(config)
        
def darkvstemp() :
    for heatervoltage,itime in testconfigtemp :
        rd['itime'] = itime*1000
        tempsetup(heatervoltage)
        takesutrdata()
        # print "Written image %d of itime %d seconds."
        
def darkvsbias() :
    for bias in testconfigbias :
        biassetup(bias)
        takesutrdata()

def majordark() :
    for heatervoltage,itime in testconfigtemp :
        rd['itime'] = itime*1000
        tempsetup(heatervoltage)
        for bias in testconfigbias :
            biassetup(bias)
            takesutrdata()
    
