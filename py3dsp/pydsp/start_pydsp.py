"""
The top level file of the pydsp system.
all execution starts here.
A shell-bang could have been used, but we
used an alias instead.

Basically, start the DSP thread,
initialize the runtime hooks,
start the gui thread
Start some extra stuff like runline completion,
then bring the control system to life.
finally, dump into the command loop.
"""
__version__ = """$Id: start_pydsp.py 426 2007-07-18 19:18:15Z dsp $ """

__author__ = "$Author: dsp $"

__URL__ = "$URL: https://astro.pas.rochester.edu/admin/svn/pydsp/trunk/pydsp/start_pydsp.py $"

import os, sys
import logging as log
sys.path.append('/home/dsp/py3dsp/pydsp/filter_wheel')

exec_dir = '/home/dsp/py3dsp/pydsp/'

# Initialize logger
logger = log.getLogger("pydsp")
logger.setLevel(log.DEBUG)
# make sure file exists
#f = open(os.getcwd()+'/logs/pydsp.log', 'w')
f = open(exec_dir + 'logs/pydsp.log', 'w')
f.close
fh = log.FileHandler(exec_dir + 'logs/pydsp.log')
fh.setLevel(log.DEBUG)

sh = log.StreamHandler()
sh.setLevel(log.WARNING)

formatter = log.Formatter('[%(levelname)s] %(asctime)s:%(name)s - %(message)s')
fh.setFormatter(formatter)
sh.setFormatter(formatter)

logger.addHandler(fh)
logger.addHandler(sh)

def initHardware(hw_mode):
    """
    Initialize the dsp device.

    The dsp initializer wil open up the device driver and
    start the singleton dsp thread. (dsp.dspthread)
    """
    global dev

    from dsp import dev
    # dev = dsp.initialize(hw_mode)
    print(dev)
    return

def loadDetAndRun():
    """
    Load the det (static) and run (volatile) dictionaries.

    This effectively fires up the system, for instance:
    setting the clock program attribute is a "smart" operation,
    and loads the clock program into the DSP as well.
    """
    import det
    det.init()
    

    # print('Loaded Det Dict!')
    import run
    run.init()
    # print('Loaded run Dict!')

def update_det():
    from run import rd
    import det
    load_file = f'{xdir.detpath}/{rd.detname}'
    with open(load_file, 'r') as f:
        for line in f.readlines():
            key, att = line.split(' ')[0], line.split(' ')[1].split('\n')[0]
            if str.isnumeric(att):
                setattr(det.dd, key, int(att))
            else:
                setattr(det.dd, key, att)
            # setattr(det.dd, key, att)
    det.loaddet(rd.detname)


def startPyshowall():
    """
    Attempt to start the pyshowall GUI.

    The gui is coupled to the det and run dictionaries.
    Each dictionary has a set_widget method that is called
    when one of the attribuetes is changed. This updates the GUI.
    Some widget names are different from attribute names, so
    a namemap translates from one to the other.
    """
    try :
        import pyshowall.pyshowall as pyshowall
        import pydsp
        import det
        import run
        # if we can import pyshowall..
        # then tell det.py and run.py how to publish to it.
        det.dd.set_widget = pyshowall.set_widget
        run.rd.set_widget = pyshowall.set_widget
        pydsp.pyshowall_connect() 
        pyshowall.startguithread() # need this started before set_widget calls work
        time.sleep(0.1) # give a little time for thread to sync up - not deterministic may want callback or to wait for a flag to go up? 
        try :
            import detkeys # symbolic link. 
            det.dd.namemap = {}  #
            for name, gname in list(detkeys.namemap.items()) :
                det.dd.namemap[name] = gname+'data'
                pyshowall.set_widget(gname+'name_txt',name)
                print("mapped", name, "to", gname)
        except ImportError :
            print("no detkeys namemap applied.")
            print("add symbolic link named detkeys.py to [detector].py?")
        except :
            import traceback
            traceback.print_exc()
            print("problem with detkeys namemap")
        run.rd.namemap = { "night":"nightdir", "detname":"detfile", 
                "fwp":"fwpos", "fwn":"fwname" }
    except ImportError : 
        print("Import error starting pyshowall")


if __name__ == '__main__':
    global dev

    logger.info('Starting application')


    filterinitialize = True
    # Simulate PCI card and other HW if -s is entered as an option
    # If -u is a flag, attempt to connect with USB, -s and -u are mutually exclusive
    # Sticking on a flag for connecting over usb, mostly for back-compat

    hw_mode = 'pci' # Default the HW mode to pci, could change later if desired
    if '-s' in sys.argv and '-u' in sys.argv:
        logger.error("Incompatable flags, start with either -s -u or neither")
        sys.exit()  # return # exit program

    if '-s' in sys.argv:
        hw_mode = 'sim'
    if '-u' in sys.argv:
        hw_mode = 'usb'
    import xdir

    logger.debug("Starting with hardware mode: {}".format(hw_mode))

    if hw_mode == 'sim':
        logger.info("initializing with simulated hardware interface (dummy mode)")
    elif hw_mode == 'usb':
        logger.info("Initializing hardware interface over USB")
    elif hw_mode == 'pci':
        logger.info("Initializing hardware interface over PCI")
    else:
        logger.error('Unknown Hardware Mode selected, exiting')
        sys.exit()  # return
    
    initHardware(hw_mode)
    # dev = initialize_hw(hw_mode)
    # dev.open()
    logger.debug(f'{dev=}')

    

    logger.debug("loading det and run")
    import det
    import run
    loadDetAndRun()
    update_det()
    logger.debug("loading pydsp")
    import pydsp
    from pydsp import *
    from util_funcs import LastRunObj

    # import * is usually BAD. (Not explicit.)
    # however, we want the namespaces of cloop (in pydsp)
    # and the command prompt (this file)
    # to have considerable overlap. So, it is ok.

    # Commenting out the autocompleter, until I can figure out how it works - joe
    
    #pydsp.doCompleter()

    logger.debug("booting dsp")

    recov, lro = False, LastRunObj(emptylastrun=True)
    if hw_mode != 'sim':
        recov = True
        lro = LastRunObj()
    
    pydsp.startpydsp(recov=recov, lastrunobject=lro) # don't recover if simulating
    
    #if input("start temp control? [no]").lower().startswith("y"):
    # if hw_mode == 'pci':
    logger.debug("pydsp:temp control starting.")
    startTempControl()
    # startPyshowall() # This does a weird thread in a thread thing, necessary?
    exit_pydsp = cloop()  # pass our namespace into cloop???
    if exit_pydsp:
        print(f'Good work today!  You can close this with exit() or ctrl-d')
        sys.exit()
