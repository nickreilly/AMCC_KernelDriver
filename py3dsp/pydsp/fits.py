"""
fits.py.. wrapper for pyfits, puts good header stuff in fits files.

"""

__revision__ = __version__ = """$Id: fits.py 400 2006-06-19 22:39:30Z drew $ """

__author__ = "$Author: drew $"

__URL__ = "$URL: http://astro.pas.rochester.edu/svn/pydsp/trunk/pydsp/fits.py $"

# print "loading fits.py"
# import pyfits  # the fits file..
import astropy.io.fits as pyfits
import numpy #
import det
import run # to know the rundata
import time # time goes into the image

# U/R header has start and end time for scan.
# there are header lines that have start " COMMENT"
# and header lines that start " HISTORY"

# it is notable that this file does NOT have knowledge of sending
# the fits image to dv!

'''
data buffers in memory???

old notes from Drew, on numarray:
digging into numarray..
grab_image calls ...
numarray.array ...
numarray.NewArray ...
numarray.NumArray ... (still, with buffer, shape, type..) 
we finally get into 
NumArray.__init__ (which calls, buffer, shape, and itemsize..) 
numarray.NDArray.__init__ 
Which calls memory.new_memory(size)
size is self.nelements times itemsize
'''
# we could loop thru things, but
# the only funtion that would need to is fitsheader..
'''
fitskeys = [
    "nrowskip",
    "ncolskip",
    "object",
    "telescope",
    "observer",
    "time-obs",
    "sec-obs",
    "ftime",
    ]
'''

# descramblers.
# These functions take an image as a pyfits header data unit (hdu)
# and rearrange the pixels to the proper arrangement.

def interleaved_descrambler(hdu):
    "For interleaved or single output, there is no need to descramble"
    # So do nothing.
        
def block_descrambler(hdu):
    "descramble any number of outputs into block stripe formatted array."
    # Note: it is up to the user to be sure that ncol/nout is an integer.
    # Start with the first block from first output because it is a bad thing
    # to try to start with an empty array and use numpy.concatenate with
    # another array that is not empty.  In the limit of 1 output, this does
    # the same as the interleaved_descrambler function, i.e. nothing!
    newblock = hdu.data[:, ::det.dd.nout]
    i=1
    for i in range(1,det.dd.nout):
        tmpblock = hdu.data[:, i::det.dd.nout]
        oldblock = newblock
        newblock = numpy.concatenate((oldblock, tmpblock), axis=1)
    hdu.data = newblock

def quadrant_descrambler(hdu):
    "descramble a four output array in quadrant format."
    # From all rows, take every 4th pixel (column).
    upperleft  = hdu.data[:, ::4]
    upperright = hdu.data[:, 1::4]
    lowerright = hdu.data[:, 2::4]
    lowerleft  = hdu.data[:, 3::4]
    # Unfortunately, the above quads are not the right shape (2x row, 1/2 col).
    # So, reshape them into squares that are nrow/2 by ncol/2 in size.
    # Technically, the image does not have to be square, and this still works.
    sqrupperright = numpy.reshape(upperright, (run.rd.nrow/2,run.rd.ncol/2))
    sqrupperleft  = numpy.reshape(upperleft, (run.rd.nrow/2,run.rd.ncol/2))
    sqrlowerright = numpy.reshape(lowerright, (run.rd.nrow/2,run.rd.ncol/2))
    sqrlowerleft  = numpy.reshape(lowerleft, (run.rd.nrow/2,run.rd.ncol/2))
    # Put it all back together.
    upperhalf = numpy.concatenate((sqrupperleft, sqrupperright), axis=1)
    lowerhalf = numpy.concatenate((sqrlowerleft, sqrlowerright), axis=1)
    hdu.data = numpy.concatenate((upperhalf, lowerhalf), axis=0)

def no_rowshiftreg_descrambler(hdu):
    "descramble any number of outputs into separate rows for each output, and write formatted array."
    # Note: it is up to the user to be sure that ncol/nout is an integer.
    # Start with the first block from first output because it is a bad thing
    # to try to start with an empty array and use numpy.concatenate with
    # another array that is not empty.  In the limit of 1 output, this does
    # the same as the interleaved_descrambler function, i.e. nothing!
    tmpblock = hdu.data[:, ::det.dd.nout]
    newblock = numpy.reshape(tmpblock, (1,run.rd.ncol))
    i=1
    for i in range(1,det.dd.nout):
        tmpblock = hdu.data[:, i::det.dd.nout]
        # the above has nout-number of rows.  But really it is one row.  So make it that
        rowblock = numpy.reshape(tmpblock, (1,run.rd.ncol))
        oldblock = newblock
        # now put the previous rows together with this new row.
        newblock = numpy.concatenate((oldblock, rowblock), axis=0)
    hdu.data = newblock

def row_col_reverse_descrambler(hdu):
    "descramble an array that has rows and cols switched."
    # number of pixels per row is run.rd.ncol, but the number of pixels
    # read out may span different number of columns/rows for non-square
    # images.  This call of numpy.reshape will rearrange the array 
    # to swap rows and cols.  However, a vertical rectangle now becomes
    # a horizontal rectangle.  So, this isn't the complete solution, but
    # does make it easier to rearrange (otherwise need a set of nested
    # for loops to get individual pixels and put them in a new array).
    reversearray = numpy.reshape(hdu.data, (run.rd.ncol,run.rd.nrow))
    # now put array back into the correct orientation
    hdu.data = numpy.transpose(reversearray)

