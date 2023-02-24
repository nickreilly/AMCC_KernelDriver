//
// Version:
//     $Id: ociwpy.c,v 1.4 2004/05/11 02:05:09 dsp Exp $
//
// Revisions:
//     $Log: ociwpy.c,v $
//     Revision 1.4  2004/05/11 02:05:09  dsp
//     some documentation and cleanup.
//
//     Revision 1.3  2004/02/24 23:15:42  dsp
//     removed print statements.
//
//     Revision 1.2  2004/02/14 16:58:38  dsp
//     eliminated hard-coded Python include.
//
//     Revision 1.1  2003/08/14 02:36:49  dsp
//     initial UR version. previously used at RIT
//
//
//  history from when it was broken in many files..
//  and this file was ociwmodule.c..
//     
//     Revision 1.13  2002/12/06 19:46:50  drew
//     added function to change the destination image number.
//
//     Revision 1.12  2002/11/25 08:01:46  lars
//     changed get_buf to take an argument.
//     This allows multiple destination buffers to be mapped.
//
//     Revision 1.11  2002/11/24 19:54:33  drew
//     latest stuff.
//
//     Revision 1.10  2002/09/26 10:08:18  lars
//     Added setimagemode to change the image you get back.
//
//     Revision 1.9  2002/08/21 10:30:12  lars
//     Removed pixel by pixel access and replaced with faster routine with buffer
//     access
//
//     Revision 1.8  2002/08/15 16:30:25  lars
//     Added getnextpixel and resetlocation.
//     Replace by faster access in future!
//
//     Revision 1.7  2002/08/05 16:27:03  lars
//     Added pixtogo()
//     Changed mmap to take just a size, no offset
//
//     Revision 1.6  2002/08/01 13:47:22  lars
//     Removed the remap command from the interface
//
//     Revision 1.5  2002/07/29 15:17:40  lars
//     Can now reconfigure mmap size dynamically.
//     Can also get the buffer size the driver uses
//
//     Revision 1.4  2002/07/19 09:28:21  lars
//     Added a close command the the mmap gets unmapped.
//
//     Revision 1.3  2002/07/17 14:17:43  lars
//     Added CVS Header
//     
//
#include "Python.h"
//#include <stdio.h>
//#include <stdlib.h>

#include <fcntl.h> // for open operation, etc...
#include <unistd.h> // for close, write, read...
//#include "../driver2.2/ociwpci/ociw.h"
#include "../ociwpci/ociw.h"
#include <sys/ioctl.h>
#include <sys/mman.h> // for memory map operation.

static char ociwdoc[] = "Physical (lowest level) interface to the OCIW PCI device driver.\n\
\n\
Implements a file-like object for communicating with the\n\
electronics as a device at the operating system level.\n\
This module *should* handle opening the device and returning a \n\
handle to the caller. If there are several devices in the system,\n\
it will open one and pass that handle back.\n\
The current implementation assumes the device is a singleton \n\
and keeps a single static file descriptor internally \n\
the device driver itself also makes a singleton assumption. \n\
Some of the ioctl and memmap calls here may be doable right from python. \n\
If this is the case, that approach would be preferable." ;

static int  ociw_fd = 0L;              /* device */
static char *ociwbase = 0;  //Beginning of dest image. indexed in bytes.
//static short nRows, nCols, nSamps, Sampmode; //
static long physbufsize = 0; // total physical buffer size, in bytes, to allocate for mmap
static long imgsize = 0 ; // size, in bytes, of destination images
        
static char ociw_open_doc[] =
"open(devicename) - devicename is a string. \n\
if device opens, query the device for physbufsize (total phys mem) \n\
and imgsize (max image size of dest images). \n\
returns the integer file descriptor that the system open returns.\n\
XXX - should raise an exception if there is a problem opening device.";

