
#define EXTERN extern

/*-------------------------
**  Standard include files
**-------------------------
*/
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <math.h>

#include <sys/param.h>
#include <malloc.h>
#include <memory.h>

/*------------------------------
**  Non-standard include files
**------------------------------
*/

#include "ir2.h"

/*---------------------------------------------------------------------------
**  df_write_fits() - writes the struct df_buf_t as a FITS file.
**         fd     - File description number.
**       bufp     - Pointer to df_buf_t structure.
**  Returns     ERR_NONE  File read OK.
**         ERR_FILE_READ  Error reading file.
**---------------------------------------------------------------------------
*/

int df_write_fits(
   int  fd,                    /* file description to read.                   */
   char * path,                /* path & filename need for header inforamtion */
   char * filename,
   struct df_buf_t * bufp      /* Buffer struct to hold the data. */
)
{
  int  rc,
		 numrec,
		 items_per_record,
		 loop, inx, i;
  char *fits_buf;
  uint32_t *dptr,
		     *sptr;
  struct df_fheader_t * f_hdr;

#if FUN_NAME
   printf(">df_write_fits()\n");
#endif

   rc = ERR_NONE;
   fits_buf = NULL;
   /*
   **  Construct filename, open file.
   */
   strxcpy( bufp->directory, path,     sizeof(bufp->directory));
   strxcpy( bufp->filename,  filename, sizeof(bufp->filename));

   /*------------------------------------------------------
   **  Output the header
   */
   f_hdr = bufp->fheader;
   while( f_hdr != NULL )
   {
      if( DF_FITS_RECORD_LEN != write( fd, f_hdr->buf, DF_FITS_RECORD_LEN))
      {
         rc = ERR_FILE_WRITE; goto Lerror;
      }
 
      f_hdr = f_hdr->next;
   }

   /*------------------------------------------------------
   **  Output data
   */

   /* allocate buffer */
   if( NULL == ( fits_buf = (char *) calloc( DF_RW_BUFFER_SIZE, sizeof(char))) )
      return(ERR_MEM_ALLOC);

   numrec = ceil( (double) bufp->N*sizeof(float)/ DF_RW_BUFFER_SIZE );
   items_per_record = DF_RW_BUFFER_SIZE / sizeof(float);
   inx = 0;
	sptr = (uint32_t*)bufp->fdata;

   for(loop=0; loop < numrec; loop++)
   {
		dptr = (uint32_t*) fits_buf;
      for( i=0; i < items_per_record && inx < bufp->N; i++)
		{
			*dptr++ = htonl( *sptr++ );
			inx++;
		}

		if( DF_RW_BUFFER_SIZE != write( fd, (char *)fits_buf, DF_RW_BUFFER_SIZE))
				{ rc = ERR_FILE_READ; goto Lerror; }
   }

   /*
	**  If everything went OK, set status to saved.
	*/
	bufp->status = DF_SAVED;

Lerror:
   if( fits_buf )
      free( fits_buf );
   return rc;
}


/************************ eof ************************/