# To flip an array in the left-right direction use this:
# tmpblockrev = tmpblock[:, ::-1] # flip Left-Right
       
# Setup a dictionary to allow choice of output formats. 
descramble = {
  "interleaved":interleaved_descrambler,
  "block":block_descrambler,
  "quadrant":quadrant_descrambler,
  "reverse":row_col_reverse_descrambler,
  "no_rowshiftreg":no_rowshiftreg_descrambler,
  None:interleaved_descrambler
}

# we do source and background buffers as fits objects.
# seems like we'd want to inherit here..
class fitsbuffer:
    def __init__(self, buf=None):
        self.HDU = pyfits.HDUList() # This is the big one.
        self.HDU.append(pyfits.PrimaryHDU()) # append ONE HDU
        self.buf = buf  # need to keep buffer handy.
        self.header = self.HDU[0].header # short cut to the header.
        self.header.update('BITPIX', 16, comment= "signed shorts")
        self.header.update('NAXIS', 2)
    def set_size(self, height, width):
        self.HDU[0].data = numpy.ndarray(buffer=self.buf,
            shape=(height, width), dtype=numpy.int16)
        self.header.update('NAXIS1', width)
        self.header.update('NAXIS2', height)
#    def descramble(self):
#        hawaii_descrambler(self.HDU[0])

    
def fitsheader(header):
    """
    Update the header with the current information from the system.
    """
    header.update('NOUT', det.dd.nout, comment="Number of outputs")
    header.update('OUTFORM', det.dd.outputformat, comment="Output Format: interleaved, block, quad")
    header.update('NIMAGE', 1, comment="Num of images stacked - NOT USED")
    header.update('NCOLSKIP', run.rd.ncolskip, comment="Cols skipped")
    header.update('NROWSKIP', run.rd.nrowskip, comment="Rows skipped")
    header.update('OBJECT', run.rd.object)
    header.update('NIGHT', run.rd.night)
    header.update('TELESCOP', run.rd.telescope)
    header.update('OBSERVER', run.rd.observer)
    gmtime = time.gmtime()
    header.update('DATE-OBS', "%d_%d_%d"%gmtime[0:3])
    header.update('TIME-OBS', "%d:%d:%d GMT"%gmtime[3:6])
    # header.update('SEC-OBS', 0, comment="in secs since night dir's midnight")
    header.update('FTIME', run.rd.ftime,
                            comment="local time before first pedestal pixel")
    header.update('LTIME', run.rd.ltime,
                            comment="local time after last signal pixel")
    header.update('ITIME', run.rd.itime, comment="integration time in msec")
    header.update('FWHEEL', str(run.rd.fwn), comment="Filter Wheel Name")
    header.update('FILTER', str(run.rd.filter), comment=str(run.rd.filtercomment))
    header.update('LAMBDA', run.rd.wavelength, comment="Wavelength in nm")
    header.update('BNDWIDTH', run.rd.bandwidth, comment="Bandwidth in nm")
    header.update('TRANSMIT', run.rd.transmission, comment="Filter Transmission in percent")
    header.update('FWP', run.rd.fwp, comment="Filter Wheel Position, motor steps")
    header.update('LYOT', run.rd.lyotstop, comment="Cold aperture stop diameter in mm")
    header.update('DISTLYOT', run.rd.dist2lyot, comment="Distance in mm between Lyot stop and Detector")
    header.update('OMODE', run.rd.bufferflag, comment="Observing Mode, source or background" )
    header.update('SAMPMODE', run.rd.sampmode)
    header.update('RUNMODE', "burst")

    #header.update('TEMP', "%7.3f"% run.rd.pre_temp, comment="Pre frame Temperature in Kelvin. ")
    header.update('TEMP', run.rd.pre_temp, comment="Pre frame Temperature in Kelvin. ")
    header.update('POSTTEMP', run.rd.post_temp, comment="Post frame.")
    header.update('DETFILE', det.dd.detname)

    header.update('BOXNAME', det.dd.boxname, comment="Version of detector array controller")
    header.update('ADRANGE', det.dd.adrange, comment="16-bit A to D Converter input range in volts")
    header.update('AMPGAIN', det.dd.ampgain, comment="Amp Gain prior to A2D convert")

    header.update('CLOCKPGM', det.dd.clockpgm)
    header.update('CTSTIME', run.rd.ctstime, comment="Clamp To Sample Time in 0.1 microsec")
#    header.update('ADCTIME', run.rd.adctime)
    header.update('NSAMP', run.rd.nsamp, comment="Number of samples for SUTR or Fowler")
    header.update('COMMENT1', run.rd.gc)
    header.update('COMMENT2', run.rd.lc)
    header.update('BSCALE', 1)
    header.update('BZERO', 0)
    for bias in det.biaslist:
        #header.update(bias, str(det.dd[bias]))
        header.update(bias, det.dd[bias])

def write_image(filename, obuf):
    """update a new buffer with the proper fits stuff and write to disk.
    
    This is a mix of two responsibilities.
    getting the proper time and temp data is one thing..
    sending the image to the disk is another.
    they 'should' be separated. Not a biggie though."""
    import os.path
    fitsobj = pyfits.HDUList() # new fitsfile object
    hdu = pyfits.PrimaryHDU() 
    fitsobj.append(hdu) # and 
    hdu.data = obuf
    header = fitsobj[0].header
    fitsheader(header) # add the rest of the runinfo to the header
    header.update('DATAFILE', os.path.basename(filename))
    descramble[det.dd.outputformat](hdu)
    fitsobj.writeto(filename)

