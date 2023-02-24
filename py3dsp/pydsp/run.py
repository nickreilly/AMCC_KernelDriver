"""run.py. Handles the run dictionary-- 

This smart dictionary contains the things that are persisted and restored.
when the system is shut down and restarted.

it is hard to say what is "detector" and what is "last run"
some things might be shared between them.
bias voltages?? clock rails?
detector might have defaults 

This seems to go against the single responsibility principle.
"""

__version__ = """$Id: run.py 405 2006-09-15 18:49:11Z drew $ """

__author__ = "$Author: drew $"

__URL__ = "$URL: http://astro.pas.rochester.edu/svn/pydsp/trunk/pydsp/run.py $"

import os # for basic access to the run files.
import sys
import xdir # paths from environment. Mostly used for finding last.run
import det # we call det to load detector file


try:
    from detkeys import detkeys
except ImportError:
    detkeys = []
    print("no detkeys.py")

#detkeys = []

from DataDict import DataDict

class RunDict(DataDict) :
    pass

rd = RunDict() # create the run dictionary, an instance of DataDict class.

# Make sure the filter wheel is on the path
# fw_path = os.getcwd() + '/filter_wheel'
pydsp_env = os.environ["PYDSPHOME"]
fw_path = f'{pydsp_env}/pydsp/filter_wheel'
if fw_path not in sys.path:
    sys.path.append(fw_path)


# Set this to True if you want to run the filter wheel from an independent 
# program.  Otherwise, this should be set to False so that PYDSP can 
# move the filter wheel.
External_FW_Control = False

if not External_FW_Control:
    import FilterMove  # PYDSP should run Filter Wheel
import filterBase
def fwpUpdate(newpos):
    if not External_FW_Control:
        rd.fwp=FilterMove.filterLoc(rd.fwp,newpos)
    rd.bandwidth=filterBase.get_bandwidth()
    rd.wavelength=filterBase.get_wavelength()
    rd.transmission=filterBase.get_transmission()
    rd.fwn=filterBase.filters.get_wheelname()
    rd.filtercomment=filterBase.get_filtercomment()
    rd.filter

