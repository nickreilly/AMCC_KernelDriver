""" 
det.py det.fth equivalent module..

The det module supports detector configuration, with persistence.
This is the low level "static" configuration details.
The names of the bias voltages and how to nominally set them (which dacs)
The max number of rows and columns
etc.

It is responsible for the detector files.
it needs to know the detector name and the path to detector file directory

In that directory:
'detname'.map is maps signal names to dac numbers.
'detname'.bias maps names to voltages. (Actually, it does clock rails .)
plain old 'detname' is the file that initializes the other stuff.

the 'run' module does the same thing at a higher level
det stuff does not normally get persisted. run stuff does...
Things that are to be restored in the next session are saved in 'lastrun.run'

The file used by the old system (det.fth) was order dependent.
each line had a specific meaning. extra or missing lines are bad news.

With this package, the file is more tolerant, and line items are
key - value pairs.

notes:
the device driver itself might eventually need to be insmod'ed from here!
(it is the equivalent of the data program.. 
the driver might be enhanced/overhauled over time)

What is a bias voltage, at this level?
well, it is tuple.. of..
a name, a dacnumber, a conversion from dac counts to volts, 
a sanity check for values..
and something else?
"""

__version__ = """$Id: det.py 405 2006-09-15 18:49:11Z drew $ """

__author__ = "$Author: drew $"

__URL__ = "$URL: http://astro.pas.rochester.edu/svn/pydsp/trunk/pydsp/det.py $"

import xdir # this module is so trivial we could easily eliminate it.
import os # used to detect file existence..

# the data dictionary wraps the storage of the detector data
# and the execution of special code when detector data is changed.
from DataDict import DataDict
class DetDict(DataDict):
    """
    Smart dictionary that contains the detector basic parameters.

    Getting and setting values in this dictionary may actually 
    read or write the physical hardware.
    Class is do-nothing, but properties can be added to the class
    on-the-fly outside of the class statement.
    """
    pass

# dd = DetDict() # get a new data dictionary (dd is detector dictionary)
dd = DetDict()
biaslist = []    # ordered list of biases (order taken from bias map file)

from dsp import dspthreadable

# setclockpgm wraps the loading of a new clock program in the DSP
# with the other things that go with it.
import sys
def setclockpgm(clockprogname, raw_input=input, wfile = sys.stdout):
    """
    Resets the DSP and reboots it with new clock program.

    It tries 5 times after which it allows the program to continue even if boot fails.
    """
    import dsp, time

    # Note: checkrunmode was here in original forth.
    fqclockpgm = xdir.clockpath + "/" + clockprogname
    if os.access(fqclockpgm, 4): # can we read it?
        for i in range(5):
            try:
                dsp.load_srec(fqclockpgm) # load it. 
                time.sleep(0.2)
                dsp.amcc.clear_fifo()
                time.sleep(0.2)
                dsp._initheadbits()
                dd.booted = 1
                break
            except SystemError:
                print("boot error %d. retrying." % (i+1), file=wfile)
                continue
        else:
            dd.booted = 0
            print("\n*** unable to boot after 5 tries.\n check power/cables?", file=wfile)
    else:
        print("*** can't locate %s!" % fqclockpgm, file=wfile)

# the DataDict module is probably a bad way to implement this.
# Python supports properties in the new style classes.
# most code outside this uses property-like syntax (dd.vbias)
# rather than dict like syntax (dd["vbias"]) since it is easier to type
# all instances of dd[name] could be replaced with getattr(dd, name)
# and dd[name] = value could change to setattr(dd, name, value)
# all of these items really should be properties of the det dictionary.
# You can only put properties on the base class, not on the instance.
# This means that dd and rd would be instances of separate base classes
# They could probably derive from a common base class.
# properties can be added to the base class programmatically outside
# of the class statement.

def init():
    """
    Initialize the detector smart dictionary. Normally called only once.
    """
    dd.additem("clockpgm", "")#, setfunc=setclockpgm, 
                # docstring="name.ext of clock program object file.")
    dd.additem("detname", "")#, docstring="name of the current detector.")
    dd.additem("datapgm", "")#, docstring="device driver version? unimplemented.")
    dd.additem("outputformat", "block")#, docstring="Video output format: quad, block, interleaved, etc.")
    dd.additem("badpixfile", "")#, docstring="name.ext of bad pixel map. unused.") 
    dd.additem("maxrow", 0)#, docstring="max number of rows in detector. TBD.")
    dd.additem("maxcol", 0)#, docstring="max number of columns in detector. TBD.")
    dd.additem("nout", 0)# docstring="number of outputs of detector.")
    dd.additem("boxname", "")#, docstring="name of array controller electronics")
    dd.additem("adrange", 0.0)#, docstring="A/D converter input voltage range")
    dd.additem("ampgain", 0.0)#, docstring="gain of amp prior to A/D convert")
    dd.additem("temp", 0.0)
    dd.additem("tempB", 0.0)
    dd.additem("prepix", 0)
    dd.additem("postpix", 0)
    dd.additem("booted", 1) # Joe - setting to one initially to see what happens