static PyObject *
ociw_open(PyObject *self, PyObject *args)
{
    char* devname;

    if (!PyArg_ParseTuple(args, "s", &devname))
      return NULL;

    if (ociw_fd) // open already?
    {
        close(ociw_fd); // close it.
        ociwbase = 0; // closing frees maps, I think...
    }
    ociw_fd = open( devname, O_RDWR ); // now reopen.
    if (ociw_fd <=0) { // opened ok?
        ociw_fd = 0; // no file open.
        return PyErr_Format(PyExc_IOError,"Error opening %s",devname); 
    }
    else {
        printf("Device %s opened \n",devname);
    }

    // get the constants it was installed with...
    ioctl(ociw_fd, OCIW_GET_IMAGESIZE, &imgsize); //
    ioctl(ociw_fd, OCIW_GET_PHYSBUFSIZE, &physbufsize); //
    printf("python phys_bufsize: 0x%08lx \n", physbufsize);

    // map the whole thing to our space. if not mapped already.
    // (unmap if it is??)
    if(ociwbase == 0) 
        ociwbase = (char *) mmap(0,physbufsize,PROT_READ|PROT_WRITE,MAP_SHARED,ociw_fd,0);
    if (ociwbase == MAP_FAILED)
    {
        ociwbase = 0;
        return PyErr_Format(PyExc_MemoryError,"Mapping Failed"); 
    }
    else
    {
        printf("python mapped ok, address= %08lx \n", (unsigned long)ociwbase );
    }

    return Py_BuildValue("i", ociw_fd); // useful?
    /*
    Py_INCREF(Py_None);
    return Py_None;
    */
}
    
static char ociw_close_doc[] = "\
close() - unmap the memory and close the file descriptor. \n\
XXX seems to unmap without checking to see if a mapping exists. \n\
XXX check on multiple clients, multiple mappings.. possible? \n\
also returns the file descriptor. (Probably not useful.) \n\
";
static PyObject* 
ociw_close(PyObject* self, PyObject* args)
{
    int unmap = munmap( ociwbase, physbufsize ); // needed?
    printf("Results from unmap: %d\n", unmap);

    if (ociw_fd > 0) 
    {
        close(ociw_fd);
        ociw_fd = 0;
        ociwbase = 0;
        printf("Device /dev/ociw0 closed\n");
    }
    return Py_BuildValue("i", ociw_fd); // useful?
}


static PyObject* 
ociw_reset(PyObject* self, PyObject* args)
{
    ioctl(ociw_fd, OCIW_RESET);  // reset the whole board

    Py_INCREF(Py_None);
    return Py_None;
}

// Turn interrupts on
static PyObject *
ociw_start(PyObject *self, PyObject *args)
{
    // printf("starting the process\n");
    ioctl(ociw_fd, OCIW_INTS_ON);
    Py_INCREF(Py_None);
    return Py_None;
}

// 
static PyObject *
ociw_stop(PyObject *self, PyObject *args)
{
    // might leave interrupts on when we don't
    // want them to be on. 
    printf("stopping the process\n");
    //ioctl(ociw_fd, OCIW_INTS_ON);
    Py_INCREF(Py_None);
    return Py_None;
}
    

// Set the destination image number 
static PyObject *
ociw_set_imagenum(PyObject *self, PyObject *args)
{
    int num;

    if (!PyArg_ParseTuple(args, "i", &num))
        return NULL;
        
    ioctl(ociw_fd, OCIW_SET_IMAGE, &num);

    Py_INCREF(Py_None);
    return Py_None;
}

// Set the image mode (sample mode??)
static PyObject *
ociw_set_imagemode(PyObject *self, PyObject *args)
{
    int mode;

    if (!PyArg_ParseTuple(args, "i", &mode))
        return NULL;
        
    ioctl(ociw_fd, OCIW_SET_IMAGEMODE, &mode);

    Py_INCREF(Py_None);
    return Py_None;
}

