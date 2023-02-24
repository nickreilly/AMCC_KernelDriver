/*********************************************************************************
**  stats.c - help with statistics
**            d_stats_p1/p2() - double  based functions.
**            l_stats_p1/p2() - int32_t based functions.
********************************************************************************
*/
#define EXTERN extern
 
/*-------------------------
**  Standard include files
**-------------------------
*/
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#include "ir2.h"

 
/*---------------------------------------------------------------
** d_stats_p1() -  stats pass 1. Find min, max, mean.
**---------------------------------------------------------------
*/
void d_stats_p1(
   double *r_min,    // return min value
   double *r_max,    // return max value
   double *r_mean,   // return the mean
   double *data,     // data array
   const int n      // number of elements in array
)
{
   int i;
   double min, max;
   double sum;

   min = data[0];
   max = data[0];
   sum = 0;
   for( i=0; i<n; i++ )
   {
      if( min > data[i] ) min = data[i];
      if( max < data[i] ) max = data[i];
      sum += data[i];
   }

   *r_min  = min;
   *r_max  = max;
   *r_mean = sum / n;
}

/*---------------------------------------------------------------
** d_stats_p2() -  stats pass 2. Given the mean, find the std dev.
**---------------------------------------------------------------
*/

void d_stats_p2(
   double * r_std,    // return std
   double * data,     // array of data value
   const int n,       // number of data values
   const double mean  // mean value
)
{
   int i;
   double d, sum, std;

   std = 0;
   sum = 0;

   for( i=0; i<n; i++ )
   {
      d = data[i] - mean;
      sum += (d*d);
   }

   if( n > 1 )
      std = sqrt( sum / (n-1));

   *r_std = std;
}

/*-----------------------------------------------------------------
** l_stats_p1() - statistics pass 1 - get min, max, mean.
**-----------------------------------------------------------------
*/
void l_stats_p1( int32_t * Rmin, int32_t *Rmax, double *Rmean, int32_t * addr, int n)
{
   int i;
   int32_t min, max;
   double mean;

   min = 0x7fffffff;
   max = 0x80000001;
   mean = 0;

   for( i=0; i<n; i++)
   {
      int32_t l;
      l = *addr++;
      if( l > max ) max = l;
      if( l < min ) min = l;
      mean += l;
   }
   mean /= (double)n;

   *Rmin  = min;
   *Rmax  = max;
   *Rmean = mean;
}  
   
/*-----------------------------------------------------------------
** stats_p2()- statistics pass 2 - get std, but needs mean.
**-----------------------------------------------------------------
*/
void l_stats_p2( double *Rstd, int32_t * addr, int n, double mean)
{  
   int i;
   double std;

   std = 0;
   for( i=0; i<n; i++)
   {
      double d;

      d = (double)(*addr++) - mean;
      std += d*d;
   }
   std = sqrt( std / (n-1));

   *Rstd = std;
}

