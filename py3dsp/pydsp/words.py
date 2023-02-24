"""
General user-level words for pydsp
"""

__version__ = """$Id: words.py 400 2006-06-19 22:39:30Z drew $"""

__author__ = "$Author: drew $"

__URL__ = "$URL: http://astro.pas.rochester.edu/svn/pydsp/trunk/pydsp/words.py $"

raise ImportError

import sys

def help(thing = None, input=input,  wfile=sys.stdout) :
    """wrapper for pydoc's help function.

    checks input. If it is a string, check all the likely
    places for a match: the run dict, the det dict, and the
    globals. If it is not a string, just pass it to pydoc.
    """
    from run import rd
    from det import dd
    import pydoc
    if not thing : # user just typed "help[cr]. give extra help."
        helpstring = 'Type "help [name]" to get help on a specific word.\n' \
        '"words" will tell you a whole bunch of words you can use\n' \
        '"see" will try to dig up source code for an object\n' \
        '"break" will go from ur_ok prompt to >>> prompt)\n' \
        '"cloop()" will go from >>> prompt to ur_ok prompt\n' \
        'The >>> prompt is the python interpreter prompt.\n' \
        'The ur_ok prompt is the command interpreter.\n' \
        'The command interpreter IS the python function cloop()\n' \
        'Lines containing parentheses () or semicolons ; are\n' \
        'treated as Python by the command interpreter.'
        print(helpstring, file=wfile)
    elif isinstance(thing,str) : # is it a word? try to look it up.
        helpstrings = []
        docstring = None
        if thing in list(rd.keys()) : 
            dictobj, dictname = rd, "run"
        elif thing in list(dd.keys()) : 
            dictobj, dictname = dd, "det"
        else :
            dictobj, dictname = None, ""
        if dictname :
            helpstrings.append("%s is a %s dictionary word.\n"%(thing, dictname))
            docstring = dictobj.docstring[thing]
            if docstring : 
                helpstrings.append(docstring+'\n')
            try :
                getfunc = dictobj.getfunc[thing]
                if dictobj.getfunc[thing] :
                    helpstrings.append("it has a getfunc, "+repr(getfunc) + '\n')
                    if getfunc.__doc__ :
                        helpstrings.append(getfunc.__doc__+'\n')
            except :
                pass
            try :
                setfunc= dictobj.setfunc[thing]
                if setfunc :
                    helpstrings.append("it has a setfunc, "+repr(setfunc) + '\n')
                    if setfunc.__doc__ :
                        helpstrings.append(setfunc.__doc__+'\n')
            except :
                pass
            try :
                args = dictobj.args[thing]
                if args :
                    helpstrings.append("it has args: " + repr(args) + '\n')
            except:
                pass
            print(''.join(helpstrings), file=wfile)
            return
        elif thing in list(globals().keys()):
            print(thing, " is in globals.", file=wfile)
            thing = globals()[thing]
            pydoc.help(thing)
        else :
            # parse string? might be a python expression.
            # could try an eval on it.
            print(thing, "is an unknown string.", file=wfile)
    else : # if it is a python object, call pydoc.help on it.
        pydoc.help(thing)

def see(thing, wfile=sys.stdout) :
    """See the source code associated with an item, if possible.

    it accepts strings or python objects.
    see checks a string argument against the run and det dictionaries,
    then against the global namespace.
    it converts to a python object if possible.
    it tries to get the source code for python objects. 
    """
    
    import inspect
    helpstrings = []
    if isinstance(thing, str) :
        if thing in list(rd.keys()) :
            helpstrings.append( "%s is in the run dictionary.\n"%thing)
        elif thing in list(dd.keys()) :
            helpstrings.append( "%s is in the detector dictionary"%thing)
        elif thing in list(globals().keys()) :
            helpstrings.append( thing + "is in globals.\n")
            thing = globals()[thing]
            helpstrings.append(inspect.getabsfile(thing))
            helpstrings.append('\n')
            helpstrings.append(inspect.getsource(thing))
    else :
        helpstrings.append(inspect.getabsfile(thing))
        helpstrings.append('\n')
        helpstrings.append(inspect.getsource(thing))
    print("".join(helpstrings), file=wfile)

# --------------------------------------------

def vbias(value = None, wfile=sys.stdout) :
    "todo: put this in det dict?"
    if value != None :
         dd['dsub'] = value + dd['vreset']
    else :
         print(dd['dsub'] - dd['vreset'], file=wfile)

from runrun import *
from run import rd, savesetup, recover 
from det import dd, savedet 
from dv import bmax
from movieshow import movieshow

from dsp import rnorm, ron, rrow, rglobal, rampnext, singlepix
import dsp

