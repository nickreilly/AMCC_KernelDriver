"""
Use reference pixels to correct data for temporal (1/f), temperature and/or bias drift.

"""

import pyfits
import numpy
import matplotlib.pyplot as matplt
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

# TIS data have the number of outputs in the header, but OCIW's doesn't.
print 'Enter number of outputs used to read out detector array:'
numoutputs = int(raw_input())
#numoutputs = 32

print 'Do you want to view plots of reference pixels and the fitted values? (y or n)'
doplots = raw_input()
if doplots == 'y':
    print 'Should this program wait for user before going ahead to the next plot? (y or n) \n If not, each plot will display for 1 second plus program processing time.'
    plotwait = raw_input()

print 'Which reference pixels do you want to use to correct the images?'
print 'Choose: '
print '    1 = reference pixels (4 col wide) along left & right sides'
print '        where the average of all ref pix is subtracted from the'
print '        entire frame.'
print '    2 = reference pixels (4 col wide) along left & right sides'
print '        where the average of the 8 ref pix per row are subtracted'
print '        from the respective row.'
print '    3 = reference pixels (4 col wide) along left & right sides'
print '        where a smooth curve is fit along the right 4 columns'
print '        and those smoothed values are subtracted from each row.'
print '    4 = reference pixels (4 rows wide) along the top side'
print '        where the average value of all ref pix is subtracted '
print '        from a given output block'
print '    5 = both 3 & 4'
print ' ABOVE IS NOT WORKING YET -- no need to enter any number. '
#refpixmode = raw_input()  

print 'Which set of reference pixels do you want to use for row corrections: "left" or "right"?'
LeftOrRight = raw_input()

"""
This is how to use a dictionary in place of the switch-case from other langs.
values = { 
           value1: do_some_stuff1, 
           value2: do_some_stuff2, 
           ... 
           valueN: do_some_stuffN,
         }
values.get(var, do_default_stuff)()
"""

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

def OutputRefCorr(ncol, nrow, numoutputs, imgdata):
    # Correct for output amplifier or pre-amp/ADC offsets.
    if doplots == 'y':
        x = range(0,ncol)
        colrefpix = numpy.mean(imgdata[nrow-4:nrow,:], axis=0)
        plottitle = 'All column reference pixels for output offset correction'
        # plotting the same thing twice because this func expects two sets.
        ShowMyPlot(x, colrefpix, x, colrefpix, plottitle, plotwait)

    ncolperchan = ncol / numoutputs
    print 'Number of cols per channel = %f' % (ncol / numoutputs)

    for colchan in range(0,ncol,ncolperchan):
        # Calculate the mean ref pix value for the current output channel.
        # There is a large "bias" offset for the first ~25 rows....
        # Maybe using bottom refpix rows is better than top refpix rows?
        chanrefpix = numpy.mean(imgdata[nrow-4:nrow,colchan:colchan+ncolperchan], axis=0) # Average 4 rows of ref pix to 1 row.
        chanrefpixmean = numpy.mean(imgdata[nrow-4:nrow,colchan:colchan+ncolperchan])  # Give a single number for ALL the ref pix.
        #print chanrefpix
        #print chanrefpixmean
        #print imgdata[nrow-4:nrow,colchan:colchan+ncolperchan]
        #print imgdata[:,colchan:colchan+ncolperchan]
        #print imgdata[:,colchan:colchan+ncolperchan]-chanrefpix

        # Subtract the mean ref pix value from the current output channel.
        colchancorr = numpy.array(imgdata[:,colchan:colchan+ncolperchan]-chanrefpixmean)
        # I don't understand why numpy.append can't take an empty array and
        # just append an arbitrary array to the empty one.  Numpy.append
        # needs to be assigned to a variable, since it appends to a copy of
        # the first argument and not in place.  Numpy.append can also append
        # more than 1 array at a time to the first array. 
        if colchan == 0:
            chancorrimg = colchancorr
        else:
            chancorrimg = numpy.append(chancorrimg, colchancorr, axis=1)
        
        if doplots == 'y':
            x = range(0,ncolperchan)
            chanrefpixfit = [chanrefpixmean] * ncolperchan
            plottitle = 'Reference pixels for output offset correction'
            ShowMyPlot(x, chanrefpix, x, chanrefpixfit, plottitle, plotwait)
    # done correcting the image for output offsets.
    return chancorrimg 

