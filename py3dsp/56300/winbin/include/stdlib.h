/* Copyright (C) 1990, 1995 by Motorola, Inc. */

#if ! defined( __STDLIB )
#define __STDLIB

/* TYPEDEF SECTION */

#if ! defined( __SIZE_T )
typedef unsigned int size_t;
#define __SIZE_T
#endif

#if ! defined( __WCHAR_T )
typedef int wchar_t;
#define __WCHAR_T
#endif

typedef struct 
{
    int quot;
    int rem;
}
div_t;

typedef struct
{
    long quot;
    long rem;
}
ldiv_t;

/* MACRO DEFINITION SECTION. */

#if ! defined( NULL )
#define NULL ((void*)0)
#endif

#define	EXIT_FAILURE	(-1)
#define	EXIT_SUCCESS	0

#define	RAND_MAX	32767

#if defined( __DSP561C__ ) || defined( __DSP566C__ )
#define MB_CUR_MAX      2
#elif defined( __DSP56K__ ) || defined( __DSP563C__ )
#define	MB_CUR_MAX	3
#elif defined( __DSP96K__ )
#define	MB_CUR_MAX	4
#endif

double atof ( const char* );
int atoi ( const char* );
long int atol ( const char* );
double strtod ( const char*, char** );
long int strtol ( const char*, char**, int );
unsigned long int strtoul ( const char*, char**, int );
int rand ( void );
void srand ( unsigned int );
void* calloc ( size_t, size_t );
void free ( void* );
void* malloc ( size_t );
void* realloc ( void*, size_t );
void abort ( void );
int atexit ( void (*)( void ) );
void exit ( int );
char* getenv ( const char* );
int system ( const char* );
void* bsearch ( const void*, const void*, size_t, size_t,
	       int (*)( const void*, const void* ) );
void qsort ( void*, size_t, size_t,
	    int (*)( const void*, const void* ) );
int abs ( int );
div_t div ( int, int );
long int labs ( long int );
ldiv_t ldiv ( long int, long int );

int mblen ( const char*, size_t );
#define mblen( s, n ) mbtowc ( NULL, s, n )

int mbtowc ( wchar_t*, const char*, size_t );
int wctomb ( char*, wchar_t );
size_t mbstowcs ( wchar_t*, const char*, size_t );
size_t wcstombs ( char*, const wchar_t*, size_t );

#define abs( a )  __builtin_abs ( a )
#define labs( a ) __builtin_labs ( a )

#endif
