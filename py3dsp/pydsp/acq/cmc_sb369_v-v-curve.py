"""
cmc_sb369_v-v-curve.py - a script to run in pydsp.
takes a few pedrun at different bias voltages 
to use, type: execuser cmc_sb369_v-v-curve
NOTE: you must load the cmc_sb369 file first!  That has the turn on/off
sequence and the command to update the DataWord.
"""

from autouser import AutoUser
from run import rd
from pydsp import cloop
import numarray
import xdir
import pyfits

ITIME = 336

# The dual A/D board uses Analogic ADC 4325(500KHz)/4320(1MHz)/4322(2MHz):
# where +/- 5V range is +/- 32768 ADU 
# The quad ADC boards use LTC 1608: +/- 2.5V range = +/- 32768 ADU
ADCrange = 2500.0 # in mV
# Normally, both are using gain = 25
preampgain = 2.17417
ADUtoVolt = ADCrange / ( 2**15 * preampgain)

def setupVVcurve():
    """Set up the detector parameters."""
    user = AutoUser(
        "nrow 408",
        "ncol 260",
        "nrowskip 0",
        "ncolskip 0",
        "nsamp 1 ",
        "itime %d" % ITIME,
        "seeclockpin 18",
    )
    cloop(user)


def flipbits(x):
    """
    reverse bits in a byte
    The idea is to first swap the two nybbles, then swap bits 0,
    1, 5, 6 with 2, 3, 6, 7 respectively, and finally swap bits 0,
    2, 4, 6 with bits 1, 3, 5, 7 respectively.
    """
    x1 = x << 4 | x >> 4
    x2 = (x1 & 0x33) << 2 | (x1 & 0xcc) >> 2
    return (x2 & 0x55) << 1 | (x2 & 0xaa) >> 1


def singlerundata():

    # recreate file name (duplicate code from runrun)
    # Need to setup file name before doing [ped,s,b]run, otherwise this
    # variable will refer to the NEXT file, which hasn't been taken yet.
    # I should be able to use xdir.get_objfilename() -- without the next --
    # but it is not working...?
    # For runs:
    pedfilename = xdir.get_nextobjfilename()
    # For scans:
    #pedfilename = xdir.get_objpath() + "/" + rd.bufferflag
    # Now, take some data
    pedrun()
    # Now that the filename is set and the data were taken, open the file.
    pedfile = pyfits.open(pedfilename)
    # These are FITS files. Get just the data and ignore header.
    peddata = pedfile[0].data
    # Write some statistics to screen.
    print "# Temp Voffset PED-mean  PED-StDev"
    print rd.pre_temp, dd.voffset, peddata[100:201, 4:259].mean(), peddata[100:201, 4:259].stddev()
    # Open a file, using "a" = append mode
    meanfile = open(xdir.get_objpath() + "/" + rd.object + "_mean.dat", "a")
    stdevfile = open(xdir.get_objpath() + "/" + rd.object + "_stdev.dat", "a")
    # Write some statistics to file.
    meanfile.write("%s\t" % rd.pre_temp)
    meanfile.write("%s\t" % dd.voffset)
    stdevfile.write("%s\t" % rd.pre_temp)
    stdevfile.write("%s\t" % dd.voffset)
    for rowindex in range(0, 408, 1):
        # Write the mean and standard deviation per row.  Both are in mV. 
        pedmean = peddata[rowindex:rowindex+1, 4:259].mean() * ADUtoVolt - dd.voffset
        meanfile.write("%s\t" % pedmean)
        pedstdev = peddata[rowindex:rowindex+1, 4:259].stddev() * ADUtoVolt
        stdevfile.write("%s\t" % pedstdev)
    meanfile.write("\n")
    meanfile.close()
    stdevfile.write("\n")
    stdevfile.close()
    # Check video output voltage.  Are we still in range for A/D converters:
    # So, at a gain of 2, a change in voffset of 500 mV corresponds 
    # to 1V on video.
    # Change it slightly less, want it near 0 ADU.
    oldoffset = dd.voffset
    # Right now, the SB-369 output ranges from about 2.3 - 5.3 volts, the
    # best value of voffset is -4100mV, and changing it won't help.  
    # if peddata[100:120, 100:120].mean() > 25000 :
        #dd.voffset = oldoffset - 500
    # if peddata[100:120, 100:120].mean() < -25000 :
        #dd.voffset = oldoffset + 500
    pedfile.close()

