"""
 pydsp.py -- the almost top level file.

 Usage was originally: python -i pydsp.py
 Now, start_pydsp.py is the top level.
 Usage is effectively: python -i start_pydsp.py

 Importing this file again when it was the top level
 caused undesired execution of stuff (first import.)
 Plus, it seemed an architectural error to re-import the 
 top file.

 Alias startup to 'pydsp'
 by inserting the line:
 alias pydsp='python -tt -i $DSPHOME/pydsp/start_pydsp.py'
 in ~/.bashrc
 with this alias, just type 'pydsp' at the command line.

 it emulates dspsys.fth in the dspsys forth-based system

 TODO: It has a mishmash of things and could use a refactor.
 Its primary focus should be defining the namespace that the user sees. 
 Sections of that namespace might be sensibly broken out to separate files.
 The help functions need access to that namespace, so they
 wound up here.
"""

__revision__  = """$Id: pydsp.py 406 2006-09-22 16:36:54Z drew $"""

__version__ = """$Id: pydsp.py 406 2006-09-22 16:36:54Z drew $"""

__author__ = "$Author: drew $"

__URL__ = "$URL: http://astro.pas.rochester.edu/svn/pydsp/trunk/pydsp/pydsp.py $"

import sys
import dsp as dsp
import os

def help(thing = None, input=input,  wfile=sys.stdout):
    """
    Wrapper for python's pydoc help function.

    It checks input; if it is a string, check all the likely
    places for a match: the run dict, the det dict, and the
    globals. If it is not a string, just pass it to pydoc.
    """
    import pydoc
    if not thing: # user just typed "help[cr]. give extra help."
        helpstring = 'Type "help [name]" to get help on a specific word.\n' \
        '"words" will tell you a whole bunch of words you can use\n' \
        '"see" will try to dig up source code for an object\n' \
        '"break" will go from ur_ok prompt to >>> prompt\n' \
        '"bye" will exit the program\n'\
        '"cloop()" will go from >>> prompt to ur_ok prompt\n' \
        'The >>> prompt is the python interpreter prompt.\n' \
        'The ur_ok prompt is the command interpreter.\n' \
        'The command interpreter IS the python function cloop()\n' \
        'Lines containing parentheses () or semicolons ; are\n' \
        'treated as Python by the command interpreter.'
        print(helpstring, file=wfile)
    elif thing == "all": # Print all the docstrings to a file.
        # If the output is a file, no paging.
        if (wfile != sys.stdout):
             temp_term = os.environ['TERM']
             os.environ['TERM'] = 'dumb'

        temp_dict = list(rd.keys()) + list(dd.keys()) + list(globals().keys())
        for help_word in temp_dict: 
             wfile.write("========================\n")
             help(help_word, input, wfile)

        if (wfile != sys.stdout):
             os.environ['TERM']=temp_term
    elif isinstance(thing, str): # is it a word? try to look it up.
        helpstrings = []
        docstring = None
        if thing in list(rd.keys()): 
            dictobj, dictname = rd, "run"
        elif thing in list(dd.keys()): 
            dictobj, dictname = dd, "det"
        else:
            dictobj, dictname = None, ""
        if dictname:
            helpstrings.append("%s is a %s dictionary word.\n" % (thing, dictname))
            docstring = dictobj.docstring[thing]
            if docstring: 
                helpstrings.append(docstring+'\n')
            try:
                getfunc = dictobj.getfunc[thing]
                if dictobj.getfunc[thing]:
                    helpstrings.append("it has a getfunc, "+repr(getfunc) + '\n')
                    if getfunc.__doc__:
                        helpstrings.append(getfunc.__doc__+'\n')
            except:
                pass
            try:
                setfunc = dictobj.setfunc[thing]
                if setfunc:
                    helpstrings.append("it has a setfunc, "+repr(setfunc) + '\n')
                    if setfunc.__doc__:
                        helpstrings.append(setfunc.__doc__+'\n')
            except:
                pass
            try:
                args = dictobj.args[thing]
                if args:
                    helpstrings.append("it has args: " + repr(args) + '\n')
            except:
                pass
            print(''.join(helpstrings), file=wfile)
            return
        elif thing in list(globals().keys()):
            print(thing, " is in globals.", file=wfile)
            thing = globals()[thing]
            pydoc.help(thing)
        else:
            # parse string? might be a python expression.
            # could try an eval on it.
            print(thing, "is an unknown string.", file=wfile)
    else: # if it is a python object, call pydoc.help on it.
        pydoc.help(thing)

