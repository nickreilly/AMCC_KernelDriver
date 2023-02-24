"""
cmc_RTN.py - a script to run in pydsp.
    
    script to acquire a sequence of images at short integration times with reset on
    We are looking for Random Telegraph Noise (RTN)
    
to use, type: execuser cmc_RTN
"""
from run import rd

# Get temperature for filename
tmps()

# round and return as integer -- no decimal
CurrTemp = int(round(readTemp())) 

# Get current Vreset and Vbiasgate
origVreset = dd.vreset
origDsub = dd.dsub
origVbgate = dd.vbgate

# Turn ON the reset clock so that we do destructive reads.
dd.resetnlo = 3300
rd.lc = 'Reset Clock is always ON'

# Hardcode itime 
rd.itime = 5500

# Vary Vreset - hardcoded range
startreset = 100
endreset = 300
stepreset = 100
for resetvoltage in range(startreset, endreset+1, stepreset):
    dd.vreset = resetvoltage
    # zero applied bias --> reverse bias is ok, but forward bias is bad!
    dd.dsub = resetvoltage 
    rd.object = "rtn_%dK_vreset%dmV"%(CurrTemp, resetvoltage)
    rd.nsamp = 1
    sscan() # Take throw away data to settle bias change
    sscan()
    rd.nsamp = 200
    sutr()

"""
# Vary Vbiasgate - hardcoded range
dd.vreset = origVreset # Set Vreset back to nominal value
startvbgate = 2000
endvbgate = 2800
stepvbgate = 100
for biasgate in range(startvbgate, endvbgate+1, stepvbgate):
    dd.vbgate = biasgate
    rd.object = "rtn_%dK_vbgate%dmV"%(CurrTemp, biasgate)
    rd.nsamp = 1
    sscan() # Take throw away data to settle bias change
    sscan()
    rd.nsamp = 200
    sutr()
"""

print 'Finished taking data'
dd.vreset = origVreset
dd.dsub = origDsub
dd.vbgate = origVbgate # 
dd.resetnlo = 0 # Set reset clock to normal clocking.
rd.nsamp = 1
rd.lc = ''
sscan()
sscan()