def Output37ColRefCorr(ncol, nrow, numoutputs, imgdata):
    # Correct for output amplifier or pre-amp/ADC offsets.
    if doplots == 'y':
        x = range(0,ncol)
        colrefpix = numpy.mean(imgdata[nrow-4:nrow,:], axis=0)
        plottitle = 'All column reference pixels for output offset correction'
        # plotting the same thing twice because this func expects two sets.
        ShowMyPlot(x, colrefpix, x, colrefpix, plottitle, plotwait)

    ncolperchan = ncol / numoutputs
    print 'Number of cols per channel = %f' % (ncol / numoutputs)
    outputnum = 0
    for colchan in range(0,ncol,ncolperchan):
        outputnum += 1
        # Calculate the mean ref pix value for the current output channel.
        # There is a large "bias" offset for the first ~25 rows....
        # Maybe using bottom refpix rows is better than top refpix rows?
        chanrefpix = numpy.mean(imgdata[nrow-4:nrow,colchan:colchan+ncolperchan], axis=0) # Average 4 rows of ref pix to 1 row.
        if outputnum % 2:
            # Output is odd.  Numbering from 1, first output is odd.
            # Odd outputs are read left-to-right.  37 column on left.
            # Give a single number for ALL the ref pix.
            chanrefpixmean37 = numpy.mean(imgdata[nrow-4:nrow, colchan:colchan+37])
            chanrefpixmeanRemainder = numpy.mean(imgdata[nrow-4:nrow, colchan+37:colchan+ncolperchan])
            # Subtract the mean ref pix value from the current output channel.
            colchancorr37 = numpy.array(imgdata[:,colchan:colchan+37]-chanrefpixmean37)
            colchancorrRemainder = numpy.array(imgdata[:,colchan+37:colchan+ncolperchan]-chanrefpixmeanRemainder)
            colchancorr = numpy.append(colchancorr37, colchancorrRemainder, axis=1)
            chanrefpixfit = [chanrefpixmean37] * 37 + [chanrefpixmeanRemainder] * (ncolperchan-37)
        else:
            # Output is even.  Numbering from 1, second output is even.
            # Even outputs are read right-to-left.  37 column on right.
            chanrefpixmean37 = numpy.mean(imgdata[nrow-4:nrow, colchan+ncolperchan-37:colchan+ncolperchan])
            chanrefpixmeanRemainder = numpy.mean(imgdata[nrow-4:nrow, colchan:colchan+ncolperchan-37])
            # Subtract the mean ref pix value from the current output channel.
            colchancorr37 = numpy.array(imgdata[:,colchan+ncolperchan-37:colchan+ncolperchan]-chanrefpixmean37)
            colchancorrRemainder = numpy.array(imgdata[:,colchan:colchan+ncolperchan-37]-chanrefpixmeanRemainder)
            colchancorr = numpy.append(colchancorrRemainder, colchancorr37, axis=1)
            chanrefpixfit = [chanrefpixmeanRemainder] * (ncolperchan-37) +  [chanrefpixmean37] * 37 
        #print chanrefpix
        #print chanrefpixmean
        #print imgdata[nrow-4:nrow,colchan:colchan+ncolperchan]
        #print imgdata[:,colchan:colchan+ncolperchan]
        #print imgdata[:,colchan:colchan+ncolperchan]-chanrefpix
        
        # I don't understand why numpy.append can't take an empty array and
        # just append an arbitrary array to the empty one.  Numpy.append
        # needs to be assigned to a variable, since it appends to a copy of
        # the first argument and not in place.  Numpy.append can also append
        # more than 1 array at a time to the first array. 
        if colchan == 0:
            chancorrimg = colchancorr
        else:
            chancorrimg = numpy.append(chancorrimg, colchancorr, axis=1)
        
        if doplots == 'y':
            x = range(0,ncolperchan)
            plottitle = 'Reference pixels for output offset correction'
            ShowMyPlot(x, chanrefpix, x, chanrefpixfit, plottitle, plotwait)

    # done correcting the image for output offsets.
    return chancorrimg 


def FirstRowsBiasRefCorr(ncol, nrow, imgdata, LeftOrRight):
    # Many times, the first few rows of an image have a high value that 
    # decreases (RC-decays) to normal voltage level of the remainder of rows.
    # This higher level is sometimes called Fowler-bias or first row bias.
    # We can correct for this....
    # Find the mean value of the 4 ref pix in each row, where axis=1 
    # collapses 4 col to 1.
    if LeftOrRight == 'left':
        rowrefpix = numpy.mean(imgdata[0:nrow,0:3], axis=1)
    else : 
        rowrefpix = numpy.mean(imgdata[0:nrow,ncol-4:ncol], axis=1)
    #print rowrefpix
    # Create polynomial fit for row reference pixels.
    x = range(0,nrow)
    z = numpy.polyfit(x, rowrefpix, 16) # Returns an array of polynom coeff.
    # To get actual values use poly1d with polynomial coefficients from polyfit
    p = numpy.poly1d(z)  

    if doplots == 'y':
        plottitle = 'Reference pixels for first rows bias offset correction'
        ShowMyPlot(x, rowrefpix, x, p(x), plottitle, plotwait)
    # Unfortunately, this polynomial fit is very high order.  It fits very
    # well to first 150 rows bias offset, but then oscillates too much for
    # remainder of rows.  What I really want is to fit an exponential
    # decay to this, but non-linear regression routines are in SciPy, which
    # is not on our system.  
    # So...
    # Use high order polynomial to correct first ~150 rows.  Then use a
    # lower order polynomial to correct all rows (in another function).
    # For numpy.append, can't start with empty array, so use 
    #biascorrimg = chancorrimg[0,:] - p(0)
    biascorrimg = [] 
    for biasrow in range(0,150,1):
        #onerowbiascorr = numpy.array(imgdata[biasrow,:] - p(biasrow))
        onerowbiascorr = imgdata[biasrow,:] - p(biasrow)
        # numpy.append with axis=0 doesn't work -- gives one long line of pix
        # instead of a 2-D array.  ARRGH!
        # This should be fixed by using numpy.array() to force onerowbiascorr to
        # be an actual array and not just a list.  This is [[1,2,3]] vs [1,2,3]
        # However, casting of a list to numpy.array() is very time consuming.
        #biascorrimg = numpy.append(biascorrimg, onerowbiascorr, axis=0)
        biascorrimg.append(onerowbiascorr)
    # Now append the remainder of image without modification.
    # Well... no modification is bad too.  Gives discontinuity.  So, subtract
    # last polynomial value from rest of image.
    #biascorrimg = numpy.append(biascorrimg, chancorrimg[150:nrow,:], axis=0)
    for biasrow in range(150,nrow,1):
        onerowbiascorr = imgdata[biasrow,:] - p(150)
        #onerowbiascorr = numpy.array(imgdata[biasrow,:] - p(150))
        biascorrimg.append(onerowbiascorr)
        #biascorrimg = numpy.append(biascorrimg, onerowbiascorr, axis=0)
    # Wish numpy.append worked, but since it didn't, convert back to numpy array
    npbiascorrimg = numpy.array(biascorrimg)
    return npbiascorrimg

