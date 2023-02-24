
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

static int df_read_pixels( int fd, struct df_buf_t *bufp, int is_socket, char * fits_buf,
	double bscale, double bzero );

#define DEBUG 0
/*---------------------------------------------------------------------------
**  df_read_fits() - reads a fits file into the struct df_buf_t. This function
**     is for reading 2D Fits files. However, limited 3D Fits support is
**     provide.
**     IF is_3d is true, this function reads the 1st images of the 3D fits
**     files into bufp. the df_3dinfo_t struct is also file with relevant
**     information on the 3D Fits to enable the application to read the 
**     remaining frames.
**  Returns     ERR_NONE  File read OK.
**         ERR_FILE_READ  Error reading file.
**---------------------------------------------------------------------------
*/

int df_read_fits( 
   int  fd,                 /* file description to read.                   */
   char * path,             /* path & filename need for header inforamtion */
   char * filename,
   struct df_buf_t * bufp,  /* Buffer struct to hold the data. */
   int  is_socket,          /* TRUE if it's a socket, otherwise false */
	int  is_3d               /* If true, will read 1st image of 3d file */
)
{
  int loop, error, bread, i, naxis;
  long lpar;
  float fpar;
  double  bscale, bzero;
  char *fits_buf;
  char buf[85];
  char * str_ptr;

  struct df_fheader_t * last_hdr;
  struct df_fheader_t * f_hdr;

#if FUN_NAME
   printf(">df_read_fits()\n");
#endif
   /*
   **  Allocate buffer
   */
   if( NULL == ( fits_buf = (char *) malloc( (u_int)DF_RW_BUFFER_SIZE)) )
      return(ERR_MEM_ALLOC);
   /*
   **  Construct filename, check if over writting old data, open file.
   */

   if( bufp->status != DF_EMPTY ) df_free_fbuffer( bufp ); /* Clear the Buffer structure */
   strxcpy( bufp->directory, path, sizeof(bufp->directory));
   strxcpy( bufp->filename,  filename, sizeof(bufp->filename));
   error = ERR_NONE;
   /*
   **  Read first header block and search for required keyword & values.
   */
   if( NULL == ( bufp->fheader = (struct df_fheader_t*)
                                 malloc(  (u_int)  sizeof(struct df_fheader_t) )) )
   {
      error = ERR_MEM_ALLOC; goto Lerror;
   }
   bufp->fheader->next = NULL;
   
   /* Read 1st header block */
   if( is_socket )
      bread = sock_read_data( fd, (char*) bufp->fheader->buf, DF_FITS_RECORD_LEN, TRUE, 1000);
   else
      bread = read( fd, (char*)bufp->fheader->buf, DF_FITS_RECORD_LEN);
   if( DF_FITS_RECORD_LEN != bread )
   {
      error = ERR_FILE_READ;
      goto Lerror;
   }

   /* SIMPLE or XTENSION */
   /* should be at position 0 */
   if( (i=df_search_fheader( bufp->fheader, "SIMPLE", buf, sizeof(buf), &f_hdr, &i, FALSE)) == 0 )
   {
      if( buf[0] != 'T' )
         { error = ERR_FILE_FORMAT; goto Lerror; }
if( DEBUG ) printf("SIMPLE & T \n");
   }
   else if( (i=df_search_fheader( bufp->fheader, "XTENSION", buf, sizeof(buf), &f_hdr, &i, FALSE)) == 0 )
   {
      if( strncmp( "IMAGE", buf, 5) != 0 )
         { error = ERR_FILE_FORMAT; goto Lerror; }
if( DEBUG ) printf("XTENSION & IMAGE\n");
   }
   else
   {
      error = ERR_FILE_FORMAT;
      goto Lerror;
   }

   /* NAXIS   */
   /* Originally written specificly for naxis equals to 2....kluges for naxis=1&3
   ** Lib needs to be fixed to handle aribritary number of naxis.
   */
   if( df_search_fheader( bufp->fheader, "NAXIS", buf, sizeof(buf), &f_hdr, &i, FALSE) < 0 )
      { error = ERR_FILE_FORMAT; goto Lerror; }

   if( -1 == my_atol( buf, &lpar) || !INRANGE(0, lpar, 3) )
      { error = ERR_FILE_FORMAT; goto Lerror; }
   naxis = lpar;

   /* NAXIS1  */
	if( naxis == 0 )
		bufp->naxis1 = (short) 0;
   else 
	{
		if( df_search_fheader( bufp->fheader, "NAXIS1", buf, sizeof(buf), &f_hdr, &i, FALSE) < 0 )
			{ error = ERR_FILE_FORMAT; goto Lerror; }

		if( -1 == my_atol( buf, &lpar) || ( lpar < 1))   // no maximum
			{ error = ERR_FILE_FORMAT; goto Lerror; }
		//if( -1 == my_atol( buf, &lpar) || !INRANGE(1, lpar, 2048))
		//   { error = ERR_FILE_FORMAT; goto Lerror; }
		bufp->naxis1 = (short) lpar;
	}

   /* NAXIS2  */
   if( naxis == 0 )
		bufp->naxis2 = 0;  /* naxis is 0, no data */
   else if( naxis == 1 )
		bufp->naxis2 = 1;  /* naxis is 1, set naxis2 to be 1 */
   else
   {
		if( df_search_fheader( bufp->fheader, "NAXIS2", buf, sizeof(buf), &f_hdr, &i, FALSE) < 0 )
			{ error = ERR_FILE_FORMAT; goto Lerror; }

		if( -1 == my_atol( buf, &lpar) || (lpar < 1 ))   // no maximum
			{ error = ERR_FILE_FORMAT; goto Lerror; }
		//	if( -1 == my_atol( buf, &lpar) || !INRANGE(1, lpar, 2048))
		//		{ error = ERR_FILE_FORMAT; goto Lerror; }
		bufp->naxis2 = (short) lpar;
   }

   /* NAXIS3  ( optional, but must be equal to 1 for 2D, ) */
	lpar = 1;
   if( df_search_fheader( bufp->fheader, "NAXIS3", buf, sizeof(buf), &f_hdr, &i, FALSE) >= 0 )
	{
      if( -1 == my_atol( buf, &lpar) )
         { error = ERR_FILE_FORMAT; goto Lerror; }
	}
	if( (is_3d == FALSE) && (lpar > 1))
      { error = ERR_FILE_FORMAT; goto Lerror; }
    bufp->nframes = lpar;

   /* BITPIX  */
   if( df_search_fheader( bufp->fheader, "BITPIX", buf, sizeof(buf), &f_hdr, &i, FALSE) < 0 )
      { error = ERR_FILE_FORMAT; goto Lerror; }

   if( -1 == my_atol( buf, &lpar) || !INRANGE(-32, lpar, 32))
      { error = ERR_FILE_FORMAT; goto Lerror; }
	bufp->bitpix = lpar;
   bufp->size = (short) abs((lpar/8));

   // detemine N
	if( naxis == 0 )
		bufp->N    = 0;
   else
		bufp->N    = bufp->naxis1 * bufp->naxis2;

if( DEBUG ) printf(" naxis=%d naxis1=%d naxis2=%d nframes=%d \n", 
                     naxis, bufp->naxis1, bufp->naxis2, bufp->nframes);

   /* BSCALE */
   if( (df_search_fheader( bufp->fheader, "BSCALE", buf, sizeof(buf), &f_hdr, &i, FALSE) >= 0 ) &&
	    (parseFloat_r( &fpar, buf, " ", &str_ptr) == ERR_NONE ) )
   {
      bscale = fpar;
   }
	else
		bscale = 1;
   bufp->org_bscale = bscale;

   /* BZERO  */
   if( (df_search_fheader( bufp->fheader, "BZERO", buf, sizeof(buf), &f_hdr, &i, FALSE) >= 0 ) &&
	    (parseFloat_r( &fpar, buf, " ", &str_ptr) == ERR_NONE ) )
   {
      bzero = fpar;
   }
	else
		bzero = 0;
   bufp->org_bzero = bzero;

   /* END     */
   if( (i=df_search_fheader( bufp->fheader, "END", buf, sizeof(buf), &f_hdr, &i, FALSE)) >= 0 )
      loop = FALSE; else loop = TRUE;
   bufp->Nheader = i+1;
   bufp->sizeof_header = DF_FITS_RECORD_LEN;

   /*
   **  Now read in the rest of the headers blocks.
   */
   last_hdr = bufp->fheader;
   while( loop )
   {
      if( NULL == (last_hdr->next = (struct df_fheader_t*)
                                    malloc( (u_int) sizeof(struct df_fheader_t) )) )
      {
         error = ERR_MEM_ALLOC;
         goto Lerror;
      }
      last_hdr = last_hdr->next;
      last_hdr->next = NULL;

      if( is_socket )
         bread = sock_read_data(fd, (char*)last_hdr->buf, DF_FITS_RECORD_LEN, TRUE, 1000 );
      else
         bread = read(fd, (char*)last_hdr->buf, DF_FITS_RECORD_LEN);
      if( DF_FITS_RECORD_LEN != bread )
      {
         error = ERR_FILE_READ;
         goto Lerror;
      }
      if( 0 <= (bufp->Nheader=df_search_fheader( bufp->fheader, "END", buf, sizeof(buf), 
			&f_hdr, &i, FALSE) ))
         loop = FALSE; else loop = TRUE;

      bufp->sizeof_header += DF_FITS_RECORD_LEN;
   }
	bufp->Nheader++;   /* This is the count, not an index */

   /*-----------------------------------------------------------------
	** search for other useful keyworks that may not be in 1st block
	*/

   /* DIVISOR */
   bufp->divisor = (short) 1;
   if( df_search_fheader( bufp->fheader, "DIVISOR", buf, sizeof(buf), &f_hdr, &i, FALSE) >= 0  )
      if( 0 == my_atol( buf, &lpar) && INRANGE(1, lpar, 32767))
         bufp->divisor = (short) lpar;

   /* ASEC_PIX  or PLATE_SC */
   bufp->arcsec_pixel = 0;
   if( df_search_fheader( bufp->fheader, "PLATE_SC", buf, sizeof(buf), &f_hdr, &i, FALSE) >= 0 )
   {
	   if( parseFloatR_r( &fpar, buf, " ", &str_ptr, 0, 10.0) == ERR_NONE )
         bufp->arcsec_pixel = fpar;
   }
   if( df_search_fheader( bufp->fheader, "ASEC_PIX", buf, sizeof(buf), &f_hdr, &i, FALSE) >= 0 )
   {
	   if( parseFloatR_r( &fpar, buf, " ", &str_ptr, 0, 10.0) == ERR_NONE )
         bufp->arcsec_pixel = fpar;
   }

   /* POSANGLE */
   bufp->pos_angle = 0;
   if( df_search_fheader( bufp->fheader, "POSANGLE", buf, sizeof(buf), &f_hdr, &i, FALSE) >= 0 )
   {
	   if( parseFloat_r( &fpar, buf, " ", &str_ptr) == ERR_NONE )
			bufp->pos_angle = fpar;
		/* adjust angle to be in the range 0..360 */
		if( bufp->pos_angle > 360.0 )
			bufp->pos_angle = fmod( bufp->pos_angle, 360.0 );
		if( bufp->pos_angle < 0.0 )
			bufp->pos_angle = 360.0 - fmod( -bufp->pos_angle, 360.0 );
   }

   /* itime */
   bufp->itime = 1;
   if( (df_search_fheader( bufp->fheader, "ITIME", buf, sizeof(buf), &f_hdr, &i, FALSE) >= 0  ) &&
	    (parseFloat_r( &fpar, buf, " ", &str_ptr) == ERR_NONE ) )
		bufp->itime = fpar;

   /* filter_zp */
   bufp->filter_zp = 0;
   if( (df_search_fheader( bufp->fheader, "FLTZP", buf, sizeof(buf), &f_hdr, &i, FALSE) >= 0  ) &&
	    (parseFloat_r( &fpar, buf, " ", &str_ptr) == ERR_NONE ) )
		bufp->filter_zp = fpar;

   /* filter_zp */
   bufp->ext_coff = 0;
   if( (df_search_fheader( bufp->fheader, "EXT_COF", buf, sizeof(buf), &f_hdr, &i, FALSE) >= 0  ) &&
	    (parseFloat_r( &fpar, buf, " ", &str_ptr) == ERR_NONE ) )
		bufp->ext_coff = fpar;

   /* filter_zp */
   bufp->airmass = 1;
   if( (df_search_fheader( bufp->fheader, "AIRMASS", buf, sizeof(buf), &f_hdr, &i, FALSE) >= 0 ) &&
	    (parseFloat_r( &fpar, buf, " ", &str_ptr) == ERR_NONE ) )
		bufp->airmass = fpar;

   /* GUIDEBOX variable */
   bufp->gbox_enable = FALSE;   // default is false
   if( df_search_fheader( bufp->fheader, "GBOX_DIM", buf, sizeof(buf), &f_hdr, &i, FALSE) >= 0  )
   {
      if( parseInt_r( &bufp->gbox_dim[0], buf, " ,", &str_ptr) == ERR_NONE )
			if( parseInt_r( &bufp->gbox_dim[1], NULL, " ,", &str_ptr) == ERR_NONE )
				if( parseInt_r( &bufp->gbox_dim[2], NULL, " ,", &str_ptr) == ERR_NONE )
					if( parseInt_r( &bufp->gbox_dim[3], NULL, " ,", &str_ptr) == ERR_NONE )
						bufp->gbox_enable = TRUE;   // good box_dim[] - enable gbox.
   }
   if( df_search_fheader( bufp->fheader, "GBOX_FRM", buf, sizeof(buf), &f_hdr, &i, FALSE) >= 0  )
   {
      if( parseFloat_r( &bufp->gbox_from[0], buf, " ,", &str_ptr) == ERR_NONE )
			parseFloat_r( &bufp->gbox_from[1], NULL, " ,", &str_ptr);
   }
   if( df_search_fheader( bufp->fheader, "GBOX_TO", buf, sizeof(buf), &f_hdr, &i, FALSE) >= 0  )
   {
      if( parseFloat_r( &bufp->gbox_to[0], buf, " ,", &str_ptr) == ERR_NONE )
			parseFloat_r( &bufp->gbox_to[1], NULL, " ,", &str_ptr);
   }

if( DEBUG ) printf(" bscale=%f  bzero=%f \n", bufp->org_bscale, bufp->org_bzero);
   /*
   **  Allocate memory for data buffer.
	**  Note: The data is stored as floats.
   */
	if( NULL == ( bufp->fdata = (float*) malloc( (u_int)bufp->N*sizeof(float) ) ) )
      { error = ERR_MEM_ALLOC; goto Lerror; }

   /*
   **  Read in the data.
	*/
	if( ERR_NONE != df_read_pixels( fd, bufp, is_socket, fits_buf, bscale, bzero ))
      { error = ERR_FILE_READ; goto Lerror; }

   /*
	** Save original bitpix, size.
	** change bitpit to DF_BITPIX_FLOAT. All FITS is treated as float by dflib.
	*/
	bufp->org_size = bufp->size;
	bufp->org_bitpix = bufp->bitpix;

	bufp->size = sizeof(float);
	bufp->bitpix = DF_BITPIX_FLOAT;
	df_search_fheader( bufp->fheader, "BITPIX", buf, sizeof(buf), &f_hdr, &i, FALSE);
	df_build_card( f_hdr->buf+(i*80), "BITPIX", "-32", "32 BITS FLOATING POINT");
	/*
	** change bscale=1 & bzero=0.
	*/
	if( df_search_fheader( bufp->fheader, "BSCALE", buf, sizeof(buf), &f_hdr, &i, FALSE) >= 0 )
		df_build_card( f_hdr->buf+(i*80), "BSCALE", "1", "Scaling factor for data");
	if( df_search_fheader( bufp->fheader, "BZERO", buf, sizeof(buf), &f_hdr, &i, FALSE) >= 0) 
		df_build_card( f_hdr->buf+(i*80), "BZERO", "0", "Offset factor for data");

   /*
   **  no error.
   */
   bufp->status = DF_SAVED;
   free(fits_buf);
   return error;

   /*
   **  error handler.
   */
Lerror:                               /* on error close, free, and return */
   free(fits_buf);
   df_free_fbuffer( bufp );
   return error;
}


