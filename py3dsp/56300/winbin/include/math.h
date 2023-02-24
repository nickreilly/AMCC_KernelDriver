/* Copyright (C) 1990, 1995 by Motorola, Inc. */

#if ! defined( __MATH )
#define __MATH

#define	HUGE_VAL		1E38

extern double acos ( double );
extern double asin ( double );
extern double atan ( double );
extern double atan2 ( double, double );
extern double cos ( double );
extern double sin ( double );
extern double tan ( double );
extern double cosh ( double );
extern double sinh ( double );
extern double tanh ( double );
extern double exp ( double );
extern double frexp ( double, int* );
extern double ldexp ( double, int );
extern double log ( double );
extern double log10 ( double );
extern double modf ( double, double* );
extern double pow ( double, double );
extern double sqrt ( double );
extern double ceil ( double );
extern double fabs ( double );
extern double floor ( double );
extern double fmod ( double, double );

#if defined( __DSP96K__ )
#define frexp( a, b ) __builtin_frexp ( a, b )
__inline__ static double frexp ( double a, int* b )
{
    int c;
    double d;
    
    if ( 0.0 == a )
    {
	*b = 0;

	return 0.0;
    }
	
    __asm ( "getexp	%1,%0" : "=d" ( c ) : "f" ( a ));
    __asm ( "fgetman	%1,%0" : "=f" ( d ) : "f" ( a ));
    *b = c + 1;
    
    return d * 0.5;
}
#endif

#if defined( __DSP96K__ ) 
#define modf( a, b ) __builtin_modf ( a, b )
__inline__ static double __builtin_modf ( double a, double* b )
{
    double c;
    
    __asm ( "	ftst	%0\n"
	    "	fabs.x	%0	ifal\n"
	    "	floor	%0,%1	ifal\n"
	    "	fneg.x	%1	iflt\n"
	    "	fneg.x	%0	iflt" :
          "=f" ( a ), "=f" ( c ) : "0" ( a ) );

    *b = c;

    return a - c;
}
#endif

#define ceil( a ) __builtin_ceil ( a )
#if defined( __DSP96K__ )
__inline__ static double __builtin_ceil ( double a )
{
    double orig;

    __asm ( "	floor	%0,%0	%0.d,%1.d\n"
	    "	fcmp	%0,%1	#1.0,%1.s\n"
	    "	fadd.x	%1,%0	ffne.u" :
          "=f" ( a ), "=f" ( orig ) : "0" ( a ));
    
    return a;
}
#else
__inline__ static double __builtin_ceil ( double a )
{
    double b;
    
    return ( modf ( a, & b ) > 0.0 ? b + 1.0 : b );
}
#endif

#define fabs( a ) __builtin_fabs ( a )
#if defined( __DSP56K__ ) || defined( __DSP563C__ ) || defined( __DSP566C__ )
__inline__ static double __builtin_fabs ( double a )
{
    return (( 0.0 < a ) ? a : - a );
}
#endif

#define floor( a ) __builtin_floor ( a )
#if defined( __DSP96K__ )
__inline__ static double __builtin_floor ( double a )
{
    double b;
    
    __asm ( "floor	%1,%0" : "=f" ( b ) : "f" ( a ));

    return b;
}
#else
__inline__ static double __builtin_floor ( double a )
{
    double b;
    
    return (( modf ( a, & b ) < 0.0 ) ? b - 1.0 : b );
}
#endif
    
#define fmod( a, b ) __builtin_fmod ( a, b )
__inline static double __builtin_fmod ( double a, double b )
{
    double d;
    
    d = fabs ( a );
    
    if (( d - fabs ( b )) == d )
    {
	return 0.0;
    }
    
    (void) modf ( a / b, & d );

    return ( a - d * b );
}
#endif