def RowRefCorr(ncol, nrow, imgdata, LeftOrRight):
    # This function does row reference pixel correction
    if LeftOrRight == 'left':
        rowrefpix = numpy.mean(imgdata[0:nrow,0:3], axis=1)
    else : 
        rowrefpix = numpy.mean(imgdata[0:nrow,ncol-4:ncol], axis=1)
    x = range(0,nrow)
    z2 = numpy.polyfit(x, rowrefpix, 8) 
    p2 = numpy.poly1d(z2)
    if doplots == 'y':
        plottitle = 'Reference pixels for row noise correction'
        ShowMyPlot(x, rowrefpix, x, p2(x), plottitle, plotwait)
    # Now, correct for 1/f noise drift using polynomial fit p2.
    rowcorrimg = []
    for row in range(0,nrow,1):
        onerowcorr = imgdata[row,:] - p2(row)
        rowcorrimg.append(onerowcorr)
    return rowcorrimg

# The first plot always comes up and stops the program.  Just show a blank one
# to tell user to close it.
if doplots == 'y':
    matplt.suptitle('Close this first graph to continue')
    matplt.show()
#for imgnum in range(startimgnum,endimgnum+1,1):
# Get FITS file name from input list file, one line at a time.
for imgname in open(inputfilelist, 'r').readlines():
    # Check to see if image name ends in .fits
    if imgname[-6:] == '.fits\n' :
        # It does end in .fits, so strip off the .fits and re-add with suffix.
        outputchannel = imgname[:-6] + '_chancorr.fits'
        outputbias = imgname[:-6] + '_biascorr.fits'
        outputrow = imgname[:-6] + '_rowcorr.fits'
    else : 
        # Does Not end in .fits (strip off \n),  just add suffix.
        outputchannel = imgname[:-1] + '_chancorr.fits'
        outputbias = imgname[:-1] + '_biascorr.fits'
        outputrow = imgname[:-1] + '_rowcorr.fits'   
    #print imgname
    # Open the fits file.  These are defaulting to Float32 which is OK.
    imgfile = pyfits.open(imgname)
    # Get both the header and the data.
    imgdata = imgfile[0].data
    imgheader = imgfile[0].header
    # Get the number of columns and rows in the image.
    ncol = imgfile[0].header['NAXIS1']
    nrow = imgfile[0].header['NAXIS2']
    # Pick the reference pixel correction mode *** DO later, get working now.
    #refpixcorrmode = { 
    #       1: ,
    #       2: , 
    #       3: ,
    #       4: ,
    #       5: ,
    #     }
    #refpixcorrmode.get(refpixmode)()

    chancorrimg = OutputRefCorr(ncol, nrow, numoutputs, imgdata)

    npbiascorrimg = FirstRowsBiasRefCorr(ncol, nrow, chancorrimg, LeftOrRight)

    rowcorrimg = RowRefCorr(ncol, nrow, npbiascorrimg, LeftOrRight)
    #
    # TO DO **** 
    # Add comment to header explaining how image was processed.
    #
    # Now, write the data to the FITS output file
    hdu = pyfits.PrimaryHDU(numpy.array(chancorrimg))
    hdu.writeto(outputchannel)
    hdu = pyfits.PrimaryHDU(npbiascorrimg)
    hdu.writeto(outputbias)
    hdu = pyfits.PrimaryHDU(numpy.array(rowcorrimg))
    hdu.writeto(outputrow)
    imgfile.close()
    
matplt.close()