def see(thing, wfile=sys.stdout):
    """
    See the source code associated with an item, if possible.

    It accepts strings or python objects.
    'see' checks a string argument against the run and det dictionaries,
    then against the global namespace.
    If a match is found, it converts to a python object if possible and subsequently
    tries to get the source code for python objects. 
    """
    
    import inspect
    helpstrings = []
    if isinstance(thing, str):
        if thing in list(rd.keys()):
            helpstrings.append( "%s is in the run dictionary.\n" % thing)
        elif thing in list(dd.keys()):
            helpstrings.append( "%s is in the detector dictionary" % thing)
        elif thing in list(globals().keys()):
            helpstrings.append("%s is in globals.\n" % thing)
            thing = globals()[thing]
            helpstrings.append(inspect.getabsfile(thing))
            helpstrings.append('\n')
            helpstrings.append(inspect.getsource(thing))
    else:
        helpstrings.append(inspect.getabsfile(thing))
        helpstrings.append('\n')
        helpstrings.append(inspect.getsource(thing))
    print("".join(helpstrings), file=wfile)

def words(filt = None, wfile=sys.stdout):
    "Prints three columns of words: funcs, run dict, det dict"
    g = globals()
    d = sorted(list(g.keys()))
    funcs = []
    helpstrings = []
    helpstrings.append("%-25s%-25s%-25s\n" % ("FUNC:", "RUN:", "DET:"))
    for word in d:
        if callable(g[word]):
            funcs.append(word)
    runkeys = list(rd.keys())
    runkeys.sort()
    ddkeys = list(dd.keys()) 
    ddkeys.sort()
    blank = " "*25
    for i in range(max(len(funcs), len(runkeys), len(ddkeys))):
        try: helpstrings.append("%-25s" % funcs[i])
        except IndexError: helpstrings.append(blank)
        try: helpstrings.append( "%-25s" % runkeys[i])
        except IndexError: helpstrings.append( " "*-25)
        try: helpstrings.append( "%-25s" % ddkeys[i])
        except IndexError: pass
        helpstrings.append('\n')
    helpstring = "".join(helpstrings)
    print(helpstring, file=wfile)

# --------------------------------------------
# hmm.. explicit is better than implicit.
#from runrun import sscan, bscan, scan, srun, brun, runit as run
#from runrun import pedscan, sigscan, src, bkg, vdiode
#from runrun import pedrun, sutr, scandir, scantime
#from runrun import saveped, savesig, crun, calibrateItime, preCheckItime

from runrun import * # __all__ defined in runrun
from run import rd, savesetup, setup
from det import dd, savedet, powerup, powerdown
from dv import bmax
from dsp import rnorm, ron, rrow, rglobal, rampnext, singlepix
from dsp import seeclockpin, seebiaspin
from movieshow import movieshow
from ls332 import *

def rdclockcurrent(wfile=sys.stdout):
    """ Internal function, do not use. """
    print(dsp.readBiasCurrent(), file=wfile)

def rdclockrail(wfile=sys.stdout):
    """ Internal function, do not use. """
    print(dsp.readBiasVoltage(), file=wfile)

def rdbiascurrent(wfile=sys.stdout):
    """ Internal function, do not use. """
    print(dsp.readBiasCurrent(), file=wfile)

def rdbiasvoltage(wfile=sys.stdout):
    """ Internal function, do not use. """
    print(dsp.readBiasVoltage(), file=wfile)

def rbv(wfile=sys.stdout):
    voltage = dsp.readBiasVoltage()
    print("%9.6f" % voltage, file=wfile)

try:
    from tmptr import tdiode
except ImportError:
    "add symbolic link 'tmptr.py' to real diode conversion."

def tmps(wfile=sys.stdout, **kwds):
    "Prints the current temperature"
    if useTempControl == 'softwareTempControl':
        # software version of temperature read/control
        from dsp import vDiode
        dd.temp = "%7.3f" % tdiode(vDiode())
    elif useTempControl == 'hardwareTempControl':
        dd.temp = readTemp()
    else:
        print("You need to set whether to use software or hardware Temperature Control.")
    print(f'{dd.temp}', file=wfile)

