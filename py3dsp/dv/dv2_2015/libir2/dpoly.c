#define EXTERN extern

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
 
#include "ir2.h"

/*----------------------------------------------------------
** dpoly() -  calculates y = a + bx + cx^2 + dx^3 + ...
**----------------------------------------------------------
*/
double dpoly( 
   double x,           /* The x value                    */
   double *coeff,      /* point to array of coefficients */
   int    degree       /* maximum degree of polynimal    */
)
{
   int i;
   double d, y;

   y = 0;
   d = 1;
   for(i=0; i<=degree; i++)
   {
      y += (*coeff++)* d;
      d *= x; 
   }
   return y;
}

