/*********************************************************************************
**  df_stats.c
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
#include <stdio.h>
#include <stdint.h>
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

extern struct df_internal_vars DFOptions;
 
/*-------------------------------------------------------------------------
**  df_stats() - Calculates the min, max, mean, and stddev of the pixels.
**      Each of the 2 passes is done via the _p1() & _p2 functions. If you
**      already know the mean, just use _p2().
**-------------------------------------------------------------------------
*/

int df_stats ( struct df_buf_t * bufp)
{
   int rc;

#if FUN_NAME
	printf(">df_stats()\n");
#endif

   if( (rc = df_stats_p1( bufp )) != ERR_NONE )
      return rc;
   rc = df_stats_p2( bufp, bufp->mean );
   return rc;
}

/*------------------------------------------------------------------
**  df_stats_p1() - pass1. Calculates the min, max, mean, 
**                  of the pixels.
**------------------------------------------------------------------
*/

int df_stats_p1 ( struct df_buf_t * bufp)
{
	float * dataptr;
	float  max, min, fdata;
	double sum;
	int i;

#if FUN_NAME
	printf(">df_stats_p1()\n");
#endif

   /* Initialize values */
   bufp->min    = DF_MAX_SIGNED_INT32;
   bufp->max    = DF_MIN_SIGNED_INT32;
   bufp->mean   = 0;

	if( bufp->N <= 0 )
      return ERR_NONE;
 
   /* Calculate mean */
   min    = DF_MAX_SIGNED_INT32;
   max    = DF_MIN_SIGNED_INT32;
	sum = 0;
	dataptr = &bufp->fdata[0];
	for( i=0; i < bufp->N; i++)
	{
		/* Determine the minimum and max */
		fdata = *dataptr++;
		if( DFOptions.divbycoadd )
			fdata /= bufp->divisor;
		if( max < fdata ) max = fdata;
		if( min > fdata ) min = fdata;

		/* Calculate the standard deviation */
		sum += fdata;
	}
	bufp->mean = sum / bufp->N;
   bufp->min    = min;
   bufp->max    = max;
   return ERR_NONE;
}
 
/*------------------------------------------------------------------
**  df_stats_p2() - pass2. Given the mean, the min, max, and stddev
**                  of the pixels.
**------------------------------------------------------------------
*/

int df_stats_p2
(
	struct df_buf_t * bufp,       /* Buffer struct to hold the data. */
	double mean                   /* mean of data points    */
)
{
	float * dataptr;
	double fdata, sum;
	int i;

#if FUN_NAME
	printf(">df_stats_p2()\n");
#endif

   /* Initialize values */
   bufp->mean   = mean;
   bufp->stddev = 0;

	if( bufp->N <= 1 )
      return ERR_NONE;
 
   /* Calculate Standard Deviation and Min/Max */
	sum = 0;
	dataptr = &bufp->fdata[0];
	for( i=0; i < bufp->N; i++)
	{
		fdata = *dataptr++;
		if( DFOptions.divbycoadd )
			fdata /= bufp->divisor;

		/* Calculate the standard deviation */
		fdata = fdata - bufp->mean;
		sum += fdata * fdata;
	}
	bufp->stddev = sqrt( sum / (bufp->N -1));

   return ERR_NONE;
}
 