def tmpsB(wfile=sys.stdout, **kwds):
    "Prints the current temperature"
    if useTempControl == 'softwareTempControl':
        # software version of temperature read/control
        from dsp import vDiode
        dd.temp = "%7.3f" % tdiode(vDiode())
    elif useTempControl == 'hardwareTempControl':
        dd.temp = readTempB()
    else:
        print("You need to set whether to use software or hardware Temperature Control.")
    print(f'{dd.temp}', file=wfile)

tmps = dsp.dspthreadable(tmps)
tmpsB = dsp.dspthreadable(tmpsB)

def loaduser(name = "", wfile=sys.stdout):
    """
    Not implemented. To load in a user program use execuser instead.
    """
    try:
        import sys
        import os
        if not name:
            print("you must specify a filename!", file=wfile)
            return
        if len(name) < 3 or name[-3:] != '.py':
            name = name + '.py'
        # assume the name of this file is pydsp.py
        prog = sys.argv[0][:-8] + 'acq/' + name
        if (os.access(prog, os.R_OK)): # can we read it?
            rd['userprog'] = prog
            print("%s was found." % prog, file=wfile)
        else:
            print("%s not found" % prog, file=wfile)
    except:
        print("loaduser error", file=wfile)

def userload(name="", wfile=sys.stdout):
    """
    Not implemented. To load in a user program use execuser instead.
    """
    try:
        import sys
        import os
        if not name:
            print("you must specify a filename!", file=wfile)
            return
        if len(name) < 3 or name[-3:] != '.py':
            name = name + '.py'
        # assume the name of this file is pydsp.py
        prog = sys.argv[0][:-8] + 'acq/' + name
        if (os.access(prog, os.R_OK)): # can we read it?
            rd['userprog'] = prog
            print("%s was found." % prog, file=wfile)
        else:
            print("%s not found" % prog, file=wfile)
        if not (name[:-3] in d):
            d.append(name[:-3])
        exec(name[:-3] + "= tmps")
    except:
        print("error", file=wfile)

def execuser(filename = None, input=input , wfile=sys.stdout):
    """
    Run a user program from the acq directory.

    User program can be a straight python script, or file
    that defines a bunch of new words.

    Any new words are added to the globals dictionary.
    """
    if not filename:
        filename = input("file name?")
    if not filename:
        print("not run", file=wfile)
    if filename[-3:] != '.py':
        filename += '.py'
    import xdir
    exec(compile(open(xdir.dsphome+'/pydsp/acq/'+filename, "rb").read(), xdir.dsphome+'/pydsp/acq/'+filename, 'exec'), globals())

def runuser(wfile=sys.stdout):
    """
    Not implemented. To load in a user program use execuser instead.
    """
    if rd['userprog']:
        try:
            print("execute %s" % rd['userprog'], file=wfile)
            exec(compile(open(rd['userprog'], "rb").read(), rd['userprog'], 'exec'))
        except:
            print("error executing user program", file=wfile)
    
# temperature control words.

def tcstart():
    """
    Activate temp control. Use 'tcgoto' to set the desired temp. 

    NOTE: Hardware temperature control not yet implemented.
    """
    startTempControl()

def tcstop():
    """
    Stop temp control.

    NOTE: Hardware temperature control not yet implemented.
    """
    dsp.dspthread.tc = None

def tcgoto(temp) :
    """
    Set a new goal temperature.

    NOTE: Hardware temperature control not yet implemented.
    """
    temp = float(temp)
    if dsp.dspthread.tc:
        dsp.dspthread.tc.goal = temp
    else :
        print("tc is off")

def tcgoal():
    """
    Print the current temperature goal. (Final temp)

    NOTE: Hardware temperature control not yet implemented.
    """
    print("%8.3f" % dsp.dspthread.tc.goal)

def tccarrot():
    """
    Print the current instantaneous temperature carrot.

    NOTE: Hardware temperature control not yet implemented.
    """
    print("%8.3f" % dsp.dspthread.tc.carrot)

from time import sleep # allow sleep from command line

def tcwait():
    """
    Wait for the desired temperature (carrot) to reach the goal temperature.

    NOTE: Hardware temperature control not yet implemented.
    """
    if not dsp.dspthread.tc:
        print("tc is off")
        return
    while dsp.dspthread.tc.goal != dsp.dspthread.tc.carrot:
        sleep(1)

