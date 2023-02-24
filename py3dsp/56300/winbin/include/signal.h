/* Copyright (C) 1990, 1995 by Motorola, Inc. */

#if ! defined( __SIGNAL )
#define __SIGNAL

typedef int sig_atomic_t;

#define SIG_DFL __sig_dfl
#define SIG_ERR __sig_err
#define SIG_IGN __sig_ign

#if defined( __DSP561C__ )

#define SIGABRT		0x30	/* Hardware implementation defined */
#define SIGFPE 		0x33	/* Hardware implementation defined */
#define SIGILL 		0x02	/* Illegal instruction */
#define SIGINT 		0x34	/* Hardware implementation defined */
#define SIGSEGV 	0x36	/* Hardware implementation defined */
#define SIGTERM 	0x38	/* Hardware implementation defined */

#elif defined( __DSP56K__ ) || defined( __DSP563C__ ) || defined( __DSP566C__ )

#define SIGABRT		0x26	/* Hardware implementation defined */
#define SIGFPE 		0x28	/* Hardware implementation defined */
#define SIGILL 		0x3e	/* Illegal instruction */
#define SIGINT 		0x2a	/* Hardware implementation defined */
#define SIGSEGV 	0x2c	/* Hardware implementation defined */
#define SIGTERM 	0x2e	/* Hardware implementation defined */

#elif defined( __DSP96K__ )

#define SIGABRT 	0x100	/* Hardware implementation defined */
#define SIGFPE 		0x102	/* Hardware implementation defined */
#define SIGILL 		0x004	/* Illegal instruction */
#define SIGINT 		0x104	/* Hardware implementation defined */
#define SIGSEGV 	0x106	/* Hardware implementation defined */
#define SIGTERM 	0x108	/* Hardware implementation defined */

#endif

void __sig_dfl ( int );
void __sig_err ( int );
void __sig_ign ( int );

void ( *signal ( int, void (*)( int ))) ( int );
int raise ( int );

#endif
