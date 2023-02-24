#define EXTERN extern

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>

#include "ir2.h"

/*----------------------------------------------------------
** strupr() - converts a string to upper case.
**               Returns: pointer to string.
**----------------------------------------------------------
*/
 
char * strupr( string )
   char * string;
{
   char * s;
   if( string == NULL ) return( string );
   s = string;
 
   for( ; *s; s++)
     if(islower( (int)*s )) 
        *s = toupper( (int)*s );
 
   return( string );
}

