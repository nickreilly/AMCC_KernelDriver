""" dv.py.. Socketed conection to DV program. 

allows system to have DV automatically load in acquired images."""

__version__ = """$Id: dv.py 400 2006-06-19 22:39:30Z drew $ """

__author__ = "$Author: drew $"

__URL__ = "$URL: http://astro.pas.rochester.edu/svn/pydsp/trunk/pydsp/dv.py $"

import socket
import sys

hostname = "localhost"
port = 30123
packsize = 160

# reminder that these things were in the original file.
autoupdatemode = 0
bgsubmode = 0

# _bmax, _srcbuf, the funcs that change them..
# they could be a class.
# then again, they are in this module, and that kind of encapuslates them
# do we need multiple instances? probably not.
# we are only running 1 dv that we are connecting to

_bmax = 3  # max buffer to use.. 0 thru 7 (8 buffers) is max. 
_srcbuf = _bmax # the last buffer we used. (not the next one!)
             # initialize it to -1 to mean no buffer.
buf = ["A", "B", "C", "D", "E", "F", "G", "H"] # 

def send_command(cmd):
    """
    Send command to dv over a socket.
    """
    packet = cmd  + ' ' * (packsize - len(cmd))
    dvsock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    dvsock.connect((hostname, port))
    dvsock.send(packet)
    dvsock.close()

def send2dv(cmd, wfile=sys.stdout):
    """
    Send the command to dv and echo it at the terminal
    """
    try:
        send_command(cmd);
        print(cmd, file=wfile)
    except:
        print("could not connect to dv on '%s', port %d" % (hostname, port), file=wfile)
        return

def view(filename):
    """
    View the image in file 'filename' in dv window
    """
    import run
    cmd = "Read " + run.rd["nightdir"] + "/" +filename + " " + buf[_srcbuf]
    send2dv(cmd);

def bmax(bufnum, wfile=sys.stdout):
    """
    Set the maximum buffer to be used in dv.

    Can use numbers or letters.
    bmax [0-7]
    or
    bmax ["a-h"|"A-H"]
    """
    global _bmax
    global _srcbuf
    if isinstance(bufnum, str):
        bufnum = bufnum.upper()
        if bufnum in buf:
            bufnum = buf.index(bufnum)
    if isinstance(bufnum, int):
        if 0 <= bufnum <= 7:
            _bmax = bufnum
            _srcbuf = bufnum # next new buffer will be A
    else: 
        print("illegal value for bmax! must be 0-7 ", file=wfile)

def math(mathstr):
    "Send math string to DV. Not implemented"
    raise NotImplementedError

def advance_fbuf():
    """
    Advance the "current" source buffer to the next one.
    """
    global _srcbuf
    _srcbuf += 1
    if _srcbuf > _bmax:
        _srcbuf = 0 

def load_bkg(bkgfilename):
    """
    Tell dv to load the bkg file into buffer F
    """
    cmd = "Read " + bkgfilename + " F"
    send2dv(cmd);

def load_src(srcfilename, wfile=sys.stdout):
    """
    Tell dv to load the src file into next src buffer  """
    global _srcbuf
    sbuf = _srcbuf # remember the original buf in case of error.
    advance_fbuf();
    cmd = "Read " + srcfilename + " " + buf[_srcbuf]
    try:
        send_command(cmd);
        print(cmd, file=wfile)
    except:
        print("could not connect to dv on '%s', port %d" % (hostname, port), file=wfile)
        _srcbuf = sbuf
        return

def src_minus_bkg():
    """
    Tell dv to subtract the background from the current source.

    We pre-advance the frame buffer in pydsp, then write the file.
    so it is pointing to the last source buffer we wrote.
    (This is unlike dspsys, which wrote the buffer, then advanced.)
    """
    cmd = "math "+ buf[_srcbuf] + " = " + buf[_srcbuf] + " - F"
    send2dv(cmd);

def dv_path(path):
    """
    Tell dv the path it should use for data files.
    """
    cmd = "path " + path
    send2dv(cmd);

def dv_dofile():
    """
    Legacy. either do a macro or tell dv where macros are. ??
    not sure what this does in fthsys.
    """
    raise NotImplementedError

def active(canvasnum):
    """
    set the active canvas. Canvas = GUI Display, 0 - 8 in L mode
    last one is the big one.
    """
    cmd = "active " + str(canvasnum)
    send2dv(cmd);

def buffer(bufferletter):
    """
    set the buffer for the active canvas.     (A-H in L mode)
    buffer = data source. Each canvas can refer to any buffer 
    """
    cmd = "buffer " + bufferletter
    send2dv(cmd);

def vfmode():
    """
    turn auto-vf mode on or off.
    if turning on, make sure paths are cool. (fthsys did it this way)
    todo: implement?
    """
    raise NotImplementedError


