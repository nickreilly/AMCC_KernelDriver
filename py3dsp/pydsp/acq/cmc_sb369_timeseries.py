"""
cmc_sb369_timeseries.py - a script to run in pydsp.
takes a few pedrun for large numbers of columns, i.e. time series
to use, type: execuser cmc_sb369_timeseries
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

def fullarraytimeseries():
    # Set up the detector parameters.
    user = AutoUser(
        "nrow 408",
        "ncol 128",
        "nrowskip 0",
        "ncolskip 0",
        "nsamp 1 ",
        "itime %d" % ITIME,
        "seeclockpin 18",
    )
    cloop(user)
    # Do a few pedscans to get the array into a more stable state.
    pedscan()
    pedscan()
    pedscan()
    pedscan()
    # Now set up the detector parameters for the time series.
    user = AutoUser(
        "ncol 24000",
    )
    cloop(user)
    # Use DataWord to set "Full Array Mode" and select columns 1 through 8
    for colselect in range(0, 8, 1):
        # Bits 27-29 select one of the eight columns to use, see page 105.  
        lsdataword = 0x4ac3 + colselect*0x0008
        sb369dwLS(lsdataword)
        # Change object directories
        user = AutoUser( "lc column = %d" % colselect)
        cloop(user)
        # Take data for each column
        pedrun()
    # Done.
    # Set everything back to nominal
    sb369dwLS(0x4ac3) # column 1 again
    sb369dwMS(0x80dd) # det bias = -5.08mV
    user = AutoUser(
        "ncol 128",
        "itime %d" % ITIME,
    )
    cloop(user)
    pedscan() # Just to be sure those values are clocked into array
    crun
    # sb369off()