# we have some special gains.
# two channels are overridden and have lower gain on their dacs.
# this should really be in a config file... and now it is!  Don't use here.
# It is a little complex, because
# one dac can go through more than one gain path...

dac_mv_per_count = {
    #0:3.000 , 1:3.000,
    #12:2.000 , 13:2.000,
    #26:4.000 , 27:4.000,
} # hard coding this is bad... so do not use this here.  
# Instead use the version of the offset and gain that are in the specific
# configuration file in detectors/ directory (e.g. detectors/hawaii-2RG.py).  

dac_mv_offset = {
    #0:-1.000 , 1:-1.000,
    #12:7.000 , 13:5.000,
}

# may want to read from a file? 
# readback will also allow for testing!

maxdac = 8000
mindac = -8000

#class dacwriter:
#    "UNUSED so far. return an object that can be called
#    to set its wrapped dac to some voltage"
#    maxdac = 3300 # base class hold default max
#    mindac = -250 # default min
#    mvpercount = 2.000 # and gain for a writeable voltage
#    import dsp
#    writefunc = staticmethod(dsp.writedac)
#    dacs = ()
#    def __init__(self,dacs,**kwds):
#        self.dacs = dacs
#        self.__dict__.update(kwds) 
#    def writemv(self, millivolts):
#        if self.mindac <= millivolts <= self.maxdac:
#            for dac in self.dacs:
#                countspermv = 1.0/self.mvpercount
#                daccounts = millivolts * countspermv
#                self.writefunc(dac, daccounts)
#        else:
#            raise ValueError

def writeclockdac(val, *dacs):
    """
    Writes val (in millivolts) to clock DACs. 
    DACs is a tuple list of clock DAC numbers.
    
    Checks that the voltage is ok.
    then converts the voltage into DAC counts
    and writes that out.
    """
    import dsp
    if (mindac <= val <= maxdac) or dacs in ( (26, 27), (8, 9) ): 
            # don't check tuples!
        for dacnum in dacs: # for each dac this name changes.
            gain = dac_mv_per_count.get(dacnum , 2.000) # get the gain. 2mv/count default.
            offset = dac_mv_offset.get(dacnum , 0.000) # get the offset. Zero offset default.
            dsp.writedac(dacnum, (val+offset)/gain) # set that voltage. 
            # print dacnum, val, offset, gain
    else:
        raise ValueError("%d exceeds allowable range of %d to %d millivolts"%
                (val, mindac, maxdac))
    
def writebiasdacfunc(*args, **kwds):
    """
    Create (curry) a function to write the bias DAC specified 
    with the proper gain and offset.
    """
    # XXX multiple dacs have multiple gains and offsets!
    # gains and offsets come from system not detector.
    offset = kwds.get("offset", 0.0)
    gain = kwds.get("gain", 2.0)
    _maxdac = kwds.get("maxdac", maxdac)
    _mindac = kwds.get("mindac", mindac)
    dacs = kwds.get("dacs", args)

    def writebiasdac(val):
        """
        Writes val (in millivolts) to the list (tuple) of DACs.

        Checks that the voltage is ok.
        converts the voltage into DAC counts
        writes that out.
        """
        import dsp
        # print "bias",
        if (_mindac <= val <= _maxdac) : 
            for dacnum in dacs: # for each dac this name changes.
                biasval = int((val + offset) / gain)
                dsp.writebias(dacnum, biasval) # set that voltage. 
                # print dacnum, biasval, offset, gain
        else :
            raise ValueError("%d exceeds allowable range of %d to %d millivolts"%
                (val, _mindac, _maxdac))

    return writebiasdac
    
