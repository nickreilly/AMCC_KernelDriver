"""
cmc_THz_vgs_Ids.py - a script to run in pydsp.
    
    Script to acquire a sequence of pedestal images with Vgs voltage swept over a range.  
    This is meant to allow the user to then calculate the current through the load
    resistor and thus the current through the drain to source of the FET, 
    and knowing that, then calculate the channel resistance of the FET.

to use, type: execuser cmc_THz_vgs_Ids
"""


from run import rd
import xdir
import pyfits


print 'Taking data while varying Vgs. '

print 'What is the name of the object file (where do we store this data)?'
rd.object = str(raw_input())
#rd.object = 'vgs-vs-current'

print 'At what value of Vgs do you want to start?'
vgs_start = int(raw_input())
# vgs_start = 300
print 'At what value of Vgs do you want to end?'
vgs_end = int(raw_input())
print 'By what value do you want increment Vgs?'
vgs_step = int(raw_input())

print 'What rows are connected to the output?'
rd.lc = 'row ' + str(raw_input())
#rd.nsamp = 1
#rd.nrowskip = 0
#rd.ncolskip = 0



# Store some of the initial voltage settings
OrigOffset= dd.voffset # Need to store the preamp offset voltage.
orig_vgs = dd.vgs

# The dual A/D board uses Analogic ADC 4325(500KHz)/4320(1MHz)/4322(2MHz):
# where +/- 5V range is +/- 32768 ADU 
# The quad ADC boards use LTC 1608: +/- 2.5V range = +/- 32768 ADU
# Normally, both are using gain = 25

# Now both these values are written in the det file for each detector 
# and stored in the dictionary dd
ADUtoVolt = dd.adrange / ( 2**16 * dd.ampgain)


# Open a file to store data, using "a" = append mode
wfile = open(xdir.get_objpath() + "/"+rd.object+".txt", "a")

# Now the array output should be on scale, so take the data for real.
for v in range(vgs_start, vgs_end, vgs_step) :
    # set Vgs
    dd.vgs = v
    
    # We need to make sure the voltage after amplifiers is within range of
    # the A/D converters.  So, if the average value in ADU is above or below
    # the specified number, then adjust Voffset to put everything back in range
    pixval = 32768
    while pixval < -15000 or pixval > 15000 :
        pixval = check_offset(50) # set Voffset if out of bounds (see cmc_THzGen1.py)
    
    pedfilename = xdir.get_nextobjfilename() + '.fits'
    #pedfilename = xdir.get_objpath() + "/" + rd.bufferflag
    pedrun()
    pedfile = pyfits.open(pedfilename)
    # These are FITS files. Get just the data and ignore header.
    peddata = pedfile[0].data
    # Write some statistics.
    value1 = peddata[0, 0:11].mean()
    value2 = peddata[1, 0:11].mean()
    
    value = (value1 + value2)/2.

    # Convert from ADUs to milliVolts
    # Seems there is a factor of 2 somewhere in the offset that is being missed.
    volts = value*ADUtoVolt*1000 - (dd.voffset/2.)
    volts1 = value1*ADUtoVolt*1000 - (dd.voffset/2.)
    volts2 = value2*ADUtoVolt*1000 - (dd.voffset/2.)

    wfile.write("%s " % dd.vgs)
    wfile.write("%s " % str(dd.voffset/2.))

    wfile.write("%s " % volts1)
    wfile.write("%s " % volts2)

    # now write each pixel separately to the file
    for i in range(0,2) :
        for j in range(0,12) :
            pixvolts = peddata[i,j] * ADUtoVolt*1000 - dd.voffset/2.
            wfile.write("%s " % pixvolts)

    wfile.write("\n")
    pedfile.close()

    print "# vreset Output-ADU  Output-Voltage    voffset"
    print dd.vgs, value, volts, dd.voffset

wfile.close()

# Put some voltages back to original.

dd.vgs = orig_vgs
dd.voffset = OrigOffset
#rd.lc = ''
pedscan()
print 'Finished taking data'