def init(wfile=sys.stdout):
    import dsp # we might want to access dsp?
            # except the dsp clock program might not be running
            # when we do a bunch of these things.
            # thread safe?? 

    rd.additem("nrow", 0, setfunc=dsp.senddsp, args=(16,), 
            docstring = "number of rows")
    rd.additem("ncol", 0, setfunc=dsp.senddsp, args=(17,), 
            docstring = "number of columns")
    rd.additem("nrowskip", 0, setfunc=dsp.senddsp, args=(23,), 
            docstring = "number of rows to skip") # 
    rd.additem("ncolskip", 0, setfunc=dsp.senddsp, args=(24,), 
            docstring = "number of columns to skip") # 
    rd.additem("nta", 0, docstring = "number of throwaways. legacy/unused.")
    rd.additem("ncd", 1, docstring = "number of coadds. legacy/unused") 
    rd.additem("itime", 1000, setfunc=dsp.senddsp, args=(19,), 
            docstring="integration time in milliseconds") # 
    rd.additem("ftime", "", docstring = "first time (time before clocking starts)") # 
    rd.additem("ltime", "", docstring = "last time (time after clocking finishes)") # 
    rd.additem("nsamp", 1, setfunc=dsp.senddsp, args=(18,), 
            docstring="number of samples")
    rd.additem("ctstime", 0, setfunc=dsp.senddsp, args=(30,), 
            docstring = "clamp to sample time.")
    rd.additem("adctime", 0, docstring = "conversion time, inactive.") #
    rd.additem("sampmode", "", docstring = "sample mode, inactive") 
    rd.additem("runflag", 0) # 
    rd.additem("biasv", 0.0, "readback of selected bias voltage")
    rd.additem("biasi", 0.0, "readback of selected bias current")
    rd.additem("clockv", 0.0, "readback of selected clock voltage")
    rd.additem("clocki", 0.0, "readback of selected clock current")

    # IMAGE BUFFERS ETC.
    rd.additem("bufferflag", "src")# are we in source or background?
    rd.additem("srcflag", 0) # is source buffer saved?
    rd.additem("bkgflag", 0) # is background buffer saved?
    
    # FILTER WHEEL
    if not External_FW_Control:
        import FilterMove
    import filterBase
    rd.additem("fwp", 0.0, getfunc = None, setfunc=None,
            docstring = "filter dial position. DOES NOT MOVE FILTER")
    rd.additem("fw", 0.0, getfunc = lambda:rd.fwp, setfunc=fwpUpdate,
            docstring = "filter dial position.")
    rd.additem("fwn", "filterI", getfunc = filterBase.filters.get_wheelname,
            docstring="filter wheel name")
    rd.additem("filter", val="cds", setfunc=filterBase.set, getfunc=filterBase.get_filtername, docstring = "filter name")
    rd.additem("wavelength", 500.0, getfunc = filterBase.get_wavelength,
            docstring = "wavelength, nanometers")
    rd.additem("bandwidth", 500.0, getfunc = filterBase.get_bandwidth,
            docstring = "bandwidth, nanometers")
    rd.additem("transmission", -1.0, getfunc = filterBase.get_transmission,
            docstring = "filter transmission in percent")
    rd.additem("filtercomment", "filtr", getfunc = filterBase.get_filtercomment,
            docstring = "filter description")
    rd.additem("lyotstop", 0.0, getfunc = filterBase.filters.get_lyotstop,
            docstring = "lyot stop, cold aperture in millimeters")
    rd.additem("dist2lyot", 0.0, getfunc = filterBase.filters.get_dist2lyot,
            docstring = "distance in mm between lyot stop and detector")
    rd.additem("pre_temp", 0.0, docstring="temp for ped or start of frame.")
    rd.additem("post_temp", 0.0, docstring="temp for sig or end of frame")
    rd.additem("tempB", 0.0, docstring="temp for sig or end of frame")
    rd.additem("userprog", "")
    rd.additem("telescope", "lab", docstring = "name of observing location")
    rd.additem("observer", "moore", docstring = "name of observer")
    #rd.additem("ad0", 0)
    #rd.additem("ad1", 0)
    #rd.additem("ad2", 0)
    #rd.additem("ad3", 0)
    #rd.additem("ad0name", "ad0")
    #rd.additem("ad1name", "ad1")
    #rd.additem("ad2name", "ad2")
    #rd.additem("ad3name", "ad3")
    rd.additem("gc", "", docstring = "global comment. stays around till you change it.")
    rd.additem("lc", "", docstring = "local comment. cleared after each use.")
    rd.additem("clockpath", xdir.clockpath, 
            setfunc = xdir.setclockpath , getfunc = lambda: xdir.clockpath)
    rd.additem("detpath", xdir.detpath, setfunc=xdir.setdetpath, 
            getfunc = lambda: xdir.detpath)
    rd.additem("datapath", xdir.datapath, setfunc=xdir.setdatapath , 
            getfunc = lambda: xdir.datapath )
    rd.additem("night", xdir.nightname, setfunc=xdir.set_night, 
            getfunc = lambda: xdir.nightname)
    rd.additem("object", xdir.objname, setfunc = xdir.set_object, 
            getfunc = lambda: xdir.objname)
    rd.additem("objnum", xdir.objnum, 
            getfunc = lambda: xdir.objnum)
    rd.additem("objfile", "", getfunc = xdir.get_nextobjfilename)
    # rd.additem("detname", "", setfunc = det.loaddet, 
    #         getfunc = det.get_detname) # !! not xdir??
    rd.additem("detname", xdir.get_detname())
    # return rd
    
    

    
def asksavedet(wfile=sys.stdout):
    "XXX check if the detector configuration has changed."
    if det.dd.dirty:
        print("Detector configuration has changed. Save it (y/n)? ", file=wfile)

