#define EXTERN extern

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>

#include "ir2.h"

/*----------------------------------------------------------
**  stricmp() - case insentive version of strcmp();
**    make obsolete by unix's: strcasecmp(), strncasecmp().
**----------------------------------------------------------
*/
int stricmp( char * s, char * t)
{
   register char sc, tc;
   while(1)
   {
      sc = ( islower( (int)*s ) ? toupper( (int)*s ) : *s);
      tc = ( islower( (int)*t ) ? toupper( (int)*t ) : *t);
      if( sc != tc )
         break;
      if( sc == '\0' || tc == '\0' )
         break;
      s++;  t++;
   }
   return ( sc  - tc );
}

