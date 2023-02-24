/* (C) 1992 by Motorola, Inc. */

/* ioprim.h - 
   This file is unique among all of the include files in this package. 
   It is intended to be included both by programs running on the dsp and 
   by programs running on the host. At the lower levels, the host and dsp
   must agree on certain definitions in order for communication to be 
   reliable. We've concentrated all of those definitions into this one file.
   We're hoping that the locality factor will keep things consistent between
   the two sets of definitions. 

   Dsp side I/O rountines should *not* include this file directly. Just:
   #include <stdio.h>
   #include <errno.h>
   and you'll get everything.

   Host side I/O routines *should* include this file directly:
   #include "/usr/local/dsp/include/ioprim.h"
   and you'll get everything.

   This is a natural place to document the low level I/O protocol between
   the host and the dsp, so here goes (each [] denotes a separate message:

   OPEN:
   to host: [ DSP_OPEN, flags, mode, path length ]
   to host: [ path ... ]
   to dsp:  [ return value, errno ]

   CLOSE:
   to host: [ DSP_CLOSE, descriptor ]
   to dsp:  [ return value, errno ]

   READ:
   to host: [ DSP_READ, descriptor, count ]
   to dsp:  [ data ... ]
   to dsp:  [ return value, errno ]

   WRITE:
   to host: [ DSP_WRITE, descriptor, count ]
   to host: [ data ... ]
   to dsp:  [ return value, errno ]

   LSEEK:
   to host: [ DSP_LSEEK, descriptor, offset, whence ]
   to dsp:  [ return value, errno ]

   UNLINK:
   to host: [ DSP_UNLINK, path length ]
   to host: [ path ... ]
   to dsp:  [ return value, errno ]

   RENAME:
   to host: [ DSP_RENAME, longest path length ]
   to host: [ old path ... ]
   to host: [ new path ... ]
   to dsp:  [ return value, errno ]

   ACCESS:
   to host: [ DSP_ACCESS, mode, path length ]
   to host: [ path ... ]
   to dsp:  [ return value, errno ]

   */

#if defined( __DSP561C__ ) || defined( __DSP56K__ ) || defined( __DSP96K__ )  || defined( __DSP563C__ ) || defined( __DSP566C__ )

/* this section is intended to be included by the dsp I/O code. please
   note that all definitions must be kept consistent with the following 
   section. */

#if defined( __GET_ERRNO )

#define	EDOM		1
#define	ERANGE		2
#define	ENOMEM		3 /* a dynamic memory allocation function was unable
			     to allocate the requested memory. */
#define EBADFORMAT      4 /* a string to numeric conversion routine was passed
			     an argument with an invalid format. */
#define ENOFILEDESC     5 /* fopen() exceeded FOPEN_MAX open files */
#define ENOENT          6 /* No such file or directory */
#define EACCES          7 /* access denied because of file permission */
#define EBADF           8 /* bad file number */
#define EINVAL          9 /* invalid argument */
#define ESPIPE         10 /* illegal seek */
#define ESETPOS        11 /* fsetpos failure */

#elif defined( __GET_STDIO )

#define FOPEN_MAX 8

/* __open( ) flag values */
#define	__O_RDONLY	0x0000		/* open for reading */
#define	__O_WRONLY	0x0001		/* open for writing */
#define	__O_RDWR	0x0002		/* open for read & write */
#define	__O_APPEND	0x0004		/* append on each write */
#define	__O_CREAT	0x0008		/* open with file create */
#define	__O_TRUNC	0x0010		/* open with truncation */
#define	__O_BINARY	0x0020

/* __lseek() whence values */
#define __L_SET         0       	/* absolute offset */
#define __L_INCR        1       	/* relative to current offset */
#define __L_XTND        2       	/* relative to end of file */

/* __access() mode values */
#define __F_OK          1               /* does file exist */
#define __X_OK          2               /* is it executable by caller */
#define __W_OK          4               /* writable by caller */
#define __R_OK          8               /* readable by caller */

enum io_commands
{
    DSP_OPEN,
    DSP_CLOSE,
    DSP_READ,
    DSP_WRITE,
    DSP_LSEEK,
    DSP_UNLINK,
    DSP_RENAME,
    DSP_ACCESS
};

void __send( const void*, int );
void __recv( void* );

#endif /* __GET_ERRNO / __GET_STDIO */

#else /* DSPX6XXX */

/* this section is intended to be included by the host I/O code. please
   note that all definitions must be kept consistent with the preceding
   section. */

#define DSP_EDOM	 1
#define DSP_ERANGE	 2
#define DSP_ENOMEM 	 3 /* a dynamic memory allocation function was unable
			      to allocate the requested memory. */
#define DSP_EBADFORMAT   4 /* a string to numeric conversion routine was passed
			      an argument with an invalid format. */
#define DSP_ENOFILEDESC  5 /* fopen() exceeded FOPEN_MAX open files */
#define DSP_ENOENT       6 /* No such file or directory */
#define DSP_EACCES       7 /* access denied because of file permission */
#define DSP_EBADF        8 /* bad file number */
#define DSP_EINVAL       9 /* invalid argument */
#define DSP_ESPIPE      10 /* illegal seek */
#define DSP_ESETPOS     11 /* fsetpos failure */

#define DSP_FOPEN_MAX 8

/* __open( ) flag values */
#define	DSP_O_RDONLY	0x0000		/* open for reading */
#define	DSP_O_WRONLY	0x0001		/* open for writing */
#define	DSP_O_RDWR	0x0002		/* open for read & write */
#define	DSP_O_APPEND	0x0004		/* append on each write */
#define	DSP_O_CREAT	0x0008		/* open with file create */
#define	DSP_O_TRUNC	0x0010		/* open with truncation */
#define	DSP_O_BINARY	0x0020

/* __lseek() whence values */
#define DSP_L_SET       0       	/* absolute offset */
#define DSP_L_INCR      1       	/* relative to current offset */
#define DSP_L_XTND      2       	/* relative to end of file */

/* DSP_access() mode values */
#define DSP_F_OK        1               /* does file exist */
#define DSP_X_OK        2               /* is it executable by caller */
#define DSP_W_OK        4               /* writable by caller */
#define DSP_R_OK        8               /* readable by caller */

enum io_commands
{
    DSP_OPEN,
    DSP_CLOSE,
    DSP_READ,
    DSP_WRITE,
    DSP_LSEEK,
    DSP_UNLINK,
    DSP_RENAME,
    DSP_ACCESS
};

#endif /*DSPX6XXX */
