/* Copyright (C) 1990, 1995 by Motorola, Inc. */

#if ! defined( __CTYPE )
#define __CTYPE

int isalnum ( int );
int isalpha ( int );
int iscntrl ( int );
int isdigit ( int );
int isgraph ( int );
int islower ( int );
int isprint ( int );
int ispunct ( int );
int isspace ( int );
int isupper ( int );
int isxdigit ( int );
int tolower ( int );
int toupper ( int );

/* special inline versions. */

#define isupper __builtin_isupper
__inline__ static int __builtin_isupper ( int c )
{
    return ( 0x40 < c && 0x5b > c );
}

#define islower __builtin_islower
__inline__ static int __builtin_islower ( int c )
{
    return ( 0x60 < c && 0x7b > c );
}

#define isdigit __builtin_isdigit
__inline__ static int __builtin_isdigit ( int c )
{
    return ( 0x2f < c && 0x3a > c );
}

#define iscntrl __builtin_iscntrl
__inline__ static int __builtin_iscntrl ( int c )
{
#if defined ( __DSP561C__ ) || ( __DSP566C__ )
    return ( 0 == ( 0xffe0 & c ) || 0x7f == c );
#elif defined ( __DSP56K__ ) || defined( __DSP563C__ )
    return ( 0 == ( 0xffffe0 & c ) || 0x7f == c );
#else
    return ( 0 == ( 0xffffffe0 & c ) || 0x7f == c );
#endif
}

#define isgraph __builtin_isgraph
__inline__ static int __builtin_isgraph ( int c )
{
    return ( 0x20 < c && 0x7f > c );
}

#define isprint __builtin_isprint
__inline__ static int __builtin_isprint ( int c )
{
    return ( 0x1f < c && 0x7f > c );
}

#define isalpha __builtin_isalpha
__inline__ static int __builtin_isalpha ( int c )
{
    return ( isupper ( c ) || islower ( c ));
}

#define isalnum __builtin_isalnum
__inline__ static int __builtin_isalnum ( int c )
{
    return ( isalpha ( c ) || isdigit ( c ));
}

#define ispunct __builtin_ispunct
__inline__ static int __builtin_ispunct ( int c )
{
    return ( isgraph ( c ) && ! isalnum ( c ));
}

#define isspace __builtin_isspace
__inline__ static int __builtin_isspace ( int c )
{
    switch ( (char) c )
    {
    case ' ':
    case '\f':
    case '\n':
    case '\r':
    case '\t':
    case '\v':
	return 1;
    }
    return 0;
}

#define isxdigit __builtin_isxdigit
__inline__ static int __builtin_isxdigit ( int c )
{
    return ( isdigit ( c ) || ( 0x40 < c && 0x47 > c ) ||
	    ( 0x60 < c && 0x67 > c ));
}

#define tolower __builtin_tolower
__inline__ static int __builtin_tolower ( int c )
{
    return ( isupper ( c ) ? c + 0x20 : c );
}

#define toupper __builtin_toupper
__inline__ static int __builtin_toupper ( int c )
{
    return ( islower ( c ) ? c - 0x20 : c );
}
#endif
