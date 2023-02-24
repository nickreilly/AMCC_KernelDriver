"""
Process the pixel noise in a sample-up-the-ramp image.
We want to look at the pixel value relative to its
horizontal neighbors and to its mean value.
Is the distribution Gaussian?
the sum of the fourth powers should help tell us that. Hopefully.

To do this accurately, we look at the pixels with the row noise removed.
We remove the row noise in the Fourier domain
by zeroing a single column of frequencies.

TODO: could fit the pixels to a polynomial over time
or over temperature..
"""

import pyfits
import numarray
import pylab
from numarray.fft import fft2d, inverse_fft2d
from numarray import Float64, abs, zeros

class sutrproc :
    """Sample up the ramp processing
    Characterize noisy pixels in an image cube.
    """
    def __init__(self, basefile, basen=1):
        """Save the first image as an (arbitrary) reference.
        """
        self.basefile=basefile
        self.first = pyfits.open(self.basefile%basen)[0].data
        self.n = 0
        self.sum = zeros(type=Float64, shape = self.first.shape)
        self.sumsq = zeros(type=Float64, shape = self.first.shape)
        self.sum4 = zeros(type=Float64, shape = self.first.shape)
        self.dodiff = True
        self.temps = []

    def process(self, imagenum):
        """Take the diff from the first image.
        remove the DC component and row noise in the Fourier domain
        Accumulate the statistics too.
        """
        diff = pyfits.open(self.basefile%(imagenum+1))[0].data
        if self.dodiff:
            diff -= pyfits.open(self.basefile%(imagenum))[0].data
        else:
            diff -= self.first
        diff = numarray.array(diff,Float64) # do with high precision
        self.fft = fft2d(diff)
        for i in range(diff.shape[0]):
            self.fft[i,0] = 0.0 # <<< remove the row noise (clear that column)
        # note 1: clearing the column is pretty simple,
        # and easily done in spatial domain too.
        # a more elaborate scheme would
        # shape this attenuation.
        # note 2: row noise could be attenuated pre-de-scramble using
        # a 1-D approach. This may be more sensible.
        # note 3: Now that we know which pixels are high variance
        # shouldn't we ignore them for the row noise estimator?
        # A second pass that weights inversely to pixel variance might work nicely.
        self.fdiff = abs(inverse_fft2d(self.fft)) # back to the spatial domain
        # at this point, self.fdiff has the row noise removed, and it is zero mean.
        # accumulate statistics
        self.n += 1
        self.sum += self.fdiff
        squared = self.fdiff*self.fdiff
        self.sumsq += squared
        self.sum4 += squared*squared
        
    def show(self):
        pylab.imshow(self.fdiff)

    def finalstats(self):
        self.mean = self.sum / self.n
        self.variance = self.sumsq / self.n - self.mean*self.mean
        # For a gaussian variable:
        # E[X^4] = 3\sigma^4 + 6sigma^2\mu^2 + \mu^4
        self.gausscheck = self.sum4/self.n 
        self.gausscheck -= 3*self.variance*self.variance
        self.gausscheck -= 6*self.variance*self.mean*self.mean
        self.gausscheck -= self.mean*self.mean*self.mean*self.mean

    def dotemps(self, maxnum=3000):
        for i in range(1, maxnum):
            f = pyfits.open(self.basefile%i)[0]
            self.temps.append(float(f.header['TEMP']))

    def doramp(self, maxnum=3000):
        for i in range(2, maxnum):
            self.process(i)
            print("done with", i)
        self.finalstats()

    def savestats(self, dest="out%s.fits"):
        f = pyfits.open(self.basefile%1)
        f[0].data = numarray.array(self.mean, type=numarray.Int32)
        f.writeto(dest%"mean")
        f[0].data = numarray.array(self.variance, type=numarray.Float32)
        f.writeto(dest%"var")
        f[0].data = numarray.array(self.gausscheck, type=numarray.Float32)
        f.writeto(dest%"_4th")

basefile = "/data/data/Hawaii1RG/H1RG-16-002/77KBurstRst/77KBurstRst.%03d.%s"
sp = sutrproc(basefile%(7,"%03d"))

