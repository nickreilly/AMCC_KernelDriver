#define EXTERN extern

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>

#include "ir2.h"

/*---------------------------------------------------------
**  strxcat() - similar to strlcpy in BSD. Or limits the 
**    number of chars that can be append to dest using
**    dest_size, typical usage is:
**       strxcat( buf, "new segment", sizeof(buf));
**---------------------------------------------------------
*/
char * strxcat( char * dest, char *src, int dest_size)
{
   int n = dest_size - strlen(dest) - 1;   // limit to prevent buffer overflow.
   return strncat( dest, src, n);
}

