#
# -*- Mode: Python; py-indent-offset: 4 -*-
""" Python version of gshowall (much much smaller!!)

uses XML output from glade-2 UI builder.
Basically does two things:
1: supports a command box, in which the user can enter commands.
2: allows the user to edit some boxes. 

Requires pyGTK, the python bindings to the GIMP toolkit.

pyGTK is 1.99 or 2.xx
GTK is 2.0, 2.2, 2.4 ish..

we have gtk-2.0 working here.

"""

__version__ = """$Id$ """

__author__ = "$Author$"

__URL__ = "$URL$"


import gtk
import gobject
import gtk.glade

import sys

import xdir
import logging

logger = logging.getLogger("pydsp.pyshowall") 
logger.debug("starting pyshowall")

_wfile = sys.stdout

verbose = 0 # can set to true if printed commands desired.

# This module has rd, dd, and runrun patched into it from the outside. 
# it assumes a pair of dictionaries named rd and dd,
# but by default it uses empty ones so it can be tested independently.
# startup will put the correct dictionaries in.
# XXX could just have a list of dictionaries to look in?

from DataDict import DataDict
rd = DataDict()  
dd = DataDict()

def on_focus_out_event(*args) :
    widgetname = args[0].get_property('name')
    if widgetname[-4:] == "_txt" :
        name = widgetname[:-4]
        if name in list(rd.keys()) : 
            value = rd[name]
        elif name in list(dd.keys()) : 
            value = dd[name]
        else :
            return
        args[0].set_property('text', value)
    elif widgetname == 'command' :
        return

def activatecmd(cmd) :
    """
    Called (by on_command_activate) when the user enters
    a command into the command box in pyshowall.
    cmd is the string that was entered.

    the default is a do-nothing command. 
    Gets overridden. pyshowall.activatecmd = xxxx
    """
    # XXX could do as publish-subscribe with a list of
    # functions to call. Initial list would be empty.
    print(cmd, file=_wfile)

def on_command_activate(*args) :
    """called from gtk when a command is entered in the command box.
    
    hook from the command box widget into the system.
    gets the text property of the command box
    (the request that was typed in.)
    Clears the command box, and 
    calls pyshowall.activatecmd with the request. """
    request = command_box.get_property('text') 
    command_box.set_property('text','') 
    activatecmd(request)

"""
def on_keypress(widget, event, *args) :
    ""
    print>>_wfile, "on_keypress"
"""

def on_enter_notify_event(*args) :
    """
    When focus enters an edit box,
    show help in status bar
    """
    name = args[0].get_property('name')
    if name[-4:] == "_txt" : name = name[:-4]
    if name in list(rd.keys()) : 
        doc = rd.docstring.get(name,'')
    elif name in list(dd.keys()) : 
        doc = dd.docstring.get(name,'')
    elif name == 'command' :
        doc = 'Type a command in this box.'
    else :
        doc = ''
    if doc == None : doc = ''
    status_bar.pop(1)
    status_bar.push (1, doc)
    
#autoenter = True
def on_leave_notify_event(*args) :
    """when focus leaves an edit box, clear status bar

    rewrite box, too. Rewrite only needs to be done if the box
    was editable, and edited."""
    # XXX add this check.
    status_bar.pop(1)
    status_bar.push (1,"")

def gui_raw_input(prompt = "") :
    """gui version of raw_input function.

    Pass in the prompt. It blocks and returns a
    response to the caller."""
    dlg = gtk.Dialog(title = "PyShowall" , parent=gsa, flags = gtk.DIALOG_MODAL)
    dlg.set_property('window-position',gtk.WIN_POS_CENTER_ON_PARENT)
    def enterfunc(arg) :
        dlg.response(1)
    label = gtk.Label(prompt)
    label.set_line_wrap(True)
    label.show()
    dlg.vbox.add(label)
    entry = gtk.Entry()
    entry.set_activates_default(True)
    entry.connect('activate',enterfunc)
    entry.show()
    dlg.vbox.add(entry)
    dlg.run()
    result = entry.get_text()
    dlg.destroy()
    return result

class gui_writer :
  """
  A file-like object that accepts and prints strings.
  """
  def write(self, printstring) :
    if not printstring.strip() :
        return
    dlg = gtk.Dialog(title="PyShowall" , parent=gsa, flags=gtk.DIALOG_MODAL)
    dlg.set_property('window-position', gtk.WIN_POS_CENTER_ON_PARENT)
    def enterfunc(arg) :
        dlg.response(1)
    label = gtk.Label(printstring)
    label.set_line_wrap(False)
    label.set_justify(gtk.JUSTIFY_LEFT)
    label.show()
    dlg.vbox.add(label)
    button = gtk.Button(stock=gtk.STOCK_OK)
    button.connect('clicked', enterfunc)
    button.show()
    dlg.vbox.add(button)
    dlg.run()
    dlg.destroy()

