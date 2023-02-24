#define EXTERN extern

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>

#include "ir2.h"

/*---------------------------------------------------------
**  str_rws() - changes white space characters in a string to ' '.
**---------------------------------------------------------
*/
char *str_rws( char * s, unsigned int maxlen )
{
   char *cptr = s;
   while( maxlen && *cptr!=0 )
   {
      if( isspace( (int)*cptr ) ) *cptr = ' ';
      maxlen--;
      cptr++;
   }
   return s;
}