def VVcurve():
    setupVVcurve()
    # The detector bias is set using DataWord bits 2-7 (and 8 for sign).
    # The tables on pages 72 & 73 show codes in MSB-LSB (bits 7,6,5,4,3,2) form
    # which is NOT HOW THEY SHOULD BE WRITTEN TO THE DATA WORD.  Reverse order!
    # This will increment the bias in 0.5mV steps by sending DataWord values of
    # 0x80dd, 0xc0dd, 0xa0dd, 0xe0dd,... 0xfedd incremented at the 2nd bit 
    # from left.  Well, the DataWord is not really incremented, but the 
    # reverse bit order is incremented.
    #
    # First do this for negative biases on page 72, bit 8 = 0. 
    # Start from -5mV go to -39mV
    # The for loop starts at 1000,0000 and goes to 1111,1110, 
    # incrementing the reverse order bits.  
    # Change +0x02 to larger number if you want to do less bias increments.
    for biasbits in range(flipbits(0x80), (flipbits(0xfe)+0x02), +0x02):
        # Convert the flipped 8 bits to the proper 16 bit half dataword
        revbias = flipbits(biasbits)*0x100 + 0x00dd
        # Send that 16 bit half dataword to the DSP to clock into the ROIC
        sb369dwMS(revbias)
        # For the negative biases on page 72, this gives an approximate bias
        # value in mV.  Basically, think of this as linear fit to table data.
        hex2bias = (biasbits - 0x01)/2 * -0.5404201 + -5.0842087
        user = AutoUser( "lc bias = %f" % hex2bias)
        cloop(user)
        # Write the DataWord and bias value to files.
        meanfile = open(xdir.get_objpath() + "/" + rd.object + "_mean.dat", "a")
        stdevfile = open(xdir.get_objpath() + "/" + rd.object + "_stdev.dat", "a")
        meanfile.write("%s\t" % hex(revbias))
        stdevfile.write("%s\t" % hex(revbias))
        meanfile.write("%s\t" % hex2bias)
        stdevfile.write("%s\t" % hex2bias)
        meanfile.close()
        stdevfile.close()
        print "\n" 
        print "Bias = ", hex2bias
        singlerundata()

    # Now do this for "positive" biases on page 73. Range is -5mV to +27mV
    for biasbits in range(flipbits(0x81), (flipbits(0xff)+0x02), +0x02):
        revbias = flipbits(biasbits)*0x100 + 0x00dd
        sb369dwMS(revbias)
        # For the positive biases on page 73, this gives an 
        # approximate bias value in mV.
        hex2bias = (biasbits - 0x81)/2 * 0.5108873 + -5.0842087
        user = AutoUser( "lc bias = %f" % hex2bias)
        cloop(user)

        meanfile = open(xdir.get_objpath() + "/" + rd.object + "_mean.dat", "a")
        stdevfile = open(xdir.get_objpath() + "/" + rd.object + "_stdev.dat", "a")
        meanfile.write("%s\t" % hex(revbias))
        stdevfile.write("%s\t" % hex(revbias))
        meanfile.write("%s\t" % hex2bias)
        stdevfile.write("%s\t" % hex2bias)
        meanfile.close()
        stdevfile.close()
        print "\n" 
        print "Bias = ", hex2bias
        singlerundata()


def fullarrayVV():
    # Use DataWord to set "Full Array Mode" and select columns 1 through 8
    for colselect in range(0, 8, 1):
        # Bits 27-29 select one of the eight columns to use, see page 105.  
        lsdataword = 0x4ac3 + colselect*0x0008
        sb369dwLS(lsdataword)
        # Change object directories
        user = AutoUser( "object v-v_bb_col_%d" % colselect)
        cloop(user)
        # Take data for each bias
        VVcurve()
    # Done.
    # Set everything back to nominal
    sb369dwLS(0x4ac3) # column 1 again
    sb369dwMS(0x80dd) # det bias = -5.08mV
    pedscan # Just to be sure those values are clocked into array
    # sb369off()

