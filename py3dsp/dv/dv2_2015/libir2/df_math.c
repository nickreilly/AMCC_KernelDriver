/*********************************************************************************
**  df_math.c
********************************************************************************
*/
#define EXTERN extern
 
/*-------------------------
**  Standard include files
**-------------------------
*/
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
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

extern struct df_internal_vars DFOptions;
 
/*-------------------------------------------------------------------------
**  df_buffer_math() - Performs a fundamental math operation on the data.
**-------------------------------------------------------------------------
*/

int df_buffer_math( 
	struct df_buf_t * dest,      /* Buffer struct to hold the results. */
	struct df_buf_t * op1,       /* Buffer struct of operand 1. */
	struct df_buf_t * op2,       /* Buffer struct of operand 2. */
	int operation)               /* operations: DF_MATH_ADD, SUB, DIV, MUL, COPY */
{
   int rc, i;
   struct df_buf_t buf;      
	float  divisor1, divisor2,
			 fdat, fdata1, fdata2;
	double sum;

#if FUN_NAME
	printf(">df_math()\n");
#endif
   rc = ERR_NONE;

   if( op1->N != op2->N )
		return ERR_DIFF_SIZE;

   /* copy struct info from op1, initialize head & data blocks */
   memcpy( &buf, op1, sizeof(struct df_buf_t));
	buf.status  = DF_UNSAVED;
	buf.size    = sizeof(float);
	buf.bitpix  = DF_BITPIX_FLOAT;
	buf.fheader = NULL;
	buf.fdata   = NULL;
   buf.divisor = 1;              /* set divisor to 1 */
   buf.nframes = 1;
	buf.org_size   = sizeof(float);
	buf.org_bitpix = DF_BITPIX_FLOAT;
	strcpy( buf.filename, "unsaved.fts");

   if( ERR_NONE != (rc = df_math_copyheader( &buf, op1 )) )
		goto lerror;

   /*
   ** Allocate data memory & do math operation
   */
	if( NULL == ( buf.fdata = (float*) malloc( sizeof(float)*buf.N )) )
		{ rc = ERR_MEM_ALLOC; goto lerror; }

   buf.max = DF_MIN_SIGNED_INT32;
   buf.min = DF_MAX_SIGNED_INT32;
   sum = 0;
   divisor1 = op1->divisor;
   divisor2 = op2->divisor;
 
   for( i=0; i < buf.N; i++ )
   {
      if( DFOptions.divbycoadd )
      {
         fdata1 = op1->fdata[i] / divisor1;
         fdata2 = op2->fdata[i] / divisor2;
      }
      else
      {
         fdata1 = op1->fdata[i];
         fdata2 = op2->fdata[i];
      }

      switch( operation )
      {
         case DF_MATH_ADD:
            fdat = fdata1 + fdata2;
            break;

         case DF_MATH_SUB:
            fdat = fdata1 - fdata2;
            break;

         case DF_MATH_MUL:
            fdat = fdata1 * fdata2;
            break;

         case DF_MATH_DIV:
            if( fdata2==0)
               fdat = 0;
            else
               fdat = fdata1 / fdata2;
            break;

         case DF_MATH_SQRT:
				if( fdata1 < 0 )
					fdat = 0;
				else
					fdat = sqrt(fdata1);
				break;

         case DF_MATH_COPY:
			default:
               fdat = fdata1;
            break;
      }
      buf.max = MAX( buf.max, fdat);
      buf.min = MIN( buf.min, fdat);
      sum += fdat;

		buf.fdata[i] = fdat;
   }

	buf.mean = sum / buf.N;
	df_stats_p2( &buf, buf.mean );

   /* Fix text in fits header */
   df_math_fixheader( &buf, "Buffer Math Performed on data.");

lerror:
   if( rc != ERR_NONE )
	{
		df_free_fbuffer( &buf );
	}
	else
	{
		df_free_fbuffer( dest );  /* free contents of current destination */
      memcpy( dest, &buf, sizeof(struct df_buf_t)); /* copy buf to destination */
	}
   return rc;
}

/*-------------------------------------------------------------------------
**  df_constant_math() - Performs a fundamental math operation on the data.
**-------------------------------------------------------------------------
*/

