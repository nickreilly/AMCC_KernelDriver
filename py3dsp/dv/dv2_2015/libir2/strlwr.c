#define EXTERN extern

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>

#include "ir2.h"

/*----------------------------------------------------------
** strlwr() - converts a string to lower case.
**               Returns: pointer to string.
**----------------------------------------------------------
*/
 
char * strlwr( string )
   char * string;
{
   char * s;
   if( string == NULL ) return(string);
   s = string;
 
   for( ; *s; s++)
     if(isupper( (int)*s )) 
        *s = tolower( (int)*s );
 
   return( string );
}

