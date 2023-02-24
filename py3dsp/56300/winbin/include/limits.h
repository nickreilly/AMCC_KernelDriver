/* Copyright (C) 1990, 1995 by Motorola, Inc. */

#if ! defined( __LIMITS )
#define __LIMITS

#if defined( __DSP561C__ ) || defined( __DSP566C__ )

/* the 561c supports 16 bit chars and ints, and 32 bit longs. */

#define	CHAR_BIT		16
#define	SCHAR_MIN		(-32767)
#define	SCHAR_MAX		32767
#define	UCHAR_MAX		0xffff
#define	MB_LEN_MAX		2
#define	LONG_MIN		(-2147483647)
#define	LONG_MAX		2147483647
#define	ULONG_MAX		0xffffffff

#elif defined( __DSP56K__ ) || defined( __DSP563C__ )

/* the 56k supports 24 bit chars and ints, and 48 bit longs. */

#define	CHAR_BIT		24
#define	SCHAR_MIN		(-8388607)
#define	SCHAR_MAX		8388607
#define	UCHAR_MAX		0xffffff
#define	MB_LEN_MAX		3
#define	LONG_MIN		(-((8388607*16777216)+16777215))
#define	LONG_MAX		((8388607*16777216)+16777215)
#define	ULONG_MAX		0xffffffffffff

#elif defined( __DSP96K__ )

/* the 96k supports 32 bit chars, ints, and longs. */

#define CHAR_BIT		32            
#define SCHAR_MIN		(-2147483647)  
#define SCHAR_MAX        	2147483647    
#define UCHAR_MAX		0xffffffff
#define	MB_LEN_MAX		4
#define LONG_MIN		SCHAR_MIN
#define LONG_MAX		SCHAR_MAX
#define ULONG_MAX		UCHAR_MAX

#endif

/* COMMON */

/* chars are, by default, signed */

#define	CHAR_MIN		SCHAR_MIN
#define	CHAR_MAX		SCHAR_MAX
#define	SHRT_MIN		SCHAR_MIN
#define	SHRT_MAX		SCHAR_MAX
#define	USHRT_MAX		UCHAR_MAX
#define	INT_MIN			SCHAR_MIN
#define	INT_MAX			SCHAR_MAX
#define	UINT_MAX		UCHAR_MAX

#endif
