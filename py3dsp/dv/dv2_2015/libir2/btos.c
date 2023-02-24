
#define EXTERN extern

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>

#include "ir2.h"

/*----------------------------------------------------------
** btos( s ) - Create a string of 0's and 1's based on the s.
**              Note: s= char[45] for 32 bit, char[90] for 64bits.
**----------------------------------------------------------
*/
char * btos(
   char * s,            // string buffer.
   unsigned long l,     // data
   int nbits            // sizeof(var) ie, 8, 16, 32 
)
{
   int i;
   unsigned long mask;
   char * cptr;

   mask = (unsigned long)0x0001 << (nbits-1);
   cptr = s;
   for( i=0; i<nbits; i++)
   {
      *cptr++ = ( l & mask ? '1' : '0' );
      if( (i % 4)==3 ) *cptr++ = ' ';
      mask >>= 1;
   }
   *cptr = 0;
   return s;
}