/*---------------------------------------------------------------------
**   df_read_pixels() - sub function to read in pixels.
**---------------------------------------------------------------------
*/

static int df_read_pixels( 
	int fd,                   /* file descriptor for reading data */
	struct df_buf_t * bufp,   /* Dest struct to hold ALL the data. */
	int is_socket,
	char * fits_buf,          /* buffer for reading in data */
	double bscale,
	double bzero
)
{
   int nloops,            /* number of fits records to read */
	    pixels_per_record, /* number of pixel in each FITS record */
	    pixels_left,       /* counter */
		 loop,
		 byte_recd,
		 byte_req,
		 fits_blocks,
		 i;

	float *dest;

#if FUN_NAME
   printf(">df_read_pixels() BITPIX=%d\n", bufp->bitpix);
#endif
   nloops = ceil( (float) (bufp->N * bufp->size) / DF_RW_BUFFER_SIZE);
   pixels_per_record = DF_RW_BUFFER_SIZE / bufp->size;
   pixels_left = bufp->N;
   dest  = bufp->fdata;

	for(loop=0; loop < nloops; loop++)
	{
	   /* bytes requested are multiples of DF_FITS_RECORD_LEN */
		fits_blocks  = ceil( (float) (pixels_left * bufp->size) / DF_FITS_RECORD_LEN);
		fits_blocks  = MIN( fits_blocks, DF_RW_BUFFER_SIZE/DF_FITS_RECORD_LEN);
		byte_req     = fits_blocks * DF_FITS_RECORD_LEN;

		/*  Read More Data */
		if( is_socket )
			byte_recd = sock_read_data(fd, (char*)fits_buf, byte_req, TRUE, 1000);
		else
			byte_recd = read(fd, (char*)fits_buf, byte_req);
	
		if( byte_req != byte_recd )
		{  
			return ERR_FILE_READ;
		}
	
		if( bufp->bitpix == DF_BITPIX_CHAR )
		{
			float pixel;
	      unsigned char  *src = (unsigned char *)fits_buf;

			/* copy to bufp->fdata */
			for( i=0; i < pixels_per_record && pixels_left; i++)
			{
				pixel = *src++;
				*dest++ = pixel;
				pixels_left--;
			}
		}
		else if( bufp->bitpix == DF_BITPIX_SHORT )
		{
			float pixel;
	      short  *src = (short *)fits_buf;

			/* copy to bufp->fdata */
			for( i=0; i < pixels_per_record && pixels_left; i++)
			{ 
if( DEBUG && (loop==0) && (i<5)) printf(" D[%d]=%d 0x%x \n", i, ntohs(*src), ntohs(*src));
				pixel = (short) ntohs(*src++);
				*dest++ = pixel;
				pixels_left--;
			}
		}
		else if( bufp->bitpix == DF_BITPIX_LONG )
		{
			float pixel;
	      int32_t  *src = (int32_t *)fits_buf;

			/* copy to bufp->fdata */
			for( i=0; i < pixels_per_record && pixels_left; i++)
			{
				pixel = (int32_t) ntohl(*src++); 
				*dest++ = pixel;
				pixels_left--;
			}
		}
		else if( bufp->bitpix == DF_BITPIX_FLOAT )
		{
			union u_df_lf pixel;
	      uint32_t  *src = (uint32_t *)fits_buf;

			/* copy to bufp->fdata */
			for( i=0; i < pixels_per_record && pixels_left; i++)
			{
				pixel.l =  ntohl( *src++); 
				*dest++ = pixel.f;
				pixels_left--;
			}
		}
		else
         return ERR_INV_FORMAT;
	}

   /* Apply bzero / bscale to data */
	if( (bscale != 1.0) || (bzero != 0) )
	{
		/* printf("Applying bscale %f & bzero %f \n", bscale, bzero); */
		dest  = bufp->fdata;
		for( i=0; i < bufp->N; i++ )
		{
			*dest = (*dest * bscale) + bzero;
if( DEBUG && i<5) { int idest = *dest; printf("S[%d]=%d 0x%x \n", i, idest, idest); }
			dest++;
		}
	}

   /* fill in stats */
   df_stats( bufp );
	/***
	printf("   min=%f max=%f mean=%f stddev=%f\n",
		bufp->min, bufp->max, bufp->mean, bufp->stddev);
	***/

	return ERR_NONE;
}

/************************ eof ************************/
