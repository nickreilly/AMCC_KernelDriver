
/*********************************************************************************
**  df_init.c
********************************************************************************
*/
#define EXTERN extern
 
/*-------------------------
**  Standard include files
**-------------------------
*/
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <math.h>
#include <sys/param.h>
#include <malloc.h>
#include <memory.h>
 
/*------------------------------
**  Non-standard include files
**------------------------------
*/
 
#include "ir2.h"
 
struct df_internal_vars DFOptions;

#define EXTRA_CHECKING 1
#define VERBOSE 0
 
/*------------------------------------------------------------------
** df_init() - This function should be called in any program using
**     the df lib, before any other df_function.
**------------------------------------------------------------------
*/
int df_init( void )
{
   DFOptions.divbycoadd = FALSE;
   DFOptions.origin     = DF_ORIGIN_TL;
   return ERR_NONE;
}
 
int df_print_options( void )
{
   printf("DFOptions.divbycoadd = %d \n", DFOptions.divbycoadd ); 
   printf("DFOptions.origin     = %d \n", DFOptions.origin     ); 
	return ERR_NONE;
}

/*------------------------------------------------------------------
** dfset_divbycoadd() - Set the divbycoadd option.
**------------------------------------------------------------------
*/
int dfset_divbycoadd( int divbycoadd )
{
   if( divbycoadd )
		DFOptions.divbycoadd = TRUE;
   else
      DFOptions.divbycoadd = FALSE;
   return ERR_NONE;
}

/*------------------------------------------------------------------
** dfget_divbycoadd() - GEt the divbycoadd option.
**------------------------------------------------------------------
*/
int dfget_divbycoadd( void )
{
	return DFOptions.divbycoadd;
}

/*------------------------------------------------------------------
** dfset_origin - sets where (0,0) is.
**------------------------------------------------------------------
*/
int dfset_origin( int origin )
{
   if ( origin == DF_ORIGIN_TL )
		DFOptions.origin = DF_ORIGIN_TL;
   if ( origin == DF_ORIGIN_BL )
		DFOptions.origin = DF_ORIGIN_BL;
   return ERR_NONE;
}

/*----------------------------------------------------------
**  dfdatarc()  - returns the value at data[m][n]  (row & column)
**  dfdataxy()  - returns the value at data[x,y];
**  dfdatainx() - returns the value at data[inx];
**
**  DFOption.origin will transpost the axis reference if necessary.
**  DFOption.divbycoadd will divide the data if necessary.
**  double df_data_f( ) - is a helper function, and should
**    not be used by other functions.
**----------------------------------------------------------
*/

double dfdatamn( struct df_buf_t * bufp, int m, int n )
{
   int i;

   if( DFOptions.origin == DF_ORIGIN_BL )
	{
		/* reverse the y axis */
		m = bufp->naxis2 - m - 1;
	}
	i = m * bufp->naxis1 + n;

#if VERBOSE
   printf("dfdatamn() m=%d n=%d i=%d\n", m, n, i);
#endif
	return df_data_f( bufp, i );
}

double dfdataxy( struct df_buf_t * bufp, int x, int y )
{
   int i;

   if( DFOptions.origin == DF_ORIGIN_BL )
	{
		/* reverse the y axis */
		y = bufp->naxis2 - y - 1;
	}
	i = y * bufp->naxis1 + x;

#if VERBOSE
   printf("dfdataxy() x=%d y=%d i=%d\n", x, y, i);
#endif
	return df_data_f( bufp, i );
}

double dfdatainx( struct df_buf_t * bufp, int i )
{
   if( DFOptions.origin == DF_ORIGIN_BL )
	{
		/* extract (x,y) from index */
		int x = i % bufp->naxis1;  
		int y = i / bufp->naxis1;

		/* transpost the y axis */
		y = bufp->naxis2 - y - 1;

		/* recalcuate i */
	   i = y * bufp->naxis1 + x;
#if VERBOSE
      printf("dfdatainx() x=%d y=%d i=%d\n", x, y, i);
#endif
	}
	return df_data_f( bufp, i );
}

double df_data_f( struct df_buf_t * bufp, int i )
{
   double d;

#if EXTRA_CHECKING
   if( (i < 0) || (i >= bufp->N) )
	{
		d = 0;
#if VERBOSE
		printf("df_data_f() inx %d is out of range! [0..%ld]\n", i, bufp->N-1);
#endif
	}
#endif
	d = bufp->fdata[i];

	/* apply divbycoadd */
	if( DFOptions.divbycoadd ) 
		d /= bufp->divisor;

#if VERBOSE
	printf("df_data_f() i=%d d=%f\n", i, d);
#endif
   return d;
}

