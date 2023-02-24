"""
cmc_dcgain.py - a script to run in pydsp.
    
    script to acquire a sequence of pedestal images
    with the reset always asserted, the bias always
    at zero, and the reset voltage swept over a range.

to use, type: execuser cmc_dcgain
"""

from autouser import AutoUser
from run import rd
from pydsp import cloop
import xdir
import pyfits
import time
import ls332 #Not needed. Including on 10/15/19 to change temperature after finished


# Ask user for SCA type
print 'Which SCA? (H1RG, H2RG, SB304)'
whichSCA = str(raw_input())

#print 'How long do you want to wait for temperature stability before starting?'
#print '    (enter in seconds)'
#waittime = int(raw_input())  
#time.sleep(waittime)

print 'Taking source follower FET gain data. Reset is always on.'

burst()
time.sleep(5)
burst()
time.sleep(5)

# What is our current temperature?
tmps()
CurrTemp = int(round(readTemp())) # round and return as integer -- no decimal
# Set the object name and include the temperature in the name
#rd.object = "SFgain%dK"%(CurrTemp)
rd['object'] = 'SFgainwarmup'

rd['lc'] = 'Taking source follower gain data. Reset is always on.'
rd['nsamp'] = 1
rd['nrowskip'] = 0
rd['ncolskip'] = 0

OrigOffset= dd['voffset']

# The dual A/D board uses Analogic ADC 4325(500KHz)/4320(1MHz)/4322(2MHz):
# where +/- 5V range is +/- 32768 ADU 
# The quad ADC boards use LTC 1608: +/- 2.5V range = +/- 32768 ADU
ADCrange = dd.adrange * 1000.0 # in mV
# Normally, both are using gain = 25
#preampgain = 16.755
#preampgain = 25
preampgain = dd.ampgain
ADUtoVolt = ADCrange / ( 2**16 * preampgain)

vrst_step = 50 # number of mV to change for each sample taken

# HAWAII-1RG
if whichSCA == 'H1RG' :
    dd['resetnlo'] = 3300 # turning reset so always on
    rd['nrow'] = 1024
    rd['ncol'] = 1024
    rd['itime'] = 6000
    # Give the detector reset voltage range
    vrst_start = -800
    vrst_end = 1900

# HAWAII-2RG
if whichSCA == 'H2RG' :
    dd['resetnlo'] = 3300 # turning reset so always on
    rd['nrow'] = 2048
    rd['ncol'] = 2048
    rd['itime'] = 11000
    # Give the detector reset voltage range
    vrst_start = -800
    vrst_end = 1900

# SB-304
if whichSCA == 'SB304' :
    # Turn both global and row reset to always on.
    dd['pRstG_lo'] = -5200
    dd['pRstG_hi'] = -5200
    dd['pRstR_lo'] = -5200
    dd['pRstR_hi'] = -5200
    rd['nrow'] = 2048
    rd['ncol'] = 2048
    rd['itime'] = 11000
    # Give the detector reset voltage range
    vrst_start = -5000
    vrst_end = -1000

# Get some initial data to make sure the beginning of this data ramp is
# still on scale.
dd['vreset'] = vrst_start
dd['dsub'] = vrst_start
pedscan()
pedfilename = xdir.get_objpath() + "/" + rd.bufferflag + ".fits"
pedfile = pyfits.open(pedfilename)
peddata = pedfile[0].data
value1 = peddata[200:240, 200:240].mean()
value2 = peddata[700:740, 200:240].mean()
value3 = peddata[200:240, 700:740].mean()
value4 = peddata[700:740, 700:740].mean()
value = (value1 + value2 + value3 + value4)/4.
if value > 15000 :
    dd['voffset'] = OrigOffset - vrst_step*4
if value < -15000 :
    dd['voffset'] = OrigOffset + vrst_step*4
pedfile.close()

pedscan()
pedfilename = xdir.get_objpath() + "/" + rd.bufferflag + ".fits"
pedfile = pyfits.open(pedfilename)
peddata = pedfile[0].data
value1 = peddata[200:240, 200:240].mean()
value2 = peddata[700:740, 200:240].mean()
value3 = peddata[200:240, 700:740].mean()
value4 = peddata[700:740, 700:740].mean()
value = (value1 + value2 + value3 + value4)/4.
if value > 15000 :
    dd['voffset'] = OrigOffset - vrst_step*4
if value < -15000 :
    dd['voffset'] = OrigOffset + vrst_step*4
pedfile.close()

pedscan()
pedfilename = xdir.get_objpath() + "/" + rd.bufferflag + ".fits"
pedfile = pyfits.open(pedfilename)
peddata = pedfile[0].data
value1 = peddata[200:240, 200:240].mean()
value2 = peddata[700:740, 200:240].mean()
value3 = peddata[200:240, 700:740].mean()
value4 = peddata[700:740, 700:740].mean()
value = (value1 + value2 + value3 + value4)/4.
if value > 15000 :
    dd['voffset'] = OrigOffset - vrst_step*4
