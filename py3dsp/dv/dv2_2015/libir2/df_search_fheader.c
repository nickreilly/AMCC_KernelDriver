
#define EXTERN extern

/*-------------------------
**  Standard include files
**-------------------------
*/
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>

/*------------------------------
**  Non-standard include files
**------------------------------
*/

#include "ir2.h"


/*--------------------------------------------------------------------------
 *  df_search_fheader() - searches a fits header block for a keyword and
 *             copies the value as a string to val_str. Returns
 *             the line number the keyword was found (0 is the first line)
 *             or -1 if keyword is not found.
 *--------------------------------------------------------------------------
 */
int df_search_fheader
(
	struct df_fheader_t *hdr,
	char             *keyword,  /* The keyword to search for.           */
	char             *val_str,  /* copy value of keyword to this buffer */
	int              val_str_size, /* sizeof val_str buffer */
	struct df_fheader_t **Rhdr, /* Address of header block containing item is returned */
	int              *offset,   /* return Line number in Rhdr where keyword is located */
	int              debug
)
{
   int  i, lnumber;
   char *cptr,
        *fits_buf_ptr;
   char kw[10];
   char line[81];
   char *st_ptr = NULL;  // for strtok ptr

#if FUN_NAME
   printf(">search_fheader()\n");
#endif

   strcpy( val_str, "");
   lnumber = 0;           /* keep track which line you are in */
   while( hdr )
   {
      fits_buf_ptr = (char *) hdr->buf;
      for( i = 0; i<36; i++)
      {
         memcpy( line, fits_buf_ptr, 80);     /* copy to local buffer */
         line[80] = 0x00;
         memcpy( kw, line, 8);
         kw[8] = 0x00;
         unpad( kw, ' ');

         if( !strcmp(keyword, kw) )
         {
            for( cptr = line+9;
                 *cptr == ' ' && cptr < line+80;
                 cptr++);                         /* skip to first char */

            if( *cptr == '\'' )
            {
               cptr++;
               if( NULL != (cptr =  strtok_r( cptr, "'", &st_ptr)) )
                  strxcpy( val_str, cptr, val_str_size);
            }
            else
            {
               if( NULL != (cptr = strtok_r(cptr, " ", &st_ptr)) )
                  strxcpy( val_str, cptr, val_str_size);
            }
				*Rhdr = hdr;
				*offset = i;
            return lnumber;
         }

         fits_buf_ptr += 80;   /* advance to next line  */
         lnumber++;
      }
      hdr = hdr->next;        /* advance to next header block */
   }
	*Rhdr = NULL;
	*offset = -1;
   return -1;
}

/************************ eof ************************/
