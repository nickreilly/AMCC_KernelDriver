"""
Calculate FFT of some data.

"""

import pyfits
#import numpy
import numarray # old -- need to upgrade!!
#import matplotlib.pyplot as matplt
import matplotlib.pylab as matplt
import time # if only I could do that in real life.


# The file name formats are different between UR, OCIW & TIS.  Also, they can
# change when their respective data taking software changes.  So, it is easiest
# to make the user input a list of FITS file names instead of generating names 
# for the user e.g. generate names based on incrementing of R?? for TIS data.
# Also, file lists add more flexibility since we can now input a non-contiguous
# set of images, e.g. for H2RG_R??_M01_N01.fits, with input 01-05, 07, 10-32.
print '\n\nThe input file name is really a text file that is a list of FITS file \nnames with paths, where each line corresponds to a separate FITS file to \nbe read.  To create this list of files, use something like: \n  ls /path/fsr_0002_??.fits > myinputlist \nOR if ls generates stupid Unicode hex stuff: \n  find `pwd` -name "fsr_0002_??_rowcorr.fits" > myinputlist '
print 'Enter input file name, including path:'
inputfilelist = raw_input()

print 'Do you want to view plots of reference pixels and the fitted values? (y or n)'
doplots = raw_input()
if doplots == 'y':
    print 'Should this program wait for user before going ahead to the next plot? (y or n) \n If not, each plot will display for 1 second plus program processing time.'
    plotwait = raw_input()



def ShowMyPlot(x1, y1, x2, y2, plottitle, plotwait):
    matplt.close() # Issue plot close at beginning of functions, rather
    # than end, to give more time to view plots.
    matplt.plot(x1, y1, '.', x2, y2, '-')
    matplt.suptitle(plottitle)
    matplt.xlabel('Pixel number')
    matplt.ylabel('Pixel value (ADU)')
    matplt.show() # doesn't stop program here to view fit, except 1st time.
    #matplt.axis([0,2100,-50,50])
    time.sleep(0.5) # leave full scale image up for a bit to see outliers
    # Now, calculate the Y-limits rejecting outliers (99% autoscaling).
    mymaxy = numpy.median(y1) + 3.0*numpy.std(y1)
    myminy = numpy.median(y1) - 3.0*numpy.std(y1)
    rejectlow = numpy.where(y1 > myminy, y1, numpy.nan)
    rejecthilow = numpy.where(rejectlow < mymaxy, rejectlow, numpy.nan)
    rejectnoNaN = rejecthilow[~numpy.isnan(rejecthilow)]
    mymaxy = numpy.median(rejectnoNaN) + 5.0*numpy.std(rejectnoNaN)
    myminy = numpy.median(rejectnoNaN) - 5.0*numpy.std(rejectnoNaN)
    matplt.ylim(myminy,mymaxy)
    if plotwait == 'y':
        print 'Press Enter when you are ready to go to next plot.'
        blahblah = raw_input()
    else :
        time.sleep(1.0)