if value < -15000 :
    dd['voffset'] = OrigOffset + vrst_step*4
pedfile.close()

pedscan()
pedfilename = xdir.get_objpath() + "/" + rd.bufferflag + ".fits"
pedfile = pyfits.open(pedfilename)
peddata = pedfile[0].data
value1 = peddata[200:240, 200:240].mean()
value2 = peddata[700:740, 200:240].mean()
value3 = peddata[200:240, 700:740].mean()
value4 = peddata[700:740, 700:740].mean()
value = (value1 + value2 + value3 + value4)/4.
if value > 15000 :
    dd['voffset'] = OrigOffset - vrst_step*4
if value < -15000 :
    dd['voffset'] = OrigOffset + vrst_step*4
pedfile.close()

# Open a file to store data, using "a" = append mode
wfile = open(xdir.get_objpath() + "/"+rd['object']+".txt", "a")


########Delete me after!!!!!!!###########
time.sleep(1)
ls332.setSetpointTemp(22) # Tell controller to change temperature
time.sleep(1)
time.sleep(3600)

while CurrTemp < 23: 
    tmps()
    CurrTemp = int(round(readTemp()))   

# Now the array output should be on scale, so take the data for real.
v=450
dd['voffset']=-2170
tmps()
CurrTemp = int(round(readTemp()))
#for v in range( vrst_start, vrst_end, vrst_step) :
while CurrTemp < 75: 
    tmps()
    CurrTemp = int(round(readTemp()))    
# Need to store the preamp offset voltage.
    oldoffset= dd['voffset']

    dd['vreset'] = v
    dd['dsub'] = v
    time.sleep(3)
    pedfilename = xdir.get_nextobjfilename() + ".fits"
    #pedfilename = xdir.get_objpath() + "/" + rd.bufferflag + ".fits"
    pedrun()
    pedfile = pyfits.open(pedfilename)
    # These are FITS files. Get just the data and ignore header.
    peddata = pedfile[0].data
    # Write some statistics.
    value1 = peddata[200:240, 200:240].mean()
    value2 = peddata[700:740, 200:240].mean()
    value3 = peddata[200:240, 700:740].mean()
    value4 = peddata[700:740, 700:740].mean()
    #value5 = peddata[1700:1740, 1700:1740].mean()
    value = (value1 + value2 + value3 + value4)/4.

    # Convert from ADUs to milliVolts
    volts = value*ADUtoVolt - (dd['voffset'])
    volts1 = value1*ADUtoVolt - (dd['voffset'])
    volts2 = value2*ADUtoVolt - (dd['voffset'])
    volts3 = value3*ADUtoVolt - (dd['voffset'])
    volts4 = value4*ADUtoVolt - (dd['voffset'])
    #volts5 = value5*ADUtoVolt - (dd['voffset'])


    wfile.write("%s " % dd['vreset'])
    wfile.write("%s " % volts1)
    wfile.write("%s " % volts2)
    wfile.write("%s " % volts3)
    wfile.write("%s " % volts4)
    #wfile.write("%s " % volts5)
    wfile.write("%s " % value1)
    wfile.write("%s " % value2)
    wfile.write("%s " % value3)
    wfile.write("%s " % value4)
    #wfile.write("%s " % value5)
    wfile.write("%s " % dd['voffset'])
    wfile.write("\n")
    pedfile.close()

    print "# vreset Output-ADU  Output-Voltage    voffset"
    print dd['vreset'], value, volts, dd['voffset']

    # We need to make sure the voltage after amplifiers is within range of
    # the A/D converters.  So, if the average value in ADU is above or below
    # the specified number, then adjust Voffset to put everything back in range
    if value > 15000 :
        dd['voffset'] = oldoffset - vrst_step*2
    if value < -15000 :
        dd['voffset'] = oldoffset + vrst_step*2

    
wfile.close()

# Put some voltages back to original (turn off reset).

# HAWAII-1RG
if whichSCA == 'H1RG' :
    dd['resetnlo'] = 0
    dd['vreset'] = 100
    dd['dsub'] = 250

# HAWAII-2RG
if whichSCA == 'H2RG' :
    dd['resetnlo'] = 0
    dd['vreset'] = 100
    dd['dsub'] = 250

# SB-304
if whichSCA == 'SB304' :
    dd['pRstG_lo'] = -3000
    dd['pRstG_hi'] = -3000
    dd['pRstR_lo'] = -5200
    dd['pRstR_hi'] = -3000
    dd['vreset'] = -3200
    dd['dsub'] = -2900


dd['voffset'] = OrigOffset
rd['lc'] = ''
pedscan()
print 'The reset clock was returned to its original state.'
print 'Finished taking dcgain data'

crun()