int df_constant_math( 
	struct df_buf_t * dest,      /* Buffer struct to hold the results. */
	struct df_buf_t * op1,       /* Buffer struct of operand 1. */
	float             op2,       /* Operand 2 */
	int operation )              /* operations: DF_MATH_ADD, SUB, DIV, MUL */
{
   int rc, i;
   struct df_buf_t buf;      
	float  divisor1, 
			 fdat, fdata1;
	double sum;

#if FUN_NAME
	printf(">df_constant_math()\n");
#endif

	/* check for obvious errors */
   if( (operation == DF_MATH_COPY) ||
       ( operation == DF_MATH_DIV && op2 == 0 ))
      return ERR_INV_OPERATION;

   rc = ERR_NONE;

   /* copy struct info from op1, initialize head & data blocks */
   memcpy( &buf, op1, sizeof(struct df_buf_t));
	buf.status  = DF_UNSAVED;
	buf.size    = sizeof(float);
	buf.bitpix  = DF_BITPIX_FLOAT;
	buf.fheader = NULL;
	buf.fdata   = NULL;
   buf.divisor = 1;              /* set divisor to 1 */
   buf.nframes = 1;
	buf.org_size   = sizeof(float);
	buf.org_bitpix = DF_BITPIX_FLOAT;
	strcpy( buf.filename, "unsaved.fts");

   if( ERR_NONE != (rc = df_math_copyheader( &buf, op1 )) )
		goto lerror;

   /*
   ** Allocate data memory & do math operation
   */
	if( NULL == ( buf.fdata = (float*) malloc( sizeof(float)*buf.N )) )
		{ rc = ERR_MEM_ALLOC; goto lerror; }

   buf.max = DF_MIN_SIGNED_INT32;
   buf.min = DF_MAX_SIGNED_INT32;
   sum = 0;
   divisor1 = op1->divisor;
 
   for( i=0; i < buf.N; i++ )
   {
      if( DFOptions.divbycoadd )
      {
         fdata1 = op1->fdata[i] / divisor1;
      }
      else
      {
         fdata1 = op1->fdata[i];
      }

      switch( operation )
      {
         case DF_MATH_ADD:
            fdat = fdata1 + op2;
            break;

         case DF_MATH_SUB:
            fdat = fdata1 - op2;
            break;

         case DF_MATH_MUL:
            fdat = fdata1 * op2;
            break;

         case DF_MATH_DIV:
				fdat = fdata1 / op2;
            break;

			default:
               fdat = fdata1;
            break;
      }
      buf.max = MAX( buf.max, fdat);
      buf.min = MIN( buf.min, fdat);
      sum += fdat;

		buf.fdata[i] = fdat;
   }

	buf.mean = sum / buf.N;
	df_stats_p2( &buf, buf.mean );

   /* Fix text in fits header */
   df_math_fixheader( &buf, "Buffer Math Performed on data.");

lerror:
   if( rc != ERR_NONE )
	{
		df_free_fbuffer( &buf );
	}
	else
	{
		df_free_fbuffer( dest );  /* free contents of current destination */
      memcpy( dest, &buf, sizeof(struct df_buf_t)); /* copy buf to destination */
	}
   return rc;
}

/*-------------------------------------------------------------------------
**  df_buffer_rotate() - Rotates an image. 
**-------------------------------------------------------------------------
*/

