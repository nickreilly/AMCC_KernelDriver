"""
cmc_warmup.py - a script to run in pydsp.
takes a few sruns at different numbers of fowler samples while the temperature is slowly warming up - read noise vs. temperature
to use, type: execuser cmc_warmup
"""

from autouser import AutoUser
from run import rd
from pydsp import cloop

ITIME = 2800

def setupwarmup():
    """Set up the detector parameters."""
    user = AutoUser(
        "nrow 256",
        "ncol 256",
        "nrowskip 0",
        "ncolskip 0",
        "itime %d" % ITIME,
    )
    cloop(user)


def noise_scan():
    import numarray
    import xdir
    import pyfits

    rd.nsamp = 1
    pedscan()
    # recreate file name (duplicate code from runrun)
    pedfilename = xdir.get_objpath() + "/" + rd.bufferflag
    pedfile = pyfits.open(pedfilename)
    # These are FITS files. Get just the data and ignore header.
    peddata = pedfile[0].data
    # Write some statistics.
    print "# PED-Temp PED-mean  PED-StDev Voffset"
    print rd.pre_temp, peddata[100:120, 100:120].mean(), peddata[100:120, 100:120].stddev(), dd.voffset
    # Open a file, using "a" = append mode
    wfile = open(xdir.get_objpath() + "/noisedataped.txt", "a")
    wfile.write("%s " % rd.pre_temp)
    wfile.write("%s " % peddata[100:220, 0:3].mean())
    wfile.write("%s " % peddata[100:220, 0:3].stddev())
    wfile.write("%s " % peddata[100:120, 100:120].mean())
    wfile.write("%s " % peddata[100:120, 100:120].stddev())
    wfile.write("%s " % peddata[200:220, 200:220].mean())
    wfile.write("%s " % peddata[200:220, 200:220].stddev())
    # Check video output voltage to make sure we are still in range for 
    # A/D converters:  +/- 5V range is +/- 32768 ADU 
    # So, at a gain of 25, a change in voffset of 200 mV corresponds 
    # to 5V on video.  Change it slightly less, want it near 0 ADU.
    oldoffset = dd.voffset
    if peddata[100:120, 100:120].mean() > 25000 :
        dd.voffset = oldoffset - 170
    if peddata[100:120, 100:120].mean() < -25000 :
        dd.voffset = oldoffset + 170
    wfile.write("%s " % dd.voffset)
    wfile.write("\n")
    wfile.close()
    pedfile.close()

    nsamptest = (1, 4, 8)
    for nsamp in nsamptest:
        rd.nsamp = nsamp
        sscan()
        srcfilename = xdir.get_objpath() + "/" + rd.bufferflag
        srcfile = pyfits.open(srcfilename)
        srcdata = srcfile[0].data
        srctemp = rd.pre_temp
        bscan()
        bkgfilename = xdir.get_objpath() + "/" + rd.bufferflag
        bkgfile = pyfits.open(bkgfilename)
        bkgdata = bkgfile[0].data
        bkgtemp = rd.pre_temp
        # need the difference of two images to get noise data
        diff = srcdata - bkgdata
        difftemp = (srctemp + bkgtemp)/ 2
        # write out stats
        # print "SRC mean = %s" % srcdata.mean()
        # print "SRC StDev = %s" % srcdata.stddev()
        # print "BKG mean = %s" % bkgdata.mean()
        # print "BKG StDev = %s" % bkgdata.stddev()
        # print "DIFF mean = %s" % diff.mean()
        # print "DIFF StDev = %s" % diff.stddev()
        print "# SRC-Temp SRC-mean SRC-StDev BKG-Temp BKG-mean BKG-StDev DIFF-Temp DIFF-mean DIFF-StDev "
        print srctemp, srcdata[100:120, 100:120].mean(), srcdata[100:120, 100:120].stddev(), bkgtemp, bkgdata[100:120, 100:120].mean(), bkgdata[100:120, 100:120].stddev(), difftemp, diff[100:120, 100:120].mean(), diff[100:120, 100:120].stddev()

        wfile = open(xdir.get_objpath() + "/noisedata%s.txt" %nsamp, "a")
        wfile.write("%s " % srctemp)
        wfile.write("%s " % srcdata[100:220, 0:3].mean())
        wfile.write("%s " % srcdata[100:220, 0:3].stddev())
        wfile.write("%s " % srcdata[100:120, 100:120].mean())
        wfile.write("%s " % srcdata[100:120, 100:120].stddev())
        wfile.write("%s " % srcdata[200:220, 200:220].mean())
        wfile.write("%s " % srcdata[200:220, 200:220].stddev())
        wfile.write("%s " % bkgtemp)
        wfile.write("%s " % bkgdata[100:220, 0:3].mean())
        wfile.write("%s " % bkgdata[100:220, 0:3].stddev()) 
        wfile.write("%s " % bkgdata[100:120, 100:120].mean())
        wfile.write("%s " % bkgdata[100:120, 100:120].stddev()) 
        wfile.write("%s " % bkgdata[200:220, 200:220].mean())
        wfile.write("%s " % bkgdata[200:220, 200:220].stddev()) 
        wfile.write("%s " % difftemp)
        wfile.write("%s " % diff[100:220, 0:3].mean())
        wfile.write("%s " % diff[100:220, 0:3].stddev())
        wfile.write("%s " % diff[100:120, 100:120].mean())
        wfile.write("%s " % diff[100:120, 100:120].stddev())
        wfile.write("%s " % diff[200:220, 200:220].mean())
        wfile.write("%s " % diff[200:220, 200:220].stddev())
        wfile.write("\n")
        # Close data files 
        srcfile.close()
        bkgfile.close()
        wfile.close()

def warmupdata():
    setupwarmup()
    # intentional infinite loop
    while 1:
        noise_scan()

