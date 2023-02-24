from run import rd
import numpy
import xdir
import pyfits
import filterBase
import time
import ls332
import dv
'''
def get_new_mosaic_img(skip, subarrayitime):
    # put comment in header
    orig_itime = rd.itime
    orig_nrow = rd.nrow

    rowstart = 0
    rowend = 2048
    fullrowend = 2048
    rd.gc = 'sub-array reads were mosaiced to make full image'
    rd.itime = subarrayitime
    rd.nrow = skip + 20 # Set the number of rows to do a sub-array read.
    toofar=0
    newrowstart=0
    new_array = numpy.zeros((fullrowend, fullrowend), dtype=int)
    print(range(newrowstart, rowend, skip))
    for i_sub, rowskip in enumerate(numpy.arange(newrowstart, rowend, skip)):
        if i_sub == 0:
            pix_start, pix_end = 0, skip + 10
            sub_start, sub_end = 0, skip + 10
            array_start = 0
        else:
            pix_start, pix_end = rowskip + 10, rowskip + skip + 10
            sub_start, sub_end = 10, skip + 10
            array_start = rowskip - 10
        if pix_end > fullrowend:
            pix_end = fullrowend
            sub_start = 20
            sub_end = skip + 10
        print(pix_start, pix_end, str('Sub:'), sub_start, sub_end)
        if pix_start > fullrowend - 1:
            continue   
        
        temp_row, temp_skip = skip + 20, rd.nrowskip
        

        # Do quick reset to help border pixels
        fast_rows = temp_row + 20
        fast_skip = temp_skip - 10
        rd.ncol = 4
        min_itime = 75
        #print(min_itime)
        rd.itime = min_itime
        pedscan()
        pedscan()
        
        rd.nrow = skip + 20
        rd.itime = subarrayitime
        rd.ncol = 2048

        
        rd.nrowskip = array_start
        rd.nrow = temp_row
        sscan() # Take an image, but do not save
        sscan() # do two because we want a better reset than just once.
        # recreate file name (duplicate code from runrun)
        scanfilename = xdir.get_objpath() + "/" + rd.bufferflag + ".fits"
        scanfile = pyfits.open(scanfilename)
        # These are FITS files. Get both the data and header.
        scandata = scanfile[0].data.astype(int)
        imgheader = scanfile[0].header
        
        print(new_array[pix_start:pix_end, :].shape, scandata[sub_start:sub_end, :].shape)
        
        new_array[pix_start:pix_end, :] = scandata[sub_start:sub_end, :]

        scanfile.close()
    
    image = new_array.astype(int)
    #print(image.shape)


    fitsobj = pyfits.HDUList() # new fitsfile object
    hdu = pyfits.PrimaryHDU()
    fitsobj.append(hdu) # and
    hdu.data = image
    imgheader['NAXIS1'] = 2048
    imgheader['NAXIS2'] = 2048
    fitsobj[0].header = imgheader
    outputfilename = xdir.get_nextobjfilename() + ".fits" # Get the filename
    rd.objnum = xdir.objnum + 1 # increase object number for next filename
    print 'Writing mosaic image to %s'%outputfilename
    fitsobj.writeto(outputfilename)
    
    # Clear comment
    rd.gc = ''

    rd.itime = 11000 #orig_itime
    rd.nrow = 2048 # orig_nrow
    rd.nrowskip = 0

    dv.load_src(outputfilename)
'''

def get_new_mosaic_img_rst(skip, subarrayitime, rowstart, rowend):
    # put comment in header
    orig_itime = rd.itime
    orig_nrow = rd.nrow

    #rowstart = 0
    #rowend = 2048
    fullrowend = 2048
    rd.gc = 'sub-array reads were mosaiced to make full image'
    rd.itime = subarrayitime
    rd.nrow = skip + 20 # Set the number of rows to do a sub-array read.
    toofar=0
    #newrowstart= rowstart #0
    new_array = numpy.zeros((int(rowend-rowstart), rd.ncol), dtype=int)
    #print(range(newrowstart, rowend, skip))
    for i_sub, rowskip in enumerate(numpy.arange(rowstart, rowend, skip)):
        if i_sub == 0:
            pix_start, pix_end = rowskip, skip + 10
            sub_start, sub_end = 0, skip + 10
            array_start = 0
        else:
            pix_start, pix_end = rowskip + 10, rowskip + skip + 10
            sub_start, sub_end = 10, skip + 10
            array_start = rowskip - 10
        if pix_end > fullrowend:
            pix_end = fullrowend
            sub_start = 20
            sub_end = skip + 10
        #print(pix_start, pix_end, str('Sub:'), sub_start, sub_end)
        if pix_start > fullrowend - 1:
            continue   
        
        temp_row, temp_skip = skip + 20, rd.nrowskip
        

        # Do quick reset to help border pixels
        fast_rows = temp_row + 20
        fast_skip = temp_skip - 10
        rd.ncol = 4
        min_itime = 75
        #print(min_itime)
        rd.itime = min_itime
        pedscan()
        pedscan()
        
        rd.nrow = skip + 20
        rd.itime = subarrayitime
        rd.ncol = 2048

        
        rd.nrowskip = array_start
        rd.nrow = temp_row
        sscan() # Take an image, but do not save
        sscan() # do two because we want a better reset than just once.
        # recreate file name (duplicate code from runrun)
        scanfilename = xdir.get_objpath() + "/" + rd.bufferflag + ".fits"
        scanfile = pyfits.open(scanfilename)
        # These are FITS files. Get both the data and header.
        scandata = scanfile[0].data.astype(int)
        imgheader = scanfile[0].header
        
        #print(new_array[pix_start:pix_end, :].shape, scandata[sub_start:sub_end, :].shape)
        
        new_array[pix_start:pix_end, :] = scandata[sub_start:sub_end, :]

        scanfile.close()
    
    image = new_array.astype(int)
    #print(image.shape)


    fitsobj = pyfits.HDUList() # new fitsfile object
    hdu = pyfits.PrimaryHDU()
    fitsobj.append(hdu) # and
    hdu.data = image
    imgheader['NAXIS1'] = 2048
    imgheader['NAXIS2'] = 2048
    fitsobj[0].header = imgheader
    outputfilename = xdir.get_nextobjfilename() + ".fits" # Get the filename
    rd.objnum = xdir.objnum + 1 # increase object number for next filename
    print 'Writing mosaic image to %s'%outputfilename
    fitsobj.writeto(outputfilename)
    
    # Clear comment
    rd.gc = ''

    rd.itime = orig_itime
    rd.nrow =  orig_nrow
    rd.nrowskip = 0

    dv.load_src(outputfilename)
    
    