int df_buffer_rotate( 
	struct df_buf_t * dest,      /* Buffer struct to hold the results. */
	struct df_buf_t * op1,       /* Buffer struct of operand 1. */
	int operation)               /* operations: DF_ROT_M90, DF_ROT_P90, ... */
{
   int rc, 
		 x, y;
   struct df_buf_t buf;      
   float *fptr, divisor;

#if FUN_NAME
	printf(">df_buffer_rotate()\n");
#endif

   rc = ERR_NONE;

   divisor = op1->divisor;

   /* copy struct info from op1, initialize head & data blocks */
   memcpy( &buf, op1, sizeof(struct df_buf_t));
	buf.status  = DF_UNSAVED;
	buf.size    = sizeof(float);
	buf.bitpix  = DF_BITPIX_FLOAT;
	buf.fheader = NULL;
	buf.fdata   = NULL;
   buf.divisor = 1;              /* set divisor to 1 */
   buf.nframes = 1;
	buf.org_size   = sizeof(float);
	buf.org_bitpix = DF_BITPIX_FLOAT;
	strcpy( buf.filename, "unsaved.fts");

   if( ERR_NONE != (rc = df_math_copyheader( &buf, op1 )) )
		goto lerror;

   /*
   ** Allocate data memory & do math operation
   */
	if( NULL == ( buf.fdata = (float*) malloc( sizeof(float)*buf.N )) )
		{ rc = ERR_MEM_ALLOC; goto lerror; }

   fptr = buf.fdata;
   if( operation == DF_ROT_M90 )  /* counter-clockwise 90 deg */
	{
		for( x=buf.naxis1-1; x >=0; x-- )
			for( y = 0; y < buf.naxis2; y++ )
			{
				*fptr = op1->fdata[y*buf.naxis1+x] / divisor;
				fptr++;
			}

      buf.naxis1 = op1->naxis2;
      buf.naxis2 = op1->naxis1;
	}
	else if( operation == DF_ROT_P90 )  /* clockwise 90 deg */
	{
		for( x=0; x < buf.naxis1; x++ )
			for( y = buf.naxis2-1; y >=0; y-- )
			{
				*fptr = op1->fdata[y*buf.naxis1+x] / divisor;
				fptr++;
			}

      buf.naxis1 = op1->naxis2;
      buf.naxis2 = op1->naxis1;
	}
	else if( operation == DF_ROT_180 )  
	{
		for( y = buf.naxis2-1; y >=0; y-- )
			for( x =  buf.naxis1-1; x >=0; x-- )
			{
				*fptr = op1->fdata[y*buf.naxis1+x] / divisor;
				fptr++;
			}
	}
	else
		{ rc = ERR_MEM_ALLOC; goto lerror; }

   /* Fix text in fits header */
   df_math_fixheader( &buf, "Math operation: Rotated image.");

lerror:
   if( rc != ERR_NONE )
	{
		df_free_fbuffer( &buf );
	}
	else
	{
		df_free_fbuffer( dest );  /* free contents of current destination */
      memcpy( dest, &buf, sizeof(struct df_buf_t)); /* copy buf to destination */
	}
   return rc;
}

/*-------------------------------------------------------------------------
**  df_copy_subarray() - copies the subarray(x,y,wid,hgt) from op1 to dest. 
**-------------------------------------------------------------------------
*/

int df_copy_subarray( 
	struct df_buf_t * dest,      /* Buffer struct to hold the results. */
	struct df_buf_t * op1,       /* Buffer struct of operand 1. */
	int op_x,                      /* op1's subarray(x,y,wid,hgt) to copy */
	int op_y,
	int op_wid, 
	int op_hgt
	)
{
   int rc,
		 x, y,
		 dinx,
		 sinx; 
   struct df_buf_t buf;      

#if FUN_NAME
	printf(">df_copy_subarray()\n");
#endif

   /* check for parameter errors */
	if( ((op_x + op_wid) > op1->naxis1 ) ||
	    ((op_y + op_hgt) > op1->naxis2 ) ||
	    ( (op_wid*op_hgt) < 0 ) )
      return ERR_SUBARRAY_FORMAT;

   rc = ERR_NONE;

   /* copy struct info from op1, initialize head & data blocks */
   memcpy( &buf, op1, sizeof(struct df_buf_t));
	buf.status  = DF_UNSAVED;
	buf.naxis1  = op_wid;
	buf.naxis2  = op_hgt;
	buf.size    = sizeof(float);
	buf.bitpix  = DF_BITPIX_FLOAT;
	buf.N       = op_wid*op_hgt;
	buf.fheader = NULL;
	buf.fdata   = NULL;
   buf.divisor = 1;              /* set divisor to 1 */
   buf.nframes = 1;
	buf.org_size   = sizeof(float);
	buf.org_bitpix = DF_BITPIX_FLOAT;
	strcpy( buf.filename, "unsaved.fts");

   if( ERR_NONE != (rc = df_math_copyheader( &buf, op1 )) )
		goto lerror;

   /*
   ** Allocate data memory for results
   */
	if( NULL == ( buf.fdata = (float*) malloc( sizeof(float)*buf.N )) )
		{ rc = ERR_MEM_ALLOC; goto lerror; }

   /* Copy over data  */
	dinx = 0;
	for( y=op_y; y < (op_y+op_hgt); y++ )
	{
		sinx = y*op1->naxis1 + op_x;
		for( x=op_x; x < (op_x+op_wid); x++ )
		{
			buf.fdata[dinx] = op1->fdata[sinx];
			dinx++;
			sinx++;
		}
	}

   /* do statistics calculations */
	df_stats( &buf );

   /* Fix text in fits header */
   df_math_fixheader( &buf, "User supplied Math operation performed.");

lerror:
   if( rc != ERR_NONE )
	{
		df_free_fbuffer( &buf );
	}
	else
	{
		df_free_fbuffer( dest );  /* free contents of current destination */
      memcpy( dest, &buf, sizeof(struct df_buf_t)); /* copy buf to destination */
	}
   return rc;
}