def temp2file(filename = "/scratch/data/tempmon" ):
    from dsp import vDiode
    outfile = open(filename,'w')
    dd.temp = "%7.3f" % tdiode(vDiode())
    outfile.write(dd.temp)

def startTempControl():
    """
    connect the automatic temperature control up to the system.
   
    There are two ways to read and control the temperature:
    1) Software -- DSP used to read temperature sensor and DSP used to
        set a heater voltage
    2) Hardware -- interface to a LakeShore temperature controller.
    """
    
    """
    For now, this is hard-coded to run the LakeShore TC, but this
    should be put into the lastrun file as a user adjustable parameter.
    """
    hardwareTempControl()

def softwareTempControl():
    """
    This is the software version of the temperature controller.
    It uses the old UR temperature box, which outputs the voltage
    across a temperature sensor (diode).  Then it uses an A/D converter
    in the black box to read the voltage, which is then sent by the
    DSP to the computer.  The computer then controls the temperature
    by increasing or decreasing the voltage across the heater resistor.
    """    
    import tempcontrol

    # temp controller needs to know how to read the temp
    # and how to set the heater
    # create temporary functions

    def readTemp():
        import dsp
        tsum = 0
        for i in range(10):
            tsum += tdiode(dsp.vDiode())
        temp = tsum / 10.0
        dd.temp = "%7.3f" % temp 
        return temp

    def setHeater(hvolts ):
        dd['heater'] = int(hvolts * 1000)

    # this func used anymore?
    def syncReadTemp():
        from run import rd
        return float(dd.temp)

    # this func used anymore?
    def syncSetHeater(hvolts ):
        hvolts *= 1000
        print("setting heater to", int(hvolts))
        import dsp
        if hvolts - 50 < dd.heater < hvolts + 50 :
            pass 
            
    # instantiate a controller
    # why not open the device straight from Python?

    tmps()
    tc = tempcontrol.tcontroller(syncReadTemp, syncSetHeater)
    tc.gain = 20
    tc.degrees_K_per_minute = 0.07
    tc.set_dc_calibration((3.0, 29.6), (2.8, 28.1)) 
        # need public interface, persist.!!!
    dsp.dspthread.tc = tc
    
def hardwareTempControl():
    """
    This is the hardware version of the temperature controller.
    It uses the LakeShore Temperature Controller.
    """
    import ls332
    #from ls332 import *  # user needs all those commands.
    ls332.openTempSensor()
    
    
# command loop stuff
def try_int( token ):
    """
    Try to make the token into an integer

    If it can be made an int, return the int.
    else, return the original token.
    """
    try:
        return int(token) 
    except:
        return token