# XXX- about the above two functions..
# apply DIP (dependency inversion principle) here? 
# the functions depends on dsp unnecessarily?
# un-dip: create a default writer that a client would replace with a real writer.
# idea: create null writer in module. Clobber writer with a real writer.
# problem with that idea: what if you wanted simulated and real writers?
# YAGNI? or a default writer would allow simulation?
# XXX- aliases? If *dacs was a list of names, we could delegate
# to those names instead. needed flexibility or unnecessary complication?
# this binds writeclockdac and the det dict -- but that seems sensible.

# more important: The validation of the new dac value is not
# extensible, and the conversion of millivolts to dac counts is
# also hard coded. We would like to set validation for the voltages
# based on the array, and perhaps on the OTHER voltages,
# we would also like to customize the conversion of a voltage
# to a dac count based on the actual hardware in the system.
# the conversion of a valid array voltage to a dac value might
# throw an error- perhaps the hardware can't generate the voltage.
# error handling in these cases should be easy for the user to
# undestand what went wrong and where to go to fix it.

def setbiases(biasrunfile, wfile=sys.stdout, zero=False):
    """
    Loads biasrunfile and sets bias voltages accordingly.

    Sets up the bias rails in the order that it finds them in the file.
    if zero==True, zero all the voltages in the reverse order of the file.
    """
    # a Python source file could be used for persisting values.
    if os.access(biasrunfile, 4):
        print("\n found bias voltages in " + biasrunfile, file=wfile)
        fname = open(biasrunfile)
        lines = fname.readlines()
        if zero : # if we are shutting down, do the file backwards.
            lines.reverse()
        for line in lines :
            tok = line.split() # break into tokens
            ntoks = len(tok) # how many?
            if ntoks == 0: # none?
                continue    # blank lines are cool.
            if tok[0][0] == '#': # comment line? 
                continue    # comments are cool.
            # if ntoks is 1, we silently ignore line. ??
            if ntoks >= 2:
                try:
                    if not zero:
                        dd[tok[0]] = int(float(tok[1]))
                    else :
                        dd[tok[0]] = 0
                    # print "wrote " + tok[0] + ", value %d "%int(tok[1])
                except:
                    import traceback
                    traceback.print_exc()
                    print("error writing ", tok[1], "to", tok[0], file=wfile)
                    raise
    else:
        print("cannot access" + biasrunfile, file=wfile)

dacfuncs = {"CLOCK":writeclockdac, "BIAS": writebiasdacfunc}
dacfuncnames = list(dacfuncs.keys())

def showbiases(wfile=sys.stdout):
    "List the bias names and their corresponding voltages"
    showstrings = []
    for name in biaslist:
        showstrings.append(("%-20s  %5s mV") % (name, dd[name]))
    print('\n'.join(showstrings), file=wfile)
        
def loadbiasmap(biasmapfile, wfile=sys.stdout):
    """Load a biasmapfile, which maps bias names to dac numbers.

    each bias (programmable voltage) has:
    1: a name we refer to it with. (the key)
    2: a tuple of dacs that it is associated with (args for the set function)
    3: a function that it calls to change the voltage.
    4: the current value that it is set to.

    a pseudo-bias (which may move around several biases in a coordinated manner)
    may be possible using this same thing..
    """
    if os.access(biasmapfile, 4):
        print("found biases in " + biasmapfile, file=wfile)
        fname = open(biasmapfile)
        global biaslist
        biaslist = [] # clear out the current list of biases.
        for line in fname.readlines():
            tok = line.split()
            if not tok : # empty list?
                continue    # blank lines are cool.
            if tok[0][0] == '#': # first char is comment?
                continue    # comments are cool.
            # does the first token identify the voltage type?
            # if so, use the specified function. 
            # Otherwise, assume it is a clock.
            dactype = "CLOCK"
            if tok[0] in dacfuncnames :
                dactype = tok[0]
                dacfunc = dacfuncs[tok[0]]
                tok = tok[1:]
            else :
                dacfunc = dacfuncs["CLOCK"]
            # XXX dacfunc might be best as a curried function or a bound 
            # method of a class instance.
            # "curried function": one that is created on the fly. This would allow
            # per voltage validation. A bound method of a class instance might be a
            # better choice, the class could give it access to the other
            # voltages it might need to know about.
            # the question is: how to pull the user customization
            # out into a separate file?

            # anyway, now crank thru the rest of the line.
            # map the bias name to the dac number (or numbers) using a tuple.
            # just loop to end of the line (or to 1st comment)
            # this would allow any number of dacs to change via the same name.
            ntoks = len(tok)
            if ntoks >= 2:
                dac = () # empty tuple
                for i in range(1, ntoks): # for the toks on the line
                    if tok[i][0] == "#": # if the tok is a comment..
                        break # then we are done with the line
                    dac += (int(tok[i]),) # else it is a number
                print(tok[0], dac, file=wfile)
                if dactype == "BIAS" :
                    dd.additem(tok[0], 0, setfunc=writebiasdacfunc(dacs=dac) )
                else:
                    dd.additem(tok[0], 0, setfunc=dacfunc, args=dac)
                biaslist.append(tok[0]) # maintain ordered list of biases.
            else:
                print("error in "+biasmapfile + " :" + line, file=wfile)
        fname.close()
    else:
        print("cant find bias map file", file=wfile)

