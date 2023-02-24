#define EXTERN extern

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
 
#include "ir2.h"

/*----------------------------------------------------------
** drange() - Normalizes the value v to be within min & max.
**             Circular coordinate system are assumed, ie:
**             min = max;  max+1=min+1; min-1 = max-1;
**----------------------------------------------------------
*/
double drange
(
	double v,     /* value to check */
	double min,   /* minimum value of v */
	double max    /* maximum value of v */
)
{
   if( v < min )
      return fmod ((v-max), max-min) + max;
   if( v >= max )
      return fmod ((v-max), max-min) + min;
   return v;
}

