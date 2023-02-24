#define EXTERN extern

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>

#include "ir2.h"

/*----------------------------------------------------------
** unpad( s ) - removes beginning and trailing characters c
**              from string s. Return s.
**----------------------------------------------------------
*/
char *unpad ( 
   char * s,    // string to modify.
   int c        // unpads this character
)
{
   int i, l;
   if( s==NULL)
		return(NULL);
   i = l = strlen(s);
   i--;
   while ( (i>=0) && (s[i]==c) )
      s[i--] = 0;
   for( i=0; s[i]==c; i++);
   if( i > 0 )
       memmove(s, s+i, l-i+1);
   return(s);
}

