"""
cmc_THz_vgs_signal.py - a script to run in pydsp.
    
    Script to acquire a sequence of images with Vgs voltage swept over a range.  
    We are trying to measure the signal from the Gunn diode with different Vgs voltages.
    This will tell us the optimum value of Vgs.

to use, type: execuser cmc_THz_vgs_signal
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
    
    filename = xdir.get_nextobjfilename() + '.fits'
    #filename = xdir.get_objpath() + "/" + rd.bufferflag
    for blah in range(100):
        srun()


# Put some voltages back to original.

dd.vgs = orig_vgs
dd.voffset = OrigOffset
#rd.lc = ''
pedscan()
print 'Finished taking data'

