"""
cmc_mosaic.py - a script to run in pydsp.
takes a few sscans at different nrowskip and ncolskip, and then concatenates
them into a mosaic image.
To use, type: execuser filename
where filename is this file's name, but WITHOUT the .py
"""

from autouser import AutoUser
from run import rd
from pydsp import cloop
import numarray
import xdir
import pyfits

outputfilename = xdir.get_objpath() + "/mosaic.fits"

rowstart = 800
rowend = 1000
colstart = 700
colend = 1100

skip = 32

for rowskip in range(rowstart, rowend, skip):
    rd.nrowskip = rowskip

    for colskip in range(colstart, colend, skip):
        rd.ncolskip = colskip
        sscan()
        # recreate file name (duplicate code from runrun)
        scanfilename = xdir.get_objpath() + "/" + rd.bufferflag
        scanfile = pyfits.open(scanfilename)
        # These are FITS files. Get just the data and ignore header.
        scandata = scanfile[0].data
        if rd.ncolskip == colstart:
            newblock = scandata
        else:
            oldblock = newblock
            newblock = numarray.concatenate((oldblock, scandata), axis=1)
        scanfile.close()

    tmprowblock = newblock # above column stripe is now row block
    if rd.nrowskip == rowstart:
        newrowblock = newblock # can't concat on empty.
    else:
        oldrowblock = newrowblock
        newrowblock = numarray.concatenate((oldrowblock, tmprowblock), axis=0)

image = newrowblock

fitsobj = pyfits.HDUList() # new fitsfile object
hdu = pyfits.PrimaryHDU()
fitsobj.append(hdu) # and
hdu.data = image
header = fitsobj[0].header
fitsobj.writeto(outputfilename)

#pyfits.writeto(outputfilename, newrowblock)