def savesetup(filename="lastrun.run", wfile=sys.stdout):
    """
    Saves the current configuration to filename. Default is "lastrun.run."

    It uses the detpath from xdir, saves the rundict stuff and also saves 
    the detector file. 

    NOTE: Does NOT save bias and clock voltages, except for a few exceptions.
    """
    keys_to_save = [
        'detpath', 'clockpath', 'detname', 'nrow', 'ncol', 'nrowskip', 'ncolskip',
        'nta', 'ncd', 'itime', 'nsamp', 'ctstime', 'adctime', 'sampmode', 'runflag',
        'bufferflag', 'fwp', 'wavelength', 'bandwidth', 'fwn', 'filter', 'telescope', 
        'observer', 'gc', 'lc', 
    ]
    if not det.dd:
        print("not saving setup!", file=wfile)
        return
    print("saving setup in " + filename, file=wfile)
    fqfilename = xdir.detpath + '/' + filename
    with open(fqfilename, 'w') as f:
        for key in keys_to_save:
            try:
                f.write(f'{key} {getattr(rd, key)}\n')
            except:
                f.write(f'{key}  \n')
                
        for key in detkeys:
            try:
                f.write(f'{key} {str(det.dd[key])}\n')
            except:
                f.write(f'{key}  \n')
                

        f.write('datapath ' + xdir.datapath+'\n')
        f.write('night ' + xdir.nightname +'\n') # strip off data path!??
        f.write('object ' + xdir.objname +'\n') # strip off night path!

#     if os.access(xdir.detpath, 6): # if we can write in the det directory
#         fhand = open(fqfilename, "w")

#         fhand.write('detpath ' + xdir.detpath+'\n') # save detpath first
#         fhand.write('clockpath ' + xdir.clockpath+'\n') # save clockpath too.
#         fhand.write('detname ' + det.dd["detname"]+'\n') # save detname next.

#         # loop over list of keys..
#         for key in ["nrow", "ncol", "nrowskip", "ncolskip",
#                     "nta", "ncd", "itime", "nsamp", 
#                     "ctstime", "adctime", "sampmode", "runflag" ]:
#             fhand.write(key + ' ' + str(rd[key])+'\n')

#         # IMAGE BUFFERS ETC.
#         for key in ["bufferflag", "fwp", "wavelength", "bandwidth"]:
#             fhand.write(key + ' ' + str(rd[key])+'\n')

#         # next, save strings.
#         for key in ["fwn", "filter", "telescope", "observer" ]:
#             fhand.write(key + ' ' + str(rd[key])+'\n')

#         # save global and local comments inside quotes.
#         for key in ["gc", "lc"]:
#             fhand.write(key + ' "' + str(rd[key])+'"\n')

#         # save the voltage overrides in detkeys.
#         for key in detkeys:  
#             fhand.write(key + ' ' + str(det.dd[key])+'\n')

#         #fhand.write('dataacqpath ' + xdir.dataacqpath+'\n')
#         fhand.write('datapath ' + xdir.datapath+'\n')
#         fhand.write('night ' + xdir.nightname +'\n') # strip off data path!??
#         fhand.write('object ' + xdir.objname +'\n') # strip off night path!
#         # fhand.write('objfile ' + xdir.get_objfile+'\n') # not needed! will auto seek.
#         fhand.flush()
#         fhand.close()
#         # det.savedet()

# another approach:
# if lastrun.run exists, we can back it up,
# and then use it to format the new file.
# retain blank lines and comments,
# just replace the values for each key in the file.
# if the dicionary entry is a function type, 
# we want to call the function's set method to initialize it.
# or call the function's get method to save it.

def setup(recover_lro, wfile=sys.stdout):
    """Recover system state from rundata file.

    using detpath, load in rundata file (usually lastrun.run)
    loop through this file and parse lines into key/value pairs.
    check the run dict for the key and assign there.
    if the key is in 'detkeys' then it is a special
    det file value we save and restore as an exception.
    """ 
    global rd
    from util_funcs import LastRunObj, save_info_to_file
    from dataclasses import fields
    #     init()
    lro = LastRunObj(emptylastrun=recover_lro)
    lro.set_actual_lro()
    atts_found = [field.name for field in fields(lro)]
    for i_at, temp_att in enumerate(atts_found):
        if temp_att in rd:
            try:
                if getattr(lro, temp_att) == 0 or getattr(lro, temp_att) == '':
                        continue
                temp_val = str(getattr(lro, temp_att))
                if str.isnumeric(temp_val):
                        setattr(rd, temp_att, int(getattr(lro, temp_att)))
                else:
                        setattr(rd, temp_att, (getattr(lro, temp_att)))
                print(f"restored {temp_att} to {getattr(lro, temp_att)}")
            except:
                print(f'{temp_att} NOT working in run.py recover')
    rd.objnum = xdir.objnum
#     print(rd)
#     save_info_to_file(rd, lro)


