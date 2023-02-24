"""
cmc_QE.py - a script to run in pydsp.
takes a few images at different wavelengths.
To use, type: execuser filename
where filename is this file's name, but WITHOUT the .py
"""


from run import rd
import numpy
import xdir
import pyfits
import filterBase

def askinput():
  global rowstart, rowend, numrows, rowskip, colstart, colend, numcols, colskip, subarrayitime, fullarrayitime
  print 'What is the object name?'
  rd.object = str(raw_input())
  ##rd.object = 'QE40K'
  #print 'What is the room temperature at black clothe (in celsius)?'
  #roomTemp = str(raw_input()) 
  #rd.lc = 'Black cloth room T = ' + roomTemp + 'C' 
  print 'What would you like the comment to be?'
  rd.lc = str(raw_input())
  ##rd.lc = 'Looking at black cloth, T = 18.15 C'

  # For the H1RG, it is easier to read out a complete row (or rows) and
  # then concatenate that. 
  print 'How many rows do you want to read for each sub-array?'
  numrows = int(raw_input())
  print 'How many rows do you want to skip?'
  rowskip = int(raw_input())
  ##rowskip = 64
  
  print 'How many columns do you want to read for each sub-array?'
  numcols = int(raw_input())
  print 'How many columns do you want to skip?'
  colskip = int(raw_input())
  ##colskip = 64
  
  print 'What integration time would you like for sub-array read?'
  #rd.itime = int(raw_input())  # for 2 full rows, 10 msec is good.
  subarrayitime = int(raw_input())
  ##subarrayitime = 800

  rowstart = 0
  colstart = 0
  print 'How many rows and cols in the full image?'
  #rowend = 1024
  rowend = int(raw_input())
  colend = rowend
  ##rowend = 2048

  print 'What integration time would you like for full array read?'
  #rd.itime = int(raw_input()) 
  fullarrayitime = int(raw_input())
  ##fullarrayitime = 11000

def SetFullArraySize():
  rd.nsamp = 1
  rd.itime = fullarrayitime
  rd.nrow = rowend
  rd.nrowskip = rowstart
  rd.ncol = colend
  rd.ncolskip = colskip

def SetSubArraySize():
  rd.nsamp = 1
  rd.itime = subarrayitime
  rd.nrow = numrows
  rd.nrowskip = rowskip
  rd.ncol = numcols
  rd.ncolskip = colskip

def getQE-MW():
    askinput()

    time.sleep(5)
    burst()
    time.sleep(5)
    burst()
    time.sleep(5)


    # Take some dark images
    filterBase.set("cds")
    SetFullArraySize()
    for blah in range(4):
        srun()
    SetSubArraySize()
    for blah in range(4):
        srun()

    # Now take some light data with one CVF
    # We go from high to low wavelength so stepper motor doesn't do backlash.
    # For NEOCam arrays go up to 11000nm, and for 13um arrays go up to
    # 14000nm wavelength
    SetSubArraySize() # note: itime, nrow set in sub-routine
    ##rd.itime = 500    # so reset itime here
    for wavelen in range(4400, 2699, -100):
        filterBase.set(("cvfL", wavelen))
        for zblah in range(4):
            srun()  

    
    # And some more with the other CVF
    SetSubArraySize()
    rd.itime = 1500 # uncomment to manually change itime (it is set in setsubarraysize()
    for wavelen in range(2600, 1999, -100):
        filterBase.set(("cvfK", wavelen))
        for blah in range(4):
            srun()

    # And some more with the other CVF
    SetSubArraySize()
    rd.itime = 1500 # uncomment to manually change itime (it is set in setsubarraysize()
    for wavelen in range(1900, 1299, -100):
        filterBase.set(("cvfJH", wavelen))
        for blah in range(4):
            srun()


   
    # Take a data point with j micron filter. 
    SetSubArraySize()
    rd.itime = 500
    filterBase.set("j")
    for blah in range(4):
        srun()

    # Take a data point with h filter. 
    SetSubArraySize()
    rd.itime = 120000
    filterBase.set("h")
    for blah in range(4):
        srun()

    # Take a data point with k micron filter. 
    SetSubArraySize()
    rd.itime = 233
    filterBase.set("k")
    for blah in range(4):
        srun()

    # Take a data point with 3.3 micron filter. 
    SetSubArraySize()
    rd.itime = 233
    filterBase.set("3.3")
    for blah in range(4):
        srun()

    # Take a data point with l' micron filter. 
    SetSubArraySize()
    rd.itime = 233
    filterBase.set("l'")
    for blah in range(4):
        srun()

    # Take a data point with m' micron filter. 
    SetSubArraySize()
    rd.itime = 233
    filterBase.set("m'")
    for blah in range(4):
        srun()

    # put array back in dark
    filterBase.set("cds") 
    SetFullArraySize()
    rd.lc = ''
    sscan()
  
    crun()


  

