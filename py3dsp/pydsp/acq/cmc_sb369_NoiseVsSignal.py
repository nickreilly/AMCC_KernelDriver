"""
cmc_sb369_NoiseVsSignal.py - a script to run in pydsp.
Takes a few pedrun at different integration times and bias voltages. 
With some photons hitting the array, this should give a "Noise Squared
Versus Signal" to measure capacitance.
to use, type: execuser cmc_sb369_NoiseVsSignal
NOTE: you must load the cmc_sb369 file first!  That has the turn on/off
sequence and the command to update the DataWord.
"""

from autouser import AutoUser
from run import rd
from pydsp import cloop
import numarray
import xdir
import pyfits
import math


# Only do ONE bias setting.  Could make it do more (see V-V-curve file)
sb369dwMS(0x80dd) # det bias = -5.08mV
revbias = -5.08

# The dual A/D board uses Analogic ADC 4325(500KHz)/4320(1MHz)/4322(2MHz):
# where +/- 5V range is +/- 32768 ADU 
# The quad ADC boards use LTC 1608: +/- 2.5V range = +/- 32768 ADU
ADCrange = 2500.0 # in mV
# Normally, both are using gain = 25
preampgain = 2.17417
ADUtoVolt = ADCrange / ( 2**15 * preampgain)

def setupNVS():
    """Set up the detector parameters."""
    user = AutoUser(
        "nrow 408",
        "ncol 260",
        "nrowskip 0",
        "ncolskip 0",
        "nsamp 1 ",
        "seeclockpin 18",
        "gc light=roomTemp=20.8C, DetBias=%f" % revbias,
    )
    cloop(user)


def NVSdata():
    setupNVS()
    """
    The integration time is set in pydsp using the usual method 
    (e.g. itime 120).  However, this is not the integration time in actual
    milliseconds.  Instead, it is the number of clock cycles (PMC) of 
    integration, i.e. the actual integration time is determined by 
    multiplying the # of PMC by the CTStime.  For example: if itime = 336
    and ctstime=2.6, then the actual integration time is 
    336 * 2.64us * 2 + 1.6us(overhead) = 1.7757ms.  The range in integration
    times is 60 PMC (min) to 336 PMC (max integration time)

    For ncol=132, the PYDSP system thinks (incorrecly) that the minimum
    integration is 238ms, but I'm not using itime as ms since it is PMC.
    Too much work to disable the itime checker.  Make do with it... 
    If you call pedscan() or pedrun() with the parentheses (versus not),
    then the first instance of pedscan will have the itime that you
    set, but the next will have what PYDSP thinks is the minimum.  So,
    always set itime, before pedrun().  Then you can get the minimum
    itime = 60 PMC even if you are doing large numbers of columns.
    """
    for inttime in range(60, 337, 1) :
        # recreate file name (duplicate code from runrun)
        # Need to setup file name before doing [ped,s,b]run, otherwise this
        # variable will refer to the NEXT file, which hasn't been taken yet.
        # I should be able to use xdir.get_objfilename() -- without the next --
        # but it is not working...?
        # For runs:
        pedfilenameA = xdir.get_nextobjfilename()
        # For scans:
        #pedfilename = xdir.get_objpath() + "/" + rd.bufferflag
        user = AutoUser( "lc itime = %d" % inttime)
        cloop(user)
        # Now, take some data
        rd['itime']=inttime
        pedrun()
        # Now that the filename is set and the data were taken, open the file.
        pedfileA = pyfits.open(pedfilenameA)
        # These are FITS files. Get just the data and ignore header.
        peddataA = pedfileA[0].data
        # Repeat for second pedfile
        pedfilenameB = xdir.get_nextobjfilename()
        rd['itime']=inttime
        pedrun()
        pedfileB = pyfits.open(pedfilenameB)
        peddataB = pedfileB[0].data
        # Subtract one from the other to get the noise from it
        peddiff = peddataB - peddataA
        # Write some statistics to screen.
        print "# Temp Voffset PED-mean  PED-StDev"
        print rd.pre_temp, dd.voffset, peddataA[200:201, 100:259].mean(), peddataA[200:201, 100:259].stddev(), peddiff[200:201, 100:259].stddev()/math.sqrt(2)
        # Open a file, using "a" = append mode
        statsfile = open(xdir.get_objpath() + "/" + rd.object + "_stats.dat", "a")
        for rowindex in range(0, 408, 1):
            # Write the mean and standard deviation per row.  Both are in mV. 
            # Ignore the first 100 samples in each row (stabilize?)
            pedmean = peddataA[rowindex:rowindex+1, 100:259].mean() * ADUtoVolt - dd.voffset
            # No sqrt(2) on first on, yes on second.
            pedstdev = peddataA[rowindex:rowindex+1, 100:259].stddev() * ADUtoVolt
            peddiffstdev = peddiff[rowindex:rowindex+1, 100:259].stddev() * ADUtoVolt / math.sqrt(2)
            statsfile.write("%s\t" % pedmean)
            statsfile.write("%s\t" % pedstdev)
            statsfile.write("%s\t" % peddiffstdev)
        statsfile.write("\n")
        statsfile.close()
        pedfileA.close()
        pedfileB.close()


def fullarrayNVS():
    pedscan()
    # Use DataWord to set "Full Array Mode" and select columns 1 through 8
    for colselect in range(0, 8, 1):
        # Bits 27-29 select one of the eight columns to use, see page 105.  
        lsdataword = 0x4ac3 + colselect*0x0008
        sb369dwLS(lsdataword)
        # Change object directories
        user = AutoUser( "object NVS_col_%d" % colselect)
        cloop(user)
        # Take data for each bias
        NVSdata()
    # Done.
    # Set everything back to nominal
    sb369dwLS(0x4ac3) # column 1 again
    sb369dwMS(0x80dd) # det bias = -5.08mV
    pedscan() # Just to be sure those values are clocked into array
    # sb369off()