def execcmd(s, input = input, wfile=sys.stdout):
    """
    Process s, a text command string.

    It might be a smart dictionary (run or det) word,
    or it might be a function.

    Check out ipython from scipy before rewriting this.
    """
    import sys

    tok = s.split() # break it into tokens.
    ntoks = len(tok) # how many do we have?
    if ntoks == 0: # none? blank line.. well, ok, that's cool.
        return
    # Handle global comments (gc) and local comment (lc) command
    if tok[0] in ('gc','lc'): 
        s = s.strip() # remove leading and trailing whitespace
        s = s[len(tok[0]):] # remove the command.. 
        s = s.strip() # remove the whitespace after the command
        rd[tok[0]] = s # assign the line into the run dict
        return
    # certain commands look like python source.
    # just do it.
    if ("(" in s or ":" in s or ";" in s): # use of (:; = a python line
        exec(s) # exec the darn thing as is..
        return # and keep on truckin.
    if ntoks == 1:  # single token? print value it refers to.
        if tok[0] == "bye":
            savesetup()
            readline.write_history_file()
            print("byebye!", file=wfile)
            sys.exit(0)
            # raise SystemExit
        if tok[0] == "break":
            print("type cloop() to go back to the ur_ok prompt.", file=wfile)
            raise StopIteration
        # These commands require a sanity check
        # at the command line.
        # XXX remove this?? Can they be curried instead?
        runwords = ("sscan", "scan", 
                    "srun", "run", 
                    "bscan", "brun",  
                    "pedscan", "sigscan") 
        if tok[0] in runwords:
            if preCheckItime(input):
                return
        # if it is a python callable?
        if tok[0] in globals(): # single word, in dict?
            pyobj = globals()[tok[0]]
            if callable(pyobj): 
                try: # try passing in both input and output
                    retval = pyobj(input = input, wfile = wfile)
                except TypeError: # no? well, how bout just output?
                    try:
                        retval = pyobj(wfile = wfile)
                        if retval:
                            print(retval, file=wfile) 
                    except: # no? then the heck with it. Use the terminal.
                        retval = pyobj()
                if isinstance(retval, str): # did it return a string?
                    print(retval, file=wfile) # then print that.
            else :
                print(pyobj, file=wfile)
        # could rd / dd contain callables?
        elif tok[0] in rd:
            print(rd[tok[0]], file=wfile)
        elif tok[0] in dd:
            print(dd[tok[0]], file=wfile)
        else:
            print(tok[0] + "??", file=wfile)
        return
    elif ntoks == 3:
        # commands such as "nrow = 100" and "nrow 100" are the same (both set)
        if tok[1] == '=':  
            ntoks = 2
            tok[1] = tok[2]
        # Now check for CVF on filter wheel. 
        # Should this code be so explicit in checking CVF or more general? 
        elif tok[0] == 'filter':  # more general? rd.has_key(tok[0])
            if tok[1][0:3] == 'cvf':  # matches first 3 letters: cvfIII cvfJH
                #rd[tok[0]] = (tok[1],tok[2])  # pass a tuple to set filter
                tok[1] = (tok[1],tok[2])
                ntoks = 2
    if ntoks == 2:
        arg = try_int(tok[1])
        if tok[0] in globals(): # first word is in dict?
            func = globals()[tok[0]]
            try:
                func(arg, wfile=wfile)
            except:
                func(arg)
        elif tok[0] in rd: # command is in run dictionary?
            rd[tok[0]] = arg # assign 
        elif tok[1] in rd: # try the other way too.
            rd[tok[1]] = try_int(tok[0]) # assign 
        elif tok[0] in dd:
            dd[tok[0]] = arg # assign the integer.
        elif tok[1] in dd:
            dd[tok[1]] = try_int(tok[0]) # assign the integer.
        else:
            print("can't understand that command.", file=wfile)
    else:
        print("too many words! I'm confused!", file=wfile)

def pyshowall_connect():
    """
    Connect pyshowall to the system.

    Pyshowall attempts to be generic. It is the standard interface
    for dealing with camera arrays. It does not know about the electronics.
    The main app may have different ways of hooking into the pyshowall funcs,
    so in this function, we plug in the application specific hooks.
    The main app conforms to it more than it conforms to the main app.
    """
    import det
    try:
        import pyshowall.pyshowall as pyshowall
        guiprint = pyshowall.gui_writer()
        guiinput = pyshowall.gui_input
    except:
        return

    # pydsp tells pyshowall what to run when a widget is changed.
    def activatecmd(commandline):
        """
        Execute a command entered from pyshowall

        When a user enters something in pyshowall, a command line
        is generated and sent to this function.
        """
        readline.add_history(commandline)
        try:
            execcmd(commandline, input=guiinput, wfile=guiprint)
        except SystemExit:
            guiinput("do you really want to exit??")
            print("need to use the command line. sorry.", file=guiprint)
            return
    pyshowall.activatecmd = activatecmd

    def scanfunc():
        """
        Respond to a scan button poke in pyshowall.
        """
        activatecmd("sscan")
    pyshowall.scanfunc = scanfunc # hook up the scan function

    def runfunc():
        """
        Respond to a run button poke in pyshowall.
        """
        activatecmd("srun")
    pyshowall.runfunc = runfunc # hook up the run function

    def showbiases():
        """
        Respond to a voltages button poke in pyshowall
        """
        det.showbiases(wfile=guiprint)
    pyshowall.showbiases = showbiases

    # there is an abort button in pyshowall that should
    # stop acquisition. hook it up to a real function.
    pyshowall.abortfunc = dsp.dspthread.interrupt 

    # the goal of the following lines is to allow pyshowall
    # to be run with a dummy run and det dict..
    # Pyshowall should not depend upon a particular run and det dict.
    pyshowall.rd = rd
    pyshowall.dd = dd

    # Pyshowall has a timer that displays the current scan progress.
    # the following line 
    import runrun
    pyshowall.scantime = runrun.scantime

    # when pydsp starts and stops acquisition, we want to send
    # a message to pyshowall to start and stop the progress display.
    if not pyshowall.restart_timer in dsp.startfuncs:
        dsp.startfuncs.append(pyshowall.restart_timer)
    if not pyshowall.stop_timer in dsp.stopfuncs:
        dsp.stopfuncs.append(pyshowall.stop_timer)

    # the following is optional..
    # it is an early attempt to remote the command line UI completely.
    # It might be a good way to go.
    if 0 :
        import threading
        import server
        t = threading.Thread(target = server.sockserv)
        t.setDaemon(True)
        t.start()
        def activatecmd(commandline):
            readline.add_history(commandline)
            try:
                execcmd(commandline, 
                    input=server.sockfile.input, wfile=server.sockfile)
            except:
                pass

        server.activatecmd = activatecmd