/*-------------------------------------------------------------------------
**  df_buffer_userfun() - supports user supplied calculation function. 
**-------------------------------------------------------------------------
*/

int df_buffer_userfun( 
	struct df_buf_t * dest,      /* Buffer struct to hold the results. */
	struct df_buf_t * op1,       /* Buffer struct of operand 1. */
	int (*userfun)( struct df_buf_t *dest, struct df_buf_t *op1 )
	)
{
   int rc; 
   struct df_buf_t buf;      

#if FUN_NAME
	printf(">df_buffer_userfun()\n");
#endif

   rc = ERR_NONE;

   /* copy struct info from op1, initialize head & data blocks */
   memcpy( &buf, op1, sizeof(struct df_buf_t));
	buf.status  = DF_UNSAVED;
	buf.size    = sizeof(float);
	buf.bitpix  = DF_BITPIX_FLOAT;
	buf.fheader = NULL;
	buf.fdata   = NULL;
   buf.divisor = 1;              /* set divisor to 1 */
   buf.nframes = 1;
	buf.org_size   = sizeof(float);
	buf.org_bitpix = DF_BITPIX_FLOAT;
	strcpy( buf.filename, "unsaved.fts");

   if( ERR_NONE != (rc = df_math_copyheader( &buf, op1 )) )
		goto lerror;

   /*
   ** Allocate data memory for results
   */
	if( NULL == ( buf.fdata = (float*) malloc( sizeof(float)*buf.N )) )
		{ rc = ERR_MEM_ALLOC; goto lerror; }

   /* call user supplied funtion to modify data */
	userfun( &buf, op1 );

   /* do statistics calculations */
	df_stats( &buf );

   /* Fix text in fits header */
   df_math_fixheader( &buf, "User supplied Math operation performed.");

lerror:
   if( rc != ERR_NONE )
	{
		df_free_fbuffer( &buf );
	}
	else
	{
		df_free_fbuffer( dest );  /* free contents of current destination */
      memcpy( dest, &buf, sizeof(struct df_buf_t)); /* copy buf to destination */
	}
   return rc;
}

/*----------------------------------------------------------------------------------
**  df_math_copyheader() - duplicates the header for the src buffer to the 
**                         dest buffer.
**----------------------------------------------------------------------------------
*/

int df_math_copyheader( 
	struct df_buf_t * dest,      /* Buffer struct to hold the results. */
	struct df_buf_t * src)      /* Buffer struct of operand 1. */
{
   int first_time;
   struct df_fheader_t * src_hdr,
                       * dest_hdr,
                       * temp;
   int rc = ERR_NONE;
 
   if( dest == src )
      return ERR_NONE;
 
   /*  Delete destintation header */
   dest_hdr = dest->fheader;
   dest->fheader = NULL;
   dest->Nheader = 0;
   while( dest_hdr )
   {
      temp = dest_hdr;
      dest_hdr = dest_hdr->next;
      free( (char*) temp);
   }
 
   /*  Duplicate the fits header information */
   first_time = TRUE;
   src_hdr = src->fheader;
   while( src_hdr )
   {
      /* Allocate data and fill in structures */
      if( NULL == ( temp=(struct df_fheader_t *)malloc( sizeof(struct df_fheader_t)) ) )
      {
         rc = ERR_MEM_ALLOC;
         goto Lerror;
      }
      temp->next = NULL;
      memcpy((char*)temp->buf, (char*)src_hdr->buf, sizeof(temp->buf));
 
      /* Append new block to destination frame */
      if( first_time )
      {
         dest->fheader = temp;
         first_time = FALSE;
      }
      else
         dest_hdr->next = temp;
      dest_hdr = temp;
 
      /* point to next src block */
      src_hdr = src_hdr->next;  /* goto next header block */
   }
 
