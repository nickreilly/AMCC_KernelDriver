#define EXTERN extern

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>

#include "ir2.h"

/*---------------------------------------------------------
**  strxcpy() - similar to strncpy, but will always
**       place an null character at the end of dest.
**---------------------------------------------------------
*/
char * strxcpy
(
   char * dest,
   const char * src,
   int  maxlen 
)
{
   strncpy( dest, src, maxlen );
   dest[maxlen-1] = '\0';
   return dest;
}

