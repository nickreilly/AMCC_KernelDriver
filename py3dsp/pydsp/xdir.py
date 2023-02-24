"""
xdir.py is a translation of xdir.fth. 

It keeps track of the paths to various system-related files.
It seems trivial enough that most of its functionality
should probably go into start_pydsp.py and run.py
"""

__version__ = """$Id: xdir.py 405 2006-09-15 18:49:11Z drew $ """

__author__ = "$Author: drew $"

__URL__ = "$URL: http://astro.pas.rochester.edu/svn/pydsp/trunk/pydsp/xdir.py $"

import os
import sys

# handle datapath, night, object, and object number.
# How about having xdir depend upon run.py??
# can run depend upon xdir too??
# (this is not a package, so maybe its ok.)

datapath = ""
detname = ''
nightname = "" # the name of the night. observes changes in data path.
objname = "" # object name. Observes changes in night.
objnum = 0 # Observes changes in object name.
            # note, if the night changes but the object name does not..
            # well, the object really does change, so the object number
            # is dirty and needs to check itself anyway.
try :
    dsphome = os.environ["PYDSPHOME"]
except KeyError :
    print("PYDSPHOME environment variable is not set.")
    ans = input("Set PYDSPHOME to default? (path/to/parent/folder/dsp) [y]")
    if ans.lower() in ("", "y", "yes"):
        os.environ["PYDSPHOME"] = os.getcwd().replace("/pydsp", "")
        print("set PYDSPHOME as " + os.environ["PYDSPHOME"])
        dsphome = os.environ["PYDSPHOME"]
    else:
        print("please set it to the full path to the folder")
        print("that contains the pydsp, 56300, det, and ociwpci folders")
        print("e.g. PYDSPHOME=/home/dsp/dsp")
        sys.exit()

try :
    detpath = os.environ["DETPATH"]
except KeyError :
    detpath = f'{dsphome}/det'

## 
try :
    clockpath = os.environ["CLOCKPATH"]
except KeyError :
    clockpath = f'{dsphome}/56300'

try :
    driverpath = os.environ["OCIWPCIPATH"]
except KeyError :
    driverpath = f'{dsphome}/ociwpci'

#try:
#    datapath = os.environ["DATAPATH"] # where to store the fits files.
#except:
datapath = "/data"

def paths() :
    "prints out the det, data, and clock paths."
    print("detpath : " + detpath)
    print("datapath : " + datapath)
    print("clockpath : " + clockpath)

def noobject() :
    "clear out the current object name and number"
    global objname
    global objnum 
    objname = ""
    objnum = 0

def nonight() :
    "clear out the current night name, also clear the current object"
    global nightname 
    nightname = ""
    noobject()

def get_nightpath() :
    "Return the full path to the current night directory, or None"
    if datapath and nightname :
        return '/'.join([datapath,nightname])
    raise RuntimeError("must set night name and/or data path")

def get_objpath() :
    "Return the full path to the current object directory"
    if get_nightpath() and objname :
        return '/'.join([get_nightpath(),objname])
    raise RuntimeError("must set object name")

def get_objfilename(num = None) :
    "Return the full path to the object file number num"
    if num == None :
        num = objnum
    if get_objpath() :
        if num :
            return '/'.join([get_objpath(),objname])+('_%03d'%num)
        else :
            return ''
    raise RuntimeError("no path to object")

def get_nextobjfilename() :
    """return the name of the next file we can write to.
    leave objnum at the last file that was written, or zero if
    none have been written yet. objnum is always less than or equal to
    the number of objects in the directory. """
    global objnum 
    if not objnum :
        objnum = 1
    while os.access(get_objfilename()+".fits",4) : 
        objnum += 1
    runfile = get_objfilename()
    objnum -= 1
    return runfile

def get_detname():
    global detname
    '''Load the detname so we dont have to rely on det.py'''
    temp_file = f'{os.environ["PYDSPHOME"]}/det/lastrun.run'
    with open(temp_file, mode='r') as temp_load:
        for line in temp_load.readlines():
            if line.split(' ')[0] == 'detname':
                detname = line.split(' ')[1].split('\n')[0]
    return detname


  
# FILES AND DIRECTORIES
# data path changes very rarely..
# create the directory outside of the pysys envoronment.

def setclockpath(newclockpath) :
    # check if the new clockpath exists
    global clockpath
    if os.access(newclockpath, 6) :
        clockpath=newclockpath  # 
    else :
        print(newclockpath + " has access problem")

def setdatapath(newdatapath) :
    """set a new data path. 
    if the path is a change to the current one,
    clear out the night and object."""
    global datapath
    # check if the new datapath exists
    if os.access(newdatapath, 6) :
        if datapath != newdatapath :
            datapath = newdatapath  # 
    else :
        print(newdatapath + " has access problem")

def setdetpath(newdetpath) :
    ""
    global detpath
    if (not newdetpath) or (detpath == newdetpath) :
        return
    if os.access(newdetpath, 4) : # can we read it?
        detpath=newdetpath  # 
        print("detpath changed. specify new night and object")
    else :
        print(newdetpath + " has access problem")

def set_night(newnightname) :
    """Create a new night diretory under the datapath
    we allow setting to a previously existing night..
    although that may be strange outside the lab.
    (in the lab, a "night" is usually a particular detector under test.
    """
    global nightname
    oldnightname = nightname # XXX use this for error recovery?
    if oldnightname != newnightname :
        nightname = newnightname
        # EAFP approach??
        if not os.access(get_nightpath(),4) : # does not exist?
            os.mkdir(get_nightpath()) # make new night dir
        noobject() # clear out the current object
        os.chdir(get_nightpath()) # cd into it

def set_object(newobjname) : 
    """Set the name of the current object.
    if it does not exist, make it exist.
    if it does exist, cd to it and advance the number to
    the first unused used object number."""

    global objname
    global objnum
    if(os.access(get_nightpath(),6)) : # if we can write in nightpath
        objname = newobjname # set the object name
        objnum = 0 # and initialize the number 
        if not os.access(get_objpath(),4) : # is it not there already?
            os.mkdir(get_objpath()) # then make it!
        else :
            get_nextobjfilename()
        print("%d objects in directory" % objnum)
        if objnum : 
            print("current file : " + get_objfilename())
        os.chdir(get_objpath()) # change directory into it..
    else :
        print(get_nightpath() + " access error!")
