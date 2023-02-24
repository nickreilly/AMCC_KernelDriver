/* Copyright (C) 1990, 1995 by Motorola, Inc. */

/* NOTE - NOTE - NOTE - NOTE - NOTE - NOTE - NOTE - NOTE - NOTE - NOTE
   The functions described in this header file are currently unimplemented!
   You may wish to implement them on your custom hardware. If you do so,
   please make sure that such custom implementations conform to the
   definitions in this file, for the sake of future compatability. */

#if ! defined( __TIME )
#define __TIME

#if ! defined( NULL )
#define NULL ((void*)0)
#endif

#if defined( ROMAN_ROBLES_CLOCK )
/* this is for an ADS board running at 20Mhz equipped with Roman's 4 PAL
   clock. */
#define CLOCKS_PER_SEC 20000000
#else
/* this is for a simulated 40Mhz part. */
#define CLOCKS_PER_SEC 40000000
#endif

#if ! defined( __SIZE_T )
typedef unsigned int size_t;
#define __SIZE_T
#endif

typedef unsigned long clock_t;
typedef unsigned long time_t;

struct tm
{
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
    int tm_wday;
    int tm_yday;
    int tm_isdst;
};

#if defined( ROMAN_ROBLES_CLOCK )
/* this is for an ADS board running at 20Mhz equipped with Roman's 4 PAL
   clock. __clock_reset ( ) is used to reset this clock. it is *not* an
   ANSI C function Do not rely on its continued existence.
 */
void __clock_reset ( );
#endif

clock_t clock ( void );
double difftime ( time_t, time_t );
time_t mktime ( struct tm* );
time_t time ( time_t* );
char* asctime ( const struct tm* );
char* ctime ( const time_t* );
struct tm* gmtime ( const time_t* );
struct tm* localtime ( const time_t* );
size_t strftime ( char*, size_t, const char*, const struct tm* );

#endif
