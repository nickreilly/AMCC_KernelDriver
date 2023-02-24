"""
majordark.py - a script to run in pydsp.
does sample up the ramp
to use, type: execuser majordark
"""

from autouser import AutoUser
from run import rd
from pydsp import cloop

def vbias(bias) :
    """
    vbias has been moved to det dictionary.
    """
    dd.vbias = bias

# This is going to give us 52 images spaced by the 
# amount of itime shown below.
NSAMPS = 3000
TEMPDEFAULT = 30.0
BIASDEFAULT = 0

# heater voltage and itime for sutr spacing
testconfigtemp = (
#    (36.0, 40),
#    (39.0, 30),
#    (42.0, 20),
#    (45.0, 10),
    (27.0, 60),
    (30.0, 60),
    (33.0, 50),
)

# testconfigbias = (0, 50, 100, 200)
# testconfigbias = (-20, 0, 20, 40, 60, 80, 100, 200)
curtest = (2370, 2390, 2400, 2410, 2430)
# curtest = (2340, 2360, 2380, 2400, 2420, 2440)
import time
    
# 4 images per loop.
# def traddarkl(itimes):
#   rd.itime, rd.nsamp = 2000, 1
#   sscan()
#   rd.nsamp = 16      
#   for itime in itimes: # list of itimes
#         rd.itime = itime*1000
#         for i in range (4):  # 4 images each itime
#             srun()
#             print "Written image %d of itime %d seconds."%(i+1,itime)

def takesutrdata(itime=2000):
    """Do one sscan, then sutr with global nsamps and specified itime."""
    user = AutoUser(
        "nsamp 1",
        "itime 5000",
        "sscan",
        "nsamp %d" % NSAMPS,
        "itime %d" % itime,
        "sutr"
    )
    cloop(user)

def takersutr(itime=2000):
    """Do one sscan, then sutr with global nsamps and specified itime."""
    user = AutoUser(
        "nsamp 1",
        "itime 5000",
        "rscan",
        "nsamp %d" % NSAMPS,
        "itime %d" % itime,
        "rsutr",
        "nsamp 1"
    )
    cloop(user)

def sutrvcur(itime=2000):
    for vltg in curtest:
        rd.object = "30Kcur%dmV_VBG"%(vltg)
        dd.vgate = vltg
        takersutr(itime)

gatetest = (0, 350, 700, 1050, 1350)
def sutrvreset(itime=2000):
    offset = dd.voffset
    for vltg in gatetest:
        rd.object = "30K_%dmV_Vrst"%(vltg)
        dd.vreset = vltg
        dd.voffset = offset - vltg*9/10
        takersutr(itime)
    dd.vreset = 0
    dd.voffset = offset

def biassetup(bias):
    """If bias needs changing, change it and wait for it to settle.
    return immediately otherwise."""
    if dd.dsub - dd.vreset == bias:
        return
    vbias(bias)
    # scan for a few minutes
    itime = rd.itime
    nsamp = rd.nsamp
    rd.nsamp = 1
    rd.itime = 5000
    start = time.time()
    while time.time() - start < 300:
        sscan()
    rd.itime = itime
    rd.nsamp = nsamp
    # rd.gc = 'Vbias = %d.'%(config)
    
def darkvsbias():
    for bias in testconfigbias:
        itime = rd.itime
        rd.object="%dmV_%ds"%(bias,itime/1000)
        biassetup(bias)
        takesutrdata(itime)
    biassetup(BIASDEFAULT)

"""        
def darkvstemp():
    for temp,itime in testconfigtemp:
        rd.object="%dK_%ds"%(temp,itime)
        tc_goto(temp)
        
        takesutrdata(itime*1000)
    tempsetup(TEMPDEFAULT)
        

def majordark():
    time.sleep(1000)
    tc.degrees_K_per_minute=.1
    for temp,itime in testconfigtemp:
        tempsetup(temp)
        for bias in testconfigbias:
            rd.object="%dK_%dmV_%ds"%(temp,bias,itime)
            biassetup(bias)
            takesutrdata(itime*1000)
        biassetup(BIASDEFAULT)
    tempsetup(TEMPDEFAULT)

def darknstrpvsb(itime=rd.itime):
    for bias in testconfigbias:
        itimesutr = 100
        biassetup(bias)
        rd.object="sutrmo%dmV_%dms"%(bias,itimesutr)
        stripemo()
        rd.object="dark%dmV_%ds"%(bias,itime/1000)
        takesutrdata(itime)
    biassetup(BIASDEFAULT)
"""

