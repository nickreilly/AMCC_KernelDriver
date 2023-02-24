#!/usr/bin/python
# movieshow
# a "noise map" of the array could be done using a similar technique.
# 

"""
movieshow. can be used at the command line or imported as a module.

It does require pydsp modules to be available.
movieshow was never used very much, and the data rate seemed to overwhelm DV.
"""
__version__ = """$Id: movieshow.py 400 2006-06-19 22:39:30Z drew $ """

__author__ = "$Author: drew $"

__URL__ = "$URL: http://astro.pas.rochester.edu/svn/pydsp/trunk/pydsp/movieshow.py $"

import astropy.io.fits as pyfits
import numpy
import os
import sys
import dv
import time
import xdir

def movieshow(infile = None) :
    """
    Progress through an image cube and compute successive deltas (i.e. a movie).
    """
    if not infile :
        if xdir.objnum : 
            infile = xdir.objfileroot%xdir.objnum 
        else :
            print("need sutr object!")
            return

    pyfile = pyfits.open( infile ) # read the base file first.
    header = pyfile[0].header

    filename = os.path.basename(infile)
    outfile = "/tmp/temp.delt"

    naxis = header["NAXIS"]
    # if we are passed a 2D file,
    # assume it is the basename for U/R sutr data.
    if naxis == 2 :
        nsamp = header["NSAMP"] 
    # if we are passed a 3D file, assume it is a data cube.
    elif naxis == 3 :
        nsamp = header["NAXIS3"]
    else:
    # otherwise, we have a problem.
        print("need 2D or 3D file!!")
        return

    #coef = optco(nsamp) # get a list of coefficients.
    shape = (header["NAXIS2"] , header["NAXIS1"]) # plane size (nrow, ncol)

    if naxis == 3 :
        print("reading data cube")
        pydata = pyfile[0].data  # the data cube

    pedfile = pyfits.open( infile+".%03d"%2 ) 
    peddata = pedfile[0].data
    for plane in range(1,nsamp) :
        if naxis == 2 :
            sigfile = pyfits.open( infile+".%03d"%(plane+1) )
            sigdata = sigfile[0].data
            nextped = numpy.array(sigdata, numpy.float32)

        print("plane # %d" % plane)

        # seems wasteful to get array of zeros
        # then add in.
        # numpy.array(sigdata, numpy.float32) 
        # didn't seem to work.
        sigdata -= peddata

        peddata = nextped

        if os.path.exists(outfile) :
            os.remove(outfile)
        sigfile.writeto(outfile)
        dv.load_bkg(outfile)
        time.sleep(2)
        

if __name__ == "__main__" :
    if len(sys.argv) == 2  :
        print(sys.argv[1])
        movieshow(sys.argv[1])
    else :
        print("usage: %s filename "%os.path.basename(sys.argv[0]))
        
#else :
    #print "%s finished loading" % __name__