   /* Update FBUFFER structure */
   dest->Nheader = src->Nheader;
 
Lerror:
   return rc;
}

/*-------------------------------------------------------------------------
**  df_math_fixheader() - Fixed up the FITS header string string to math
**     the values in the structure. Also adds history if available.
**-------------------------------------------------------------------------
*/

int df_math_fixheader( 
   struct df_buf_t * bufp,   /* buffer to be fixed up */
   char * history)           /* text to add as history line or NULL */
{
   int  offset;
   char buf[80];
   char *cptr;
   struct df_fheader_t * f_hdr;
 
   if( df_search_fheader( bufp->fheader, "NAXIS", buf, sizeof(buf), &f_hdr, &offset, FALSE) >= 0)
   {
      sprintf( buf, "%-8.8s= %20d / %-40.40s ", "NAXIS", 2, "Number of Axis");
      memcpy( (char*)f_hdr->buf+(offset*80), buf, strlen(buf));
   }
   if( df_search_fheader( bufp->fheader, "NAXIS1", buf, sizeof(buf), &f_hdr, &offset, FALSE) >= 0)
   {
      sprintf( buf, "%-8.8s= %20d / %-40.40s ", "NAXIS1", bufp->naxis1,
               "Pixels on 1nd most varying axis");
      memcpy( (char*)f_hdr->buf+(offset*80), buf, strlen(buf));
   }
   if( df_search_fheader( bufp->fheader, "NAXIS2", buf, sizeof(buf), &f_hdr, &offset, FALSE) >= 0)
   {
      sprintf( buf, "%-8.8s= %20d / %-40.40s ", "NAXIS2", bufp->naxis2,
               "Pixels on 2nd most varying axis");
      memcpy( (char*)f_hdr->buf+(offset*80), buf, strlen(buf));
   }
   if( df_search_fheader( bufp->fheader, "DATAMIN", buf, sizeof(buf), &f_hdr, &offset, FALSE) >= 0)
   {
      sprintf( buf, "%-8.8s= %20g / %-40.40s ", "DATAMIN", bufp->min, "Minimum Pixel Value");
      memcpy( (char*)f_hdr->buf+(offset*80), buf, strlen(buf));
   }
   if( df_search_fheader( bufp->fheader, "DATAMAX", buf, sizeof(buf), &f_hdr, &offset, FALSE) >= 0)
   {
      sprintf( buf, "%-8.8s= %20g / %-40.40s ", "DATAMAX", bufp->max, "Maximum Pixel Value");
      memcpy( (char*)f_hdr->buf+(offset*80), buf, strlen(buf));
   }
   if( df_search_fheader( bufp->fheader, "DATAMEAN", buf, sizeof(buf), &f_hdr, &offset, FALSE) >= 0)
   {
      sprintf( buf, "%-8.8s= %20g / %-40.40s ", "DATAMEAN", bufp->mean, "Mean Pixel Value");
      memcpy( (char*)f_hdr->buf+(offset*80), buf, strlen(buf));
   }
   if( df_search_fheader( bufp->fheader, "DATASTD", buf, sizeof(buf), &f_hdr, &offset, FALSE) >= 0)
   {
      sprintf( buf, "%-8.8s= %20g / Data Standard Dev. ", "DATASTD", bufp->stddev);
     memcpy( (char*)f_hdr->buf+(offset*80), buf, strlen(buf));
   }
   if( df_search_fheader( bufp->fheader, "DIVISOR", buf, sizeof(buf), &f_hdr, &offset, FALSE) >= 0)
   {
      sprintf( buf, "%-8.8s= %20f / Divide by coadds value", "DIVISOR", bufp->divisor);
      memcpy( (char*)f_hdr->buf+(offset*80), buf, strlen(buf));
   }
 
   if( history )
   {
		if( df_search_fheader( bufp->fheader, "END", buf, sizeof(buf), &f_hdr, &offset, FALSE) >= 0)
			if( offset < 35 )
			{
				sprintf( buf, "HISTORY = '%-.60s'", history);
				cptr = f_hdr->buf+(offset*80);
				memset( cptr, ' ', 80);
				memcpy( cptr, buf, strlen(buf));
	 
				cptr += 80;
				memset( cptr, ' ', 80);
				memcpy( cptr, "END", 3);
				bufp->Nheader++;
			}
   }
 
   return ERR_NONE;
}