// really, the driver only needs to know npix.
static PyObject *
ociw_set_rcs(PyObject *self, PyObject *args)
{
    int row, col, nsamp;
    
    if (!PyArg_ParseTuple(args, "iii", &row, &col, &nsamp))
        return NULL;
        
   ioctl(ociw_fd, OCIW_SET_NROWS, &row);
   ioctl(ociw_fd, OCIW_SET_NCOLS, &col);
   ioctl(ociw_fd, OCIW_SET_NSAMPS, &nsamp);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
ociw_write(PyObject *self, PyObject *args)
{
	int inval;
    unsigned short value;
    
    if (!PyArg_ParseTuple(args, "i", &inval))
        return NULL;
	    
	value = inval&0xffff;

    (void) write(ociw_fd, &value, 2);

    Py_INCREF(Py_None);
    return Py_None;
}

// read 1 word (16 bits) from FIFO.
static PyObject *
ociw_read(PyObject *self, PyObject *args)
{
    short buf; 
    
    if( read(ociw_fd, &buf, 2)!= 2)
        return NULL;

    return Py_BuildValue("i", buf); // size? lifetime? `
}

static char ociw_data24_doc[] = "\
data24(intdata) - send the 24 LSBs of the int to the dsp. \n\
Performs 3 physical 16 bit writes on the serial cable.\n\
The first sends intdata[23..16] in the high byte and 1 in the lo byte. \n\
The second sends intdata[15..8] in the high byte and 2 in the lo byte.\n\
The last sends intdata[7..0] in the high byte and 3 in the lo byte. \n\
The DSP receives these words and interprets the 1, 2, and 3 as commands. \n\
It reassembles the 24 bit value and saves it for use on a subsequent command.\
";

static PyObject *
ociw_data24(PyObject *self, PyObject *args)
{
    unsigned int data;
    short word[3];
    
    if (!PyArg_ParseTuple(args, "i", &data))
        return NULL;

    word[0] = ((data >> 8) & 0xFF00) | 0x0001;  // most sig byte first.
    word[1] = (data & 0xFF00)  | 0x0002; // 2 is next sig byte command.
    word[2] = ((data << 8) & 0xFF00) | 0x0003;  // least sig word last.

    (void) write(ociw_fd, word, 6);
    
    Py_INCREF(Py_None);
    return Py_None;
}


static char ociw_data16_doc[] = "\
data16(intdata) - send the 16 LSBs of the int to the dsp. \n\
Performs 3 physical 16 bit writes on the serial cable.\n\
The first sends 0 in the high byte and 1 in the lo byte. \n\
The second sends intdata[15..8] in the high byte and 2 in the lo byte.\n\
The last sends intdata[7..0] in the high byte and 3 in the lo byte. \n\
The DSP receives these words and interprets the 1, 2, and 3 as commands. \n\
It reassembles the 24 bit value and saves it for use on a subsequent command.\
";

// this is linked to DSP code that receives commands..
static PyObject *
ociw_data16(PyObject *self, PyObject *args)
{
    unsigned short data;
    short word[3]; // 3 words to go down
    
    if (!PyArg_ParseTuple(args, "h", &data))
        return NULL;

    word[0] = 0x0001;  // most sig byte first.
    word[1] = (data & 0xFF00)  | 0x0002; // 2 is next sig byte command.
    word[2] = ((data << 8) & 0xFF00) | 0x0003;  // least sig word last.
    
    (void) write(ociw_fd, word, 6);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
ociw_command(PyObject *self, PyObject *args)
{
    unsigned short command;
    unsigned int address;
    short word[3]; // 3 words to go down
    
    if (!PyArg_ParseTuple(args, "hi", &command, &address))
        return NULL;

    word[0] = ((address >> 8) & 0xFF00) | 0x0004;  // most sig byte first.
    word[1] = (address & 0xFF00)  | 0x0005; // 2 is next sig byte command.
    word[2] = ((address << 8) & 0xFF00) | command;  // least sig word last.
    (void) write(ociw_fd, word, 6);

    Py_INCREF(Py_None);
    return Py_None;
}


// useless to python.
static PyObject* 
ociw_pixtogo(PyObject* self, PyObject* args)
{
    long pixtogo;
    #ifdef DEBUG
        puts("inside ociwmodules ociw_pixtogo()");
    #endif

    ioctl(ociw_fd, OCIW_GET_DONE, &pixtogo);
    return Py_BuildValue("l", pixtogo);
}

// actually, this is the only call needed to get a buffer back.
// the other functions nearby to get a buffer object were never used.
static PyObject *
ociw_get_buf(PyObject *self, PyObject * args)
{
    int bufnum = 0;

    if (!PyArg_ParseTuple(args, "i", &bufnum)) // integer? destination.
        return NULL;
    
    if (bufnum == -1) // negative 1? map it all.
        return PyBuffer_FromReadWriteMemory(ociwbase,physbufsize);
    else
        return PyBuffer_FromReadWriteMemory(ociwbase+bufnum*imgsize,imgsize);
}

// 
static PyObject *
ociw_get_raw_buf(PyObject *self, PyObject * args)
{
    int bufnum = 0;
    int pixperbuf = imgsize/2;
    if (!PyArg_ParseTuple(args, "i|i", &bufnum, &pixperbuf )) // 
        return NULL;
    
    if (bufnum >= 0 ) // raw buffer??
            // the 6 in the next lines mean
            // 2 32 bit images plus 2 16 bit images.
            // it would be better to query the driver.
        return PyBuffer_FromReadWriteMemory(
                        ociwbase + 6*imgsize + bufnum*pixperbuf*2,pixperbuf*2);
    else 
        return PyBuffer_FromReadWriteMemory(
                        ociwbase + 6*imgsize, physbufsize - 6*imgsize);
}

// Initialize the DSP with a srec file
// pass it the file name as a string.
static PyObject* ociw_load_srec(PyObject* self, PyObject* args)
{
    extern int dsp_init(char*, int);
    extern void dsp_reset(int);
    char* srec;
    int sts = 0;
	int doreset = 1;
    
    #ifdef DEBUG
        puts("inside sload's init()");
    #endif

    if (!PyArg_ParseTuple(args, "s|i", &srec, &doreset))
      return NULL;
    
    if ( ociw_fd)
    {
	if (doreset)
    	dsp_reset(ociw_fd);
    sts = dsp_init(srec, ociw_fd);
    if (sts <= 0) // failed the first time?
        sts = dsp_init(srec, ociw_fd); // try again!
    }

    return Py_BuildValue("i", sts);
}
    

static PyMethodDef OciwMethods[] = {
  {"open", ociw_open, METH_VARARGS, ociw_open_doc},
  {"close", ociw_close, METH_VARARGS, ociw_close_doc},
  {"reset", ociw_reset, METH_VARARGS, "issue the magic HSS reset pattern"},
  {"start", ociw_start, METH_VARARGS, "Start acquiring image."},
  {"stop", ociw_stop, METH_VARARGS, "Stop (abort condition)"},
  {"load_srec", ociw_load_srec, METH_VARARGS, "Initialize the dsp with an s-record file"},
  {"set_rcs", ociw_set_rcs, METH_VARARGS, "set_rcs(nrow,ncol,nsamp)"},
  {"write", ociw_write, METH_VARARGS, "Write low 16 bits of int to the DSP"},
  {"read", ociw_read, METH_VARARGS, "Read one 16 bit value from the FIFO, return in low 16 bits of int"},
  {"data24", ociw_data24, METH_VARARGS, ociw_data24_doc},
  {"data16", ociw_data16, METH_VARARGS, ociw_data16_doc},
  {"command", ociw_command, METH_VARARGS, "Issue 8 bit commandi with 24 bit arg"},
  {"get_buf", ociw_get_buf, METH_VARARGS, "Get python buffer object to access cooked image data.\n\npass 0 or 1 to get a source or background buffer.\n-1 maps all memory."},
  {"get_raw_buf", ociw_get_raw_buf, METH_VARARGS, "Get python buffer object to access raw image data.\n\npass it which raw buffer you want,\nand the size (in pixels) of a raw buffer (usually nrows*ncols)"},
  // {"get_imagesize", ociw_get_imagesize, METH_VARARGS, "Get the physical buffer size"},
  {"pixtogo", ociw_pixtogo, METH_VARARGS, "Get the number of pixels left to go"},
  {"setimagemode", ociw_set_imagemode, METH_VARARGS, "Set the image mode for the copyfowler routine"},
  {"setimagenum", ociw_set_imagenum, METH_VARARGS, "Set the destination image number."},
  {NULL,NULL,0,NULL}

};


void
initociw(void)
{
    Py_InitModule3("ociw", OciwMethods, ociwdoc);
}
