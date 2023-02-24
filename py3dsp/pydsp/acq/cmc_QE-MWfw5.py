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
  global rowstart, rowend, colstart, colend, skipL, skipK, subarrayitimeL, subarrayitimeK, fullarrayitime, subarrayitime, skip, naptime, wave_skip
  print 'What is the object name?'
  rd.object = str(raw_input())
  print 'What would you like the comment to be?'
  rd.lc = str(raw_input())

  print 'How many rows in the full image? (Using hardcoded subarrays- input 256!)'
  #rowend = 1024
  rowend = int(raw_input())
  ##rowend = 2048

  #rd.ncolskip = 0  # no reason to skip in column direction
  colstart = 0
  colend = rd.ncol # full row of pixels (assuming user set it to full row)

  print 'What integration time would you like for full array read? (Try 3000)'
  #Try 22000 ms!
  #rd.itime = int(raw_input()) 
  fullarrayitime = int(raw_input())
  ##fullarrayitime = 11000

  print 'How many rows do you want to SKIP for each sub-array in CVF L? (we are using 896)'
  #4 rows is best we can do (maybe)
  # Also, number of rows to read is the same as the number to skip
  skipL = int(raw_input())
  rd.nrowskip = 0
  rowstart = 0

  print 'What integration time would you like for sub-array read in CVF L? (Hard coded)'
  #60 ms is best we can do (maybe).
  #rd.itime = int(raw_input())  # f6or 2 full rows, 10 msec is good.
  subarrayitimeL = int(raw_input())
  ##subarrayitime = 800
  rd.nsamp = 1

  print 'What wavelength step size do you want?  For QE, do 50, for spectra, do 10'
  #60 ms is best we can do (maybe).
  #rd.itime = int(raw_input())  # f6or 2 full rows, 10 msec is good.
  wave_skip = int(raw_input())
  ##subarrayitime = 800

  print 'How long would you like to wait before taking data for temperature stabiliation? (in s)'
  naptime = int(raw_input()) 

def get_mosaic_img():
  global subarrayitime, skip
  # put comment in header
  rd.gc = 'sub-array reads were mosaiced to make full isubarrayitimeLmage'
  rd.itime = subarrayitime
  rd.nrow = skip # Sfullarrayitimeet the number of rows to do a sub-array read.
  for rowskip in range(rowstart, rowend, skip):
    rd.nrowskip = rowskip

    sscan() # Take an image, but do not save
    sscan() # do two because we want a better reset than just once.
    # recreate file name (duplicate code from runrun)
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
  outputfilename = xdir.get_nextobj6filename() + ".fits" # Get the filename
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
    global skip, subarrayitime
    import filterBase
    print 'Stopping CRUN for input entry.  Will restart in a bit'
    burst()
    time.sleep(5)
    burst()
    time.sleep(5)
    burst()
    time.sleep(5)
    askinput()
    time.sleep(22)
    print 'Taking a nap before starting.  Turn off the lights and leave the lab!'
    rd.itime = 3000
    rd.nrowskip = skipL
    rd.nrow = rowend
    crun2()
    time.sleep(naptime)
    burst()
    time.sleep(5)
    burst()
    time.sleep(5)
    burst()
    time.sleep(5)


    # Take some dark images
    print 'Taking Dark Images!'
    filterBase.set("cds")
    rd.itime = fullarrayitime
    rd.nrow = rowend
    rd.nrowskip = skipL

    for blah in range(2):
        srun()

    # Now take some light data with one CVF
    # We go from high to low wavelength so stepper motor doesn't do backlash.
    # For NEOCam arrays go up to 11000nm, and for 13um arrays go up to
    # 14000nm wavelength
    
    subarrayitime=subarrayitimeL #changed to match K for full array read
    skip=skipL #changed to match K for full array read
    rd.nrow = rowend
    rd.nrowskip = skipL
    for wavelen in range(4400, 2400-1, -wave_skip):
        filterBase.set(("cvfL", wavelen))
        if wavelen > 3600:
            rd.itime = 1350
        elif wavelen > 3201:
            rd.itime = 3000
        elif wavelen > 3001:
            rd.itime = 6000
        elif wavelen > 2801:
            rd.itime = 15000
        elif wavelen > 2601:
            rd.itime = 30000
        elif wavelen > 2501:
            rd.itime = 45000
        elif wavelen > 2399:
            rd.itime = 60000


        for zblah in range(2):
            #get_mosaic_img()  # note: itime, nrow set in sub-routine
            srun()
    '''
    for wavelen in range(2700, 1900-1, -50):
        filterBase.set(("cvfK", wavelen))
        if wavelen >= 2601:
            rd.itime = 7000
        elif wavelen > 2501:
            rd.itime = 15000
        elif wavelen > 2401:
            rd.itime = 20000
        elif wavelen > 2301:
            rd.itime = 45000
        elif wavelen > 2201:
            rd.itime = 60000
        elif wavelen > 2101:
            rd.itime = 90000
        elif wavelen > 1901:
            rd.itime = 120000
        elif wavelen > 1801:
            rd.itime = 150000

        for zblah in range(2):
            #get_mosaic_img()  # note: itime, nrow set in sub-routine
            srun()
    '''
    '''
    filterBase.set("3.3")
    subarrayitime=subarrayitimeL
    #skip=skipL
    for blah in range(2):
        #get_mosaic_img()
        rd.itime = 2700
        srun()
    '''

    filterBase.set("cds")
    rd.nrowskip = 0
    rd.nrow = 2048
    rd.nsamp = 1
    rd.itime = 11000
    crun2()

