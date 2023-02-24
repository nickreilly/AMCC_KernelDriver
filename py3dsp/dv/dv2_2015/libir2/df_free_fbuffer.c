
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
#include <malloc.h>
#include <memory.h>

/*------------------------------
**  Non-standard include files
**------------------------------
*/

#include "ir2.h"

/*--------------------------------------------------------------------------
 *  df_free_fbuffer() - deallocated memory associated with buffer and sets
 *                  status to DF_FITS_EMPTY.
 *--------------------------------------------------------------------------
 */
int df_free_fbuffer( struct df_buf_t *bufp )
{
   struct df_fheader_t * nxt_hdr,
                     * tmp_hdr;

#if FUN_NAME
   printf(">df_free_fbuffer()\n");
#endif
   nxt_hdr = bufp->fheader;
   while( nxt_hdr )
   {
      tmp_hdr = nxt_hdr;          /* tmp = hdr to be freed  */
      nxt_hdr = nxt_hdr->next;    /* save reference to next */
      free( (char*)tmp_hdr );     /* de-allocate it         */
   }

   if( bufp->fdata )              /* de-allocate data blocks */
      free( (char*) bufp->fdata );

   bufp->status = DF_EMPTY;     /* Update status           */
   bufp->fheader = NULL;          /* and the pointer         */
   bufp->fdata   = NULL;

   return ERR_NONE;
}

/************************ eof ************************/