def get_detname():
    "Return the current detector name"
    return dd.detname

def savedet( wfile=sys.stdout):
    """
    Save the detector information in the "detfile."

    The current det dictionary's detname is used for the name of the file.
    If a file of this name on the proper path can be opened for writing,
    the file is written using the current entries in the detector dictionary.
    """
    fulldetpath = xdir.detpath + "/" + dd["detname"]
    if os.access(xdir.detpath, 6): # need write access to directory.
        detfile = open(fulldetpath, "w")
        for key in ("detname", "clockpgm", "datapgm", "outputformat", "badpixfile"):
            detfile.write( "%s %s\n" % (key, dd[key]))
        for key in ("maxrow", "maxcol", "nout"):
            detfile.write( "%s %d\n" % (key, dd[key]))
        detfile.write("%s %s\n" % ("boxname", dd["boxname"]))
        for key in ("adrange", "ampgain"):
            detfile.write("%s %.3f\n" % (key, dd[key]))
        detfile.flush()
        detfile.close()
    else:
        print("error accessing det file", file=wfile)

def loaddet(detfilename, wfile=sys.stdout):
    """
    Load the detector configuration from the specified detfile.

    First, it loads the biasmap.
    then it opens and reads the detfile itself, assigning the values into
    the det dictionary, which is 'smart' and runs code on some assignments.
    
    The first item in the detfile (after detname) is the clock program name.
    assigning the clock program name into the dictionary actually resets
    the dsp and loads the named clock program
    It silently ignores keys in the detfile that it does not recognize.

    After reading the detfile, loaddet sets the bias voltages.
    """
    fulldetpath = xdir.detpath + "/" + detfilename
    loadbiasmap(fulldetpath+".map") # need to load in the bias map early.
    print("loading detector configuration\n", file=wfile)
    print(fulldetpath+"\n", file=wfile)
    detfile = open(fulldetpath, "r")
    for line in detfile:
        tok = line.split() # split at whitespace into tokens.
        ntoks = len(tok) # how many tokens?
        if ntoks == 0: # none? 
            continue    # ok, skip it.
        if tok[0][0] == "#": # first char is comment?
            continue        # skip this line too.
        if tok[0] in dd: # key is in dictionary? 
            if isinstance(dd[tok[0]], int): #
                dd[tok[0]] = int(tok[1])
            else:
                if isinstance(dd[tok[0]], float):
                    dd[tok[0]] = float(tok[1])
                else:
                    try :
                        dd[tok[0]] = tok[1]
                    except :
                        print(tok[0], tok[1])
    # 
    try :
        # help(__import__) might explain the operation of the next line.
        __import__("detectors.%s"%detfilename, globals(), locals(), [True])
    except :
        __import__("detectors.default", globals(), locals(), [True])
        print("could not import", detfilename)
        import traceback
        traceback.print_exc()
    # set up the biases and clock rails now
    setbiases(fulldetpath+".bias")
    # print >> wfile, "type 'powerup' to turn on biases"
    dd.dirty = True # we don't agree with the disk file.

def powerup(wfile=sys.stdout):
    """
    Turn on all of the biases and clocks.
    use the order specified in detname.bias file
    """
    fulldetpath = xdir.detpath + "/" + dd.detname
    setbiases(fulldetpath+".bias", wfile=wfile)
    dd.dirty = False
    
def powerdown(wfile=sys.stdout):
    """
    Turn off all of the biases and clocks.
    use the order specified in detname.bias file, in reverse.
    """
    dd.vbias = 0 # hack! hardcoded pseudobias name
    dd.vreset = 0 # hack! hardcoded!
    fulldetpath = xdir.detpath + "/" + dd.detname
    setbiases(fulldetpath+".bias", wfile=wfile, zero=True)
    dd.dirty = True
    
