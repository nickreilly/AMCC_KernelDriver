"""
  Program to move the filter wheel when this program finds that a specific file 
  (or sample number) exists in the data directory.
"""

import FilterMove
import filterBase
import filters
import os
import sys
import time

print("\n \n")
print("In order for this program to work, it must be able to access the")
print("parallel port.  That means that PYDSP must not be accessing the")
print("same parallel port.  Be sure to set 'External_FW_Control = True' ")
print("in dsp/pydsp/run.py ")
print("\n Please make certain that the power to the stepper motor is ON. \n")
print("Syntax:")
print("To just move the filter wheel and go to a fixed filter, enter: \n filter('k')")
print("To go to a CVF, enter as tuple (notice extra parens): \n filter(('cvfII',5000)) ")
print("To set this whole program in motion, enter: \n do_me()")

def AskInput():
    global objectdir, filename, triggerfile1, triggerfile2, lightfilter

    print('Enter input directory containing FITS files to be scanned: ')
    print('  For example: /data/H1RG-110/dk_30K_0mV/ \n notice trailing slash')
    objectdir = input()
    #objectdir = '/data/H1RG-110/dk_30K_0mV/'

    print('Enter beginning SUTR file name that you want to watch:')
    print('  For example: dk_30K_0mV.002.  \n notice trailing dot ')
    filename = input()
    #filename = 'dk_30K_0mV.002.'

    print('Enter the sample number that you would like use as a trigger at ')
    print('which this code will move the filter wheel from dark to light:')
    print(' For example: 008   notice leading zeroes ')
    trigger1 = str(input()) 
    triggerfile1 = filename + trigger1
    print('Move filter wheel when file %s exists.'%triggerfile1)

    print('Enter the sample number that you would like use as a trigger at ')
    print('which this code will move the filter wheel from light to dark:')
    print(' For example: 008   notice leading zeroes ')
    trigger2 = str(input()) 
    triggerfile2 = filename + trigger2
    print('Move filter wheel when file %s exists.'%triggerfile2)

    print('Enter filter name that you want to use for the saturation data:')
    lightfilter = str(input())


# Here we redefine "filterBase.set" as "filter".  The original in filterBase.py
# has a dummy "move" so that PYDSP does not move the filter wheel at startup.  
# We use everything from the original except the last two lines in the main 
# "if-else" block.  There we define the move that we want.
def filter(value, nm=None):
    global fwp  # fwp needs to be global so that we only set it once
    try :
        fwp
    except NameError :
        print("\n Enter the current filter wheel position (fwp):\n")
        fwp = float(input())
    print("Current location is %.2f"%fwp)  #%currpos
    # Need to look up the fwp of the requested filter
    if value in filters._fix :
        filtername = value
        bandwidth = filters._fix[value]['bnd']
        wavelength = filters._fix[value]['wvl']
        transmission = filters._fix[value]['trans']
        filtercomment = filters._fix[value]['comment']
        position = filters._fix[value]['pos']
        newpos = position # for updating current position later.
        FilterMove.filterLoc(fwp,position)  # different move than original code.
    else :
        if isinstance(value, tuple):
            nm = float(value[1])
            filtername = value[0]
            if filtername not in list(filters.cvfparam.keys()):
                raise ValueError("can't find that CVF on this filter wheel!")
        elif value not in list(filters.cvfparam.keys()):
            raise ValueError("can't find that on this filter wheel!")
        if nm < 100 : # is request in microns?
            nm *= 1000 # convert to nanometers
        # Find closest FWP (increments of 0.25 step) to requested wavelength.
        initposition = filters.cvfparam[filtername]['start'] + (nm-filters.cvfparam[filtername]['range'][0])*filters.cvfparam[filtername]['slope']
        nearfwp = float(round(initposition*4)/4.0)
        # Find the wavelength at the nearest FWP and round to 1 decimal.
        nearwavelength = round(((nearfwp - filters.cvfparam[filtername]['start'])/filters.cvfparam[filtername]['slope'] + filters.cvfparam[filtername]['range'][0]), 1)
        # Check if the wavelength is on the given CVF
        if (nearwavelength > filters.cvfparam[filtername]['range'][0] and nearwavelength < filters.cvfparam[filtername]['range'][1]) :
            print('Closest FWP to request is ' + str(nearfwp) + ' which corresponds to center wavelength=' + str(nearwavelength))
            wavelength = nearwavelength
            # Calculate the bandwidth using the spectral resolution, and round
            # it to 2 decimal places.  
            # WARNING: CVF BANDWIDTH IS APPROXIMATE - user should do full calc.
            bandwidth = round((wavelength / filters.cvfparam[filtername]['res']), 2)
            # Calculate the transmission (OCLI CFVs only have 3 trans points).
            # Do a linear interpolation between those three trans points.
            cvfshort = filters.cvfparam[filtername]['range'][0]
            cvfmidpoint = (filters.cvfparam[filtername]['range'][1] + filters.cvfparam[filtername]['range'][0])/2
            cvflong = filters.cvfparam[filtername]['range'][1]
            translo = filters.cvfparam[filtername]['trans'][0]
            transmid = filters.cvfparam[filtername]['trans'][1]
            transhi = filters.cvfparam[filtername]['trans'][2] 
            if wavelength < cvfmidpoint :
                transmission = transmid + ((cvfmidpoint - wavelength)/(cvfmidpoint - cvfshort))*(translo - transmid)
            else: 
                transmission = transmid - ((wavelength - cvfmidpoint)/(cvflong - cvfmidpoint))*(transmid - transhi)
            # Set the text string for the filter comment in FITS header
            filtercomment = filters.cvfparam[filtername]['comment']
            # Finally, move to the requested filter.
            #cvFilter = GotoFWP()
            #cvFilter.set(nearfwp)
            newpos = nearfwp # for updating current position later
            FilterMove.filterLoc(fwp,nearfwp) # different move than original code.
        else :
            raise ValueError("That wavelength is not on the given CVF.")
    # Now that we moved, we need to set the current fwp to that value.
    fwp = newpos
    print('Filter = %s'%filtername)
    print('Wavelength = %.1f'%wavelength)
    print('Bandwidth = %.1f'%bandwidth)
    #print 'New FWP = %.2f'%fwp


def CheckSampNum(trig):
    # Yes, this is an infinite loop.
    while True :
        # listdir does not sort by filename or date -- give nearly random ordered list.
        objectfilelist = os.listdir(objectdir)
        #print objectfilelist
        if trig in objectfilelist : 
            print('Found file: %s'%trig)  # now, get us outta here!
            return
        time.sleep(2)  # want to check slowly -- data acquisition takes time.
     
    
def do_me():
    AskInput()
    # Need to access filter() once at start to set the current FWP before 
    # waiting in the CheckSampNum() which could take a long time!
    # Also, don't we want to start in the dark?  Well, this makes sure we do.
    list(filter('cds'))  
    CheckSampNum(triggerfile1)
    list(filter(lightfilter))
    time.sleep(10)
    CheckSampNum(triggerfile2)
    list(filter('cds'))
    print('I am done moving the filter wheel. ')

