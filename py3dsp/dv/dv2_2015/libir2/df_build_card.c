
#define EXTERN extern

/*-------------------------
**  Standard include files
**-------------------------
*/
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <memory.h>

/*------------------------------
**  Non-standard include files
**------------------------------
*/

#include "ir2.h"

/*--------------------------------------------------------------------
**  build_card with 20 character value with comment.
**  Writes card to file fp and updates remaining_cards.
**--------------------------------------------------------------------
*/
char * df_build_card( char *cptr, char *keyword, char *value, char *comment)
{
   int l;
   char outbuf[85];

   if( keyword == NULL )
   {
      strcpy( outbuf, "        ");
      goto ldone;
   }
   if( value == NULL )
   {
      sprintf(outbuf, "%-8.8s                ", keyword );
      goto ldone;
   }

   if( strlen(value) > 18 )
   {
      if( comment == NULL )
         sprintf(outbuf, "%-8.8s= %30.30s       ", keyword, value );
      else
         sprintf(outbuf, "%-8.8s= %30.30s / %-s ", keyword, value, comment);
   }
   else
   {
      if( comment == NULL )
         sprintf(outbuf, "%-8.8s= %20.20s       ", keyword, value );
      else
         sprintf(outbuf, "%-8.8s= %20.20s / %-s ", keyword, value, comment);
   }

ldone:
   l = strlen(outbuf);
   if( l < 80 )
      memset( &outbuf[l], ' ', 80-l);

   outbuf[80] = '\0';
   strncpy( cptr, outbuf, 80);
   return ( cptr + 80 );
}

/************************ eof ************************/

