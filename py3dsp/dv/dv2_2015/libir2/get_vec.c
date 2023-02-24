
#define EXTERN extern

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
 
#include "ir2.h"

/*----------------------------------------------------------------
**   get_vector() - determines direction and mag from start
**      to dest. This function assumes circular coordinate system
**      from 0 to size.
**----------------------------------------------------------------
*/
void get_vector(
   int * dir,       /* direction to move    */
   double *mag,     /* magnitude of move    */
   double start,    /* current location     */
   double dest,     /* location to move to  */
   double size      /* size of 1 revolution */
)
{
   int    d;
   double m;
 
   start = drange( start, 0, size);
   dest  = drange( dest, 0, size);
 
   d = 1;       /* for increasing step position */
   m = dest - start;
 
   /* if s is negative reverse direction */
   if( m < 0 )
   {
      d = -d;
      m = -m;
   }
 
   if( m > size/2 )
   {
      d = -d;
      m = size - m;
   }
 
   *dir = d;
   *mag = m;
}

