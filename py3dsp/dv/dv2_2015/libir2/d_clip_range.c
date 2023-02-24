#define EXTERN extern

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
 
#include "ir2.h"

/*---------------------------------------------------------------------------
** d_clip_range( ) - makes sure d in within min---max 
**----------------------------------------------------------------------------
*/
double d_clip_range( double d, double min, double max )
{
   if( d < min ) d = min;
   if( d > max ) d = max;
   return d;
}

