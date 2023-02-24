
#define EXTERN extern

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>

#include "ir2.h"

/*----------------------------------------------------------
**  matchname - checks if pattern matches the name. Pattern
**              can contain '*', '?' anywhere.
**  Returns:  1 if name matches pattern, 0 otherwise.
**----------------------------------------------------------
*/
 
int matchname( 
   char *pattern, 
   char *name 
)
{
   while( *pattern )
   {
     if( (*pattern == *name) || (*pattern=='?' && *name))
     {
       pattern++;
       name++;
     }
     else if( *pattern == '*' )
     {
       for(;;)
       {
         if( matchname( pattern+1, name)) return 1;
         if( *name ) name++;
         else return 0;
       }
     }
     else return 0;
   }
   return *name == '\0';
}