# the UI has several buttons 
# these are the do-nothing default handlers.
# pydsp plugs in the real ones.

def abortfunc() :
    print("override me", file=_wfile)

def scanfunc() :
    print("override me", file=_wfile)

def runfunc() :
    print("override me", file=_wfile)

def showbiases() :
    print("override me", file=_wfile)

def on_Abort_clicked(*args) :
    abortfunc()

def on_Scan_clicked(*args) :
    scanfunc()

def on_Run_clicked(*args) :
    runfunc()

def on_Voltages_clicked(*args):
    showbiases();

def on_activate(*args) :
    """generic handler for editable run and det dict widgets.

    hook FROM pyshowall into the system.
    detects that the widget has been changed.
    extracts the name and value from the widget
    and forms a text command. Then calls command processor
    on this text string."""
    for arg in args :
        name = arg.get_property('name')
        value = arg.get_property('text')
        if name[-4:] == "_txt" : name = name[:-4]
        if verbose :
            print("setting", name, "to", value, file=_wfile)
            if name in list(rd.keys()) : print("runword", file=_wfile)
            if name in list(dd.keys()) : print("detword", file=_wfile)
        print(name + ' ' + value, file=_wfile)
        activatecmd(name + ' ' + value)

# Handler for when Window is destroyed (X button clicked)
def on_destroy(*args):
    global guithread
    logger.info("Called on_destroy, closing with guithread.shutdown()")
    guithread.shutdown()

import time
starttime = time.time()

def scantime() :
    "dummy function. will be patched in when system comes up."
    return 10000

def totaltime() :
    "return the total frame time expected. Smart UI. so what??"
    logger.debug("called totaltime")
    try :
        return (scantime())/1000.0
    except :
        logger.warning("Hit exception in totaltime")
        return 10.0 # 

def mytimer() :
    logger.debug("called mytimer")
    global guithread
    if guithread.scanning:
        elapsed = (time.time() - starttime ) / totaltime()
        if 0.0 <= elapsed <= 1.0 :
            progress_bar.set_fraction(elapsed)
            return 1
        else :
            progress_bar.set_fraction(1.0)
    else:
        logger.debug("Timeout called, not scanning")
    return 0

def restart_timer() :
    global starttime
    gobject.timeout_add(100, mytimer) # first, make a timer.
    starttime = time.time() # now, tell it to go.

def stop_timer() :
    global starttime
    starttime = time.time() - totaltime()

def init_showall() :
    global xml
    global gsa
    global command_box
    global status_bar
    global progress_bar
    xml = gtk.glade.XML(xdir.dsphome+'/pydsp/pyshowall/gshowall.glade') 
    command_box = xml.get_widget("command") # look up the widget.
    status_bar = xml.get_widget("status") # look up the widget.
    progress_bar = xml.get_widget("timer_progress")
    xml.signal_autoconnect(globals())
    xml.signal_connect("on_destroy", on_destroy)
    #gobject.timeout_add(500, mytimer) This maybe doesn't need to be added until later
    gsa = xml.get_widget("gshowall")

def do_update() :
    """call gtk and process events till no more events are pending.

    makes sure that anything in the gui that needs
    updating gets updated."""
    iters = 0
    while gtk.events_pending() : 
        gtk.main_iteration()
        iters += 1
    return iters
    
def set_widget(name,value) :
    """sets the value of a named widget.

    """
    gtk.gdk.threads_enter()
    widget = xml.get_widget(name) # look up the widget.
    widget.set_property('text', value ) # set the property
    gtk.gdk.threads_leave()

import threading 
class GuiThread(threading.Thread) :
    """
    The gui runs in this object, which is a separate thread.
    """
    def __init__(self) : #, externalCallable, **kwds) :
        threading.Thread.__init__(self)
        #self.oktorun = False
        #self.inited = False
        self.done = False
        self.scanning = False
        #self.start() # make thread active and run() separately. joseph - why?
        import atexit
        atexit.register(self.shutdown)
    
    def shutdown(self) :
        """
        shut down in an orderly fashion.
        """
        self.done = True

    def run(self):
        """
        The main process of this thread.

        """
        import time
        try:
            print("initializing window")
            init_showall()
        except Exception as e:
            print("can't init showall!")
            print(str(e))
            return
        #self.inited = True
        time.sleep(0.1)
        while True :
            if self.done:
                print("Thread is done, exiting")
                return
            do_update()
            time.sleep(0.01)

def startguithread() :
    """
    create a global instance of pyshowall in a separate thread.
    """
    import time
    # fire up gui thread first.
    global guithread
    guithread = GuiThread()
    logger.info("Starting GuiThread")
    guithread.start()
    # joseph - who the hell designed this garbage
    """
    while not guithread.inited :
        time.sleep(0.1)
    guithread.oktorun = True
    """