def cloop(input=input, wfile=sys.stdout):
    """
    The main pydsp command loop. 

    Very simple. Replace this with ipython someday.
    It gets and sets things in the run dict or the data dict.
    or, runs one of the commands in this file.
    """
    import sys
    import time
    print('', file=wfile)
    filterinitialize = True

    from textwrap import dedent
    welcomeText = dedent("""\
    =============================================================
    *********************   pydsp   *****************************
    - this is the interactive command loop for pydsp
    - enter "help" for a list of commands
    - enter "bye" to close the program
    =============================================================\
    """)

    print(welcomeText, file=wfile)

    exit_pydsp = False
    while not exit_pydsp: # keep on asking user input
        try:
            if dd.booted:
                status = "ok"
            else:
                status = "not_ok"

            s = input(f"\rpydsp|{status}|> ") # read command line.
            execcmd(s, input=input, wfile=wfile) # execute command
        except SystemExit:
            # sys.exit()
            exit_pydsp = True
            # raise # allow this exception to propogate
            # return True
            return exit_pydsp
        except (EOFError, StopIteration) : 
            # control-D raises EOFError
            # 'break' raises StopIteration
            print('', file=wfile)
            # return True
            return exit_pydsp
        except KeyboardInterrupt :
            print("Keyboard Interrupt")
            dsp.dspthread.interrupt("cloop")
        except:
            import traceback
            traceback.print_exc(file=wfile)
    # return exit_pydsp
    

import readline
def doCompleter():
    """
    Handles tab completion.

    Hook up the readline module so that all commands are
    logged to a history file and are available after the
    system shuts down and gets restarted. 
    """
    try:
        import rlcompleter
        readline.parse_and_bind("tab:complete")
        readline.read_history_file()
    except:
        print("no history file")

# the main function to start up pydsp
ifuncs = []

def startpydsp(recov, lastrunobject):
    """
    Iinitialize the system from the base defaults.

    Then recover the data that is in lastrun.run
    then hook up a control-c interrupt handler,
    and then the filter wheel.
    """
    import time
    import det
    import signal
    import logging as log
    logger = log.getLogger("pydsp.pydsp")
    logger.debug("reading dsp")
    if recov:
        # print("pydsp.py::startpydsp running recover")
        logger.debug("pydsp.py::pydsp running recover")
        setup(lastrunobject) # first run of recover should NOT move the wheel.
        # print("pydsp.py::startpydsp recovered")
        logger.debug("pydsp.py::pydsp recovered!")
        

    # ociw module has a function it calls to see if acquisition
    # has been interrupted. Plug in the actual working function.
    # this keeps ociw from being directly dependent upon dsp.
    dsp.amcc.interrupted = lambda: dsp.dspthread.interrupted 

    # a control-C should abort things elegantly.
    # here we plug in a signal handler so that a control-C
    # will tell the DSP thread to stop.
    def myhand():
        dsp.dspthread.interrupt("sigcatcher")

    # the real handler has a list of functions that it calls
    # when a control-C is received.
    def inthand(signum, stack):
        # handle signals (such as keyboard interrupts)
        if signum == signal.SIGINT:
            for func in ifuncs:
                func()   

    # install interrupt handler in dsp thread.
    signal.signal(signal.SIGINT, inthand)
    ifuncs.append(myhand)
    

    # start up the filter wheel.
    # another instance of plugging in real methods to an
    # abstract implementation.
    # we want to get rid of the symbolic links, in favor
    # of something like __import__.
    try:
        import run
        import filterBase
        # The original move was fake (so startup doesn't move wheel)
        filterBase.move = run.fwpUpdate
    except ImportError:
        print("must add symbolic link 'filters.py' to real filter file.")
    

    # set the acquisition mode to src.
    src()