def rdclockcurrent(wfile=sys.stdout) :
    print(dsp.readBiasCurrent(), file=wfile)

def rdclockrail(wfile=sys.stdout) :
    print(dsp.readBiasVoltage(), file=wfile)

def rdbiascurrent(wfile=sys.stdout) :
    print(dsp.readBiasCurrent(), file=wfile)

def rdbiasvoltage(wfile=sys.stdout) :
    print(dsp.readBiasVoltage(), file=wfile)

try :
    from tmptr import tdiode
except ImportError :
    "add symbolic link 'tmptr.py' to real diode conversion."

def tmps(wfile =sys.stdout) :
    "Prints the current temperature"
    from dsp import vDiode
    print("%7.3f"%tdiode(vDiode()), file=wfile)

def words(filt = None, wfile=sys.stdout) :
    "Prints three columns of words: funcs, run dict, det dict"
    g = globals()
    d = sorted(list(g.keys()))
    funcs = []
    helpstrings = []
    helpstrings.append("%-25s%-25s%-25s\n"%("FUNC:","RUN:","DET:"))
    for word in d :
        if callable(g[word]) :
            funcs.append(word)
    runkeys = list(rd.keys())
    runkeys.sort()
    ddkeys = list(dd.keys())
    ddkeys.sort()
    blank = " "*25
    for i in range(max(len(funcs),len(runkeys),len(ddkeys))) :
        try : helpstrings.append("%-25s"%funcs[i])
        except : helpstrings.append(blank)
        try : helpstrings.append( "%-25s"%runkeys[i])
        except : helpstrings.append( " "*-25)
        try : helpstrings.append( "%-25s"%ddkeys[i])
        except : pass
        helpstrings.append('\n')
    helpstring = "".join(helpstrings)
    print(helpstring, file=wfile)

def rbv(wfile=sys.stdout) :
    voltage = dsp.readBiasVoltage()
    print("%9.6f"%voltage, file=wfile)

def temp2file(filename = "/scratch/data/tempmon" ) :
    from dsp import vDiode
    outfile = open(filename,'w')
    outfile.write("%7.3f"%tdiode(vDiode()))

import tempcontrol

# temp controller needs to know how to read the temp
# and how to set the heater
# create temporary functions

def readTemp() :
    import dsp
    tsum = 0
    for i in range(10) :
        tsum += tdiode(dsp.vDiode())
    return tsum / 10.0

def setHeater(hvolts ) :
    dd['heater'] = int(hvolts * 1000)

# instantiate a controller
    # why not open the device straight from Python?
tc = tempcontrol.tcontroller(readTemp, setHeater)

# we don't use these functions ourselves though.
# delete our references to them.
del readTemp

del setHeater

tc.gain = 20
tc.degrees_K_per_minute = 0.1
# tc.do()

import readline
import rlcompleter
readline.parse_and_bind("tab:complete")
readline.read_history_file()

pagetop = """ 
<html>
<head>
<title>Pydsp on Itchy</title>
</head>
<body>
"""

statuspage = """
<h1>Pydsp Status</h1>

<p> Current Object %s

<p> Night Dir: %s
"""
pageend = """
</body>
</html>
"""

import http.server

class pydspWebServer(http.server.BaseHTTPRequestHandler) :

    server_version = "pydspHTTP/1.0"

    def do_GET(self) :
        self.send_response(200)
        self.send_header("Content-type","text/html")
        self.end_headers()
        try :
            self.wfile.write(pagetop)
            self.wfile.write(statuspage % (rd["object"],rd["night"]))
            self.wfile.write(pageend)
        finally :
            pass

    def do_POST(self) :
        self.send_response(200)
        self.send_header("Content-type","text/html")
        self.end_headers()
        try :
            self.wfile.write(pagetop)
            self.wfile.write(pageforms)
            self.wfile.write("<p>"+self.path)
            self.wfile.write("<p>"+self.raw_requestline)
            self.wfile.write(pageend)
        finally :
            pass

    def log_request(self, *args) :
        pass

    log_message = log_request

    do_HEAD = do_GET

def startWebServer()  : 
    "Fire up a simple embedded web server in its own thread"
    def webThread() :
        webserver = http.server.HTTPServer(("",8070), pydspWebServer)
        webserver.serve_forever()

    global webthread
    webthread = threading.Thread(None,webThread)
    webthread.setDaemon(True)
    webthread.start()

import signal

ifuncs = []

def inthand(signum, stack) :
    # handle signals (such as keyboard interrupts)
    if signum == signal.SIGINT :
        for func in ifuncs :
              func()   
def myhand() :
    import dsp
    dsp.dspthread.interrupt()

