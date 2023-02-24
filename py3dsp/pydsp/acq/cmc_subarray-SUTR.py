"""
cmc_subarray-SUTR.py - a script to run in pydsp.
takes a few sscans at different nrowskip and ncolskip
To use, type: execuser filename
where filename is this file's name, but WITHOUT the .py
"""


from run import rd
import numpy
import xdir
import pyfits



def askinput():
  global rowstart, rowend, colstart, colend, skip, subarrayitime, numrows
  print 'What is the object name?'
  rd.object = str(raw_input())

  # For the H1RG, it is easier to read out a complete row (or rows) and
  # then concatenate that. 
  print 'How many rows do you want to read for each sub-array?'
  numrows = int(raw_input())
  print 'How many rows do you want to skip for the next sub-array read?'
  skip = int(raw_input())
  rd.nrowskip = 0
  rowstart = 0
  print 'How many rows in the full image?'
  rowend = int(raw_input())

  #rd.ncolskip = 0  # no reason to skip in column direction
  colstart = 0
  colend = rd.ncol # full row of pixels (assuming user set it to full row)
  
  print 'What integration time would you like for sub-array read?'
  rd.itime = int(raw_input())  
  # Note: for 2 full rows, 10 msec is good.
  #subarrayitime = int(raw_input())
  ##subarrayitime = 800
  print 'How many samples do you want for the SUTR?'
  rd.nsamp = int(raw_input())


def get_subarrays():

    time.sleep(5)
    burst()
    time.sleep(5)
    burst()
    time.sleep(5)

    old_itime = rd.itime
    old_nrow = rd.nrow
    old_ncol = rd.ncol
    old_nsamp = rd.nsamp

    askinput()
    
    rd.nrow = numrows # set the number of rows for the sub-array read.

    for cycle in range(0, 15, 1):
        for rowskip in range(rowstart, rowend, skip):
            rd.nrowskip = rowskip 
            sutr()

    # put parameters back to usual and then crun.
    rd.itime = old_itime
    rd.nrow = old_nrow
    rd.nrowskip = 0
    rd.ncol = old_ncol
    rd.ncolskip = 0
    rd.nsamp = old_nsamp
    # put array back in dark
    filterBase.set("cds") 
    rd.lc = ''
    sscan()
  
    crun()

