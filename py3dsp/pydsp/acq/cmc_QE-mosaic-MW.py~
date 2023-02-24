"""
cmc_QE-mosaic.py - a script to run in pydsp.
takes a few sscans at different nrowskip and ncolskip, and then concatenates
them into a mosaic image.
To use, type: execuser filename
where filename is this file's name, but WITHOUT the .py
"""


from run import rd
import numpy
import xdir
import pyfits
import filterBase


def askinput():
  global rowstart, rowend, colstart, colend, skip, subarrayitime, fullarrayitime
  print 'What is the object name?'
  rd.object = str(raw_input())
  print 'What would you like the comment to be?'
  rd.lc = str(raw_input())

  # For the H1RG, it is easier to read out a complete row (or rows) and
  # then concatenate that. 
  print 'How many rows do you want to read for each sub-array?'
  # Also, number of rows to read is the same as the number to skip
  skip = int(raw_input())
  ##skip = 64
  rd.nrowskip = 0
  rowstart = 0
  print 'How many rows in the full image?'
  #rowend = 1024
  rowend = int(raw_input())
  ##rowend = 2048

  #rd.ncolskip = 0  # no reason to skip in column direction
  colstart = 0
  colend = rd.ncol # full row of pixels (assuming user set it to full row)
  
  print 'What integration time would you like for sub-array read?'
  #rd.itime = int(raw_input())  # for 2 full rows, 10 msec is good.
  subarrayitime = int(raw_input())
  ##subarrayitime = 800
  rd.nsamp = 1

  print 'What integration time would you like for full array read?'
  #rd.itime = int(raw_input()) 
  fullarrayitime = int(raw_input())
  ##fullarrayitime = 11000


def get_mosaic_img():
  # put comment in header
  rd.gc = 'sub-array reads were mosaiced to make full image'
  rd.itime = subarrayitime
  rd.nrow = skip # Set the number of rows to do a sub-array read.
  for rowskip in range(rowstart, rowend, skip):
    rd.nrowskip = rowskip

    sscan() # Take an image, but do not save
    sscan() # do two because we want a better reset than just once.
    # recreate file name (duplicateWhat integration time would you like for full array read?
    scanfilename = xdir.get_objpath() + "/" + rd.bufferflag + ".fits"
    scanfile = pyfits.open(scanfilename)
    # These are FITS files. Get both the data and header.
    scandata = scanfile[0].data
    imgheader = scanfile[0].header

    if rd.nrowskip == rowstart:
        newrowblock = scandata # can't concat on empty.
    else:
        oldrowblock = newrowblock
        newrowblock = numpy.concatenate((oldrowblock, scandata), axis=0)
    scanfile.close()
  image = newrowblock

  fitsobj = pyfits.HDUList() # new fitsfile object
  hdu = pyfits.PrimaryHDU()
  fitsobj.append(hdu) # and
  hdu.data = image
  fitsobj[0].header = imgheader
  outputfilename = xdir.get_nextobjfilename() + ".fits" # Get the filename
  rd.objnum = xdir.objnum + 1 # increase object number for next filename
  print 'Writing mosaic image to \n %s'%outputfilename
  fitsobj.writeto(outputfilename)
  #pyfits.writeto(outputfilename, newrowblock)
  # Clear comment
  rd.gc = ''

  rd.itime = fullarrayitime
  rd.nrow = rowend
  rd.nrowskip = 0

def getQE():
    time.sleep(5)
    burst()
    time.sleep(5)
    burst()
    time.sleep(5)

    import filterBase
    askinput()
    rd.nrow = skip
    crun()
    print "REMOVE THE SLEEP BEFORE YOU START!"
    
    time.sleep(14400)
    burst()
    time.sleep(5)
    burst()
    time.sleep(5)
    burst()
    time.sleep(5)


    # Take some dark images
    filterBase.set("cds")
    rd.itime = fullarrayitime
    rd.nrow = rowend
    rd.nrowskip = 0

    for blah in range(2):
        srun()
    '''
    # Now take some light data with one CVF
    # We go from high to low wavelength so stepper motor doesn't do backlash.
    # For NEOCam arrays go up to 11000nm, and for 13um arrays go up to
    # 14000nm wavelength
    for wavelen in range(12000, 7899, -100):
        filterBase.set(("cvfIII", wavelen))
        for zblah in range(2):
            get_mosaic_img()  # note: itime, nrow set in sub-routine

    '''
    '''
    # And some more with the other CVF
    for wavelen in range(7000, 4299, -100):
        filterBase.set(("cvfII", wavelen))
        for blah in range(2):
            ##srun()
            get_mosaic_img()  # note: itime, nrow set in sub-routine
    '''

    for wavelen in range(4400, 2400-1, -50):
        filterBase.set(("cvfL", wavelen))
        for blah in range(2):
            ##srun()
            get_mosaic_img()  # note: itime, nrow set in sub-routine

    

    '''
    # Take a data point with 7.1 micron filter - this is well calibrated and
    # will be used to anchor the other QE data points. 
    filterBase.set("7.1")
    for blah in range(4):
        get_mosaic_img()

    # Full images commented for 10 um because saturated
    #for blah in range(4):
    #    srun()
   
    # Take a data point with 8.8 micron filter. 
    filterBase.set("8.8")
    for blah in range(4):
        get_mosaic_img()

    # Full images commented for 10 um because saturated
    #for blah in range(4):
    #    srun()

    
    # Take a data point with m' filter. 
    filterBase.set("m'")
    for blah in range(4):
        get_mosaic_img()

    rd.itime = 180000
    for blah in range(4):
        srun()
    rd.itime = fullarrayitime
    

    # Take a data point with l' filter. 
    filterBase.set("l'")
    for blah in range(4):
        get_mosaic_img()

    for blah in range(4):
        srun()

    # Take a data point with 3.3 filter. only taking full array data since we get barely any flux with this filter, so itime has to be extra long
    filterBase.set("3.3")
    rd.itime = 60000
    for blah in range(4):
        srun()
    rd.itime = fullarrayitime

    # Take a data point with 5.8 filter. Only mosaic as full array saturates
    filterBase.set("5.8")
    for blah in range(4):
        get_mosaic_img()

    # Take a data point with 8.6 filter. Only mosaic as full array saturates
    filterBase.set("8.6")
    for blah in range(4):
        get_mosaic_img()
   '''
    # put array back in dark
    filterBase.set("cds") 
    rd.lc = ''
    sscan()
  
    crun()

