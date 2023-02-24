
#define EXTERN extern

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>

#include "ir2.h"

/*-------------------------------------------------------------
** my_atol() - convert a string to a long. The string s
**   must consist of  '[white space][+-] digits [white spaces]'
**   returns error: 0 = noerror;
**                 -1 = invalid char in string
**-------------------------------------------------------------
*/
 
int my_atol( char *s, long *l )
{
   char *start;
   int  cnt;
 
   if( s == NULL)
      return -1;
 
   while( isspace( (int) *s) ) s++;     /* Skip leading spaces */
   start = s;
 
   if( *s=='-' || *s=='+' ) s++; /* First char can be a signed */
 
   cnt = 0;
   while( isdigit( (int)*s ) )          /* Only allow decimal digits */
   {
     s++;
     cnt++;
   }
   if( !cnt )  return -1;
                                 /* Allow ONLY trailing white spaces */
   while( *s )
      if( isspace( (int)*s ) )
         s++;
      else
         return -1;
 
   *l = strtol( start, (char**) NULL, 10 );
   return 0;
}
 


