/**************************************************************************
**  print.c - produces postscript output for each dpy type.
**
** Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php
** Copyright (C) 2012 <NASA IRTF. http://irtfweb.ifa.hawaii.edu>
***************************************************************************
*/
#define EXTERN extern

/*--------------------------
 *  Standard include files
 *--------------------------
 */

#include <sys/time.h>

#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <math.h>
#include <time.h>

#include <gtk/gtk.h>

#if USE_GSLFIT
#include "fitgsl.h"
#endif 

/*-----------------------------
 *  Non-standard include files
 *-----------------------------
 */

#include "dv.h"           /* DV MAIN APPLICATION  */

/*--------------------------------------------------------------------------------
**  print_psheader() - prints the file header for DV's postscript output file
**                     and produces the letter head
**--------------------------------------------------------------------------------
*/
int print_psheader( FILE *fp )
{
   int  nitems,
        fontscale,
        rm, lm,
        y;
   long l;
   char buf[256];
   char tmstr[30];
   FILE * proc_fp;

   time(&l);
   strxcpy(tmstr, ctime(&l), sizeof(tmstr));
   unpad(tmstr, '\n');
   fprintf( fp, "%%!PS-Adobe-1.0\n");
   fprintf( fp, "%%%%BoundingBox: 0 0 612 792\n");
   fprintf( fp, "%%%%Title: dv output\n");
   fprintf( fp, "%%%%Creator: dv\n");
   fprintf( fp, "%%%%CreationDate: %s\n", tmstr);
   fprintf( fp, "%%%%EndComments\n");
   fprintf( fp, "%%%%Pages: 1\n");
   fprintf( fp, "%%%%EndProlog\n");
   fprintf( fp, "%%%%Page: 1 1\n");
   fprintf( fp, "%%\n%%\n");

   /*
   **  Read append file of postscript procedures. File is 'ps_proc' in $DVHOME.
   */
   cat_pathname( buf, Lc.app_home, "ps_proc", sizeof(buf));
   if( NULL == (proc_fp = fopen( buf, "rt")) )
      return ERR_FILE_READ;

   while( 0 < ( nitems = fread( buf, sizeof(char), sizeof(buf), proc_fp)) )
      fwrite( buf, sizeof(char), nitems, fp );
   fclose( proc_fp );
   /*
   **  Write the date on 'letter head' portion.
   */
   y = 10 * 72;       /* y position   */
   lm = 0.5*72;       /* left margin  */
   rm = 8.0*72;       /* right margin */

   fontscale = 10;
   fprintf( fp, "/Helvetica findfont %d scalefont setfont\n", fontscale);
   fprintf( fp, "%d %d (%s) rtextproc\n", rm, y+2, tmstr);
   return ERR_NONE;
}


/*--------------------------------------------------------------------------------
**  print_fitsheader() - prints the fits header of the file.
**--------------------------------------------------------------------------------
*/
int print_fitsheader(
   FILE *fp,                   /* out to this file pointer */
   struct dpy_t    *dp         /* display pointer */
)
{
   int y, ymin,
       lm, rm,
       done,
       lineno,
       fontscale,
       maxlines,
       col,
       l;

   char buf[85];
   char buf2[90];
   char * cptr;

   struct df_fheader_t  *fheader;
   struct df_buf_t *bp;            /* buffer pointer */

   bp = &Buffer[dp->bufinx];

   fontscale = 8;
   fprintf( fp, "/Courier findfont %d scalefont setfont\n", fontscale);

   y = (10 * 72) - 2*fontscale; /* y position   */
   ymin = 6.5 * 72;             /* Text can't extent below this line */
   maxlines = ((y-ymin) / fontscale) + 1;

   fprintf( fp, "gsave\n");
   if( bp->Nheader <= maxlines )
   {
      /* Set up for 1 columns on the page */
      fprintf( fp, "%% 1 column mode\n");
      col = 2;           /* Set col=2 so it will not do a second colmun */
      lm = 1.00*72;      /* lineno margin  */
      rm = 8.00*72;      /* text margin    */
   }
   else
   {
      /* Set up for 2 columns on the page */
      l = 4.25 * 72;
      fprintf( fp, "%d %d %d %d lineproc stroke\n", l, y+fontscale, l, ymin);

      col = 1;           /* this is the first column */
      lm = 0.50*72;      /* lineno margin  */
      rm = 4.15*72;      /* text margin    */

      /* Set up clipping boundaries */
      fprintf( fp, "%% Setup clipping boundaries for 2 column mode\n");
      fprintf( fp, "%d %d %d %d boxproc clip\n", lm, ymin, rm-lm, (y+fontscale) - ymin);
   }

   lineno = 1;
   fheader = bp->fheader;
   done = FALSE;
   while( !done && fheader != NULL )
   {
      cptr = (char*)fheader->buf;
      for( l=0; l < (DF_FITS_RECORD_LEN/80) && !done; l++)
      {
         /* Get line from buffer and print */
         memcpy( buf, cptr, 80);
         buf[80] = 0x00;
         unpad( buf, ' ');
			fix_header_string( buf2, buf );
         fprintf( fp, "%d %d (%s) ltextproc\n", lm, y, buf2);

         /* Set done flag on END keywork */
         if( strncmp( buf, "END", 3) == 0)
            done = TRUE;

         /* Increment line number and buffer pointer */
         lineno++;
         if( lineno > bp->Nheader )
            done = TRUE;
         cptr += 80;

         /* Increment to next postion on page */
         y -= fontscale;
         if( y <= ymin )
         {
            col++;                      /* It must be the second column */
            if( col > 2 )
               done = TRUE;
            else
            {
               /* re-set margins */
               y = (10 * 72) - 2*fontscale;
               lm += 4.00 * 72;
               rm += 4.00 * 72;
               /* Set clipping */
               fprintf( fp, "grestore\ngsave  %% Set new clipping area\n");
               fprintf( fp, "%d %d %d %d boxproc clip\n", lm, ymin, rm-lm, (y+fontscale) - ymin);
            }
         }

      }

      fheader = fheader->next;   /* get next block of data */
   }

   if( col > 1 )
      fprintf(fp, "grestore\n");

   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  fix_header_string() - copies a string from s to d, replace '(' &  ')'  with
**     '\(' and '\)'. Feeding unmatched () into postscript is BAD. Some FITS header
**     have unmatching ().
**--------------------------------------------------------------------------------
*/
void fix_header_string( char *d, char *s )
{
   while( *s )
	{
	   if( (*s == '(') || ( *s == ')') )
		   *d++ = '\\';
	   *d++ = *s++;
	}
	*d++ = '\0';  // terminate string.
}

/*--------------------------------------------------------------------------------
**  print_gdummy() - dummy graph print procedure.
**--------------------------------------------------------------------------------
*/
int print_gdummy(
   FILE *fp,                   /* out to this file pointer */
   struct dpy_t    *dp         /* display pointer */
)
{
   int fontscale,
       bmargin, tmargin, rmargin, lmargin;

   fontscale = 12;
   fprintf( fp, "/Helvetica findfont %d scalefont setfont\n", fontscale);

   tmargin = 6.5 * 72;      /* define margin to be used for graph area */
   bmargin = 0.5 * 72;
   lmargin = 0.5 * 72;
   rmargin = 8.0 * 72;

   fprintf( fp, "%d %d %d %d boxproc stroke\n",
            lmargin, bmargin, rmargin-lmargin, tmargin-bmargin);

   fprintf( fp, "%d %d (Draw Graph here!) ctextproc\n",
            lmargin + (rmargin-lmargin)/2, bmargin + (tmargin-bmargin)/2);

   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  print_gray_image() - output postscript to display image
**--------------------------------------------------------------------------------
*/
int print_gray_image(
   FILE *fp,                   /* out to this file pointer */
   struct dpy_t    *dp         /* display pointer */
)
{
   int  beg, end, i,
        pix_wid,
        pix_hgt,
        fontscale;
   struct df_buf_t *bp;        /* buffer pointer */

   #define NUM_GRAY 256  /* 256 is the number of gray scale used of ps dump */
   short gray_map[NUM_GRAY];

   bp = &Buffer[dp->bufinx];

   /*-----------------------------
   ** setup a gray scale colormap
   */
   beg = (CM.center * NUM_GRAY) - (CM.width * NUM_GRAY);
   end = (CM.center * NUM_GRAY) + (CM.width * NUM_GRAY);

   /* Fill in point before beg with 0 (black) */
   for( i=0; i < beg && i < NUM_GRAY; i++)
      gray_map[i] = ( Lc.cminverse ? 255 : 0);
   /* Map the color 0 to 255 between beg and end */
   for( i=MAX(beg,0); i < end && i < NUM_GRAY; i++)
   {
      if( Lc.cminverse )
         gray_map[i] = map(i, beg, end, 255, 0);
      else
         gray_map[i] = map(i, beg, end, 0, 255);
   }
   /* Fill in point after end with 255 (white) */
   for( i=end; i < NUM_GRAY; i++)
      gray_map[i] = ( Lc.cminverse ? 0 : 255);

   /*------------------------------------------------------------
   ** determine the size of the image to be displayed (pix_wid, pix_hgt);
   */
   {
      int da_wid, da_hgt;

      gdk_drawable_get_size( dp->data_drawingarea->window, &da_wid, &da_hgt);

      if( dp->image_zoom > 0 )
      {
         pix_wid = da_wid/dp->image_zoom;
         pix_hgt = da_hgt/dp->image_zoom;
      }
      else
      {
         int zoom = abs(dp->image_zoom) + 1;
         pix_wid = da_wid*zoom;
         pix_hgt = da_hgt*zoom;
      }

      pix_wid = MIN( pix_wid, bp->naxis1 - dp->image_offx );
      pix_hgt = MIN( pix_hgt, bp->naxis2 - dp->image_offy );
   }

   /*------------------------------------------------------------
   ** Dump image as postscript
   */
   {
      int x, y,
          len,
          color;
      float block,
            data;

      fprintf( fp, "%% Print a bitmap\ngsave\n");
      fprintf( fp, "%d %d translate %% translate orgin to lower-left of image\n", 72/2, 72/2);
      len = MAX(pix_wid, pix_hgt);
      fprintf( fp, "%d %d scale  %% image size\n", (72*6*pix_wid)/len, (72*6*pix_hgt)/ len);
      fprintf( fp, "%d %d bitdumpproc\n", pix_wid, pix_hgt);


      block = (dp->image_max-dp->image_min)/ (float)NUM_GRAY;
      for( y = dp->image_offy; y < dp->image_offy+pix_hgt; y++ )
      {
         i = (y * bp->naxis1) + dp->image_offx;

         for( x = dp->image_offx; x < dp->image_offx+pix_wid; x++ )
         {
            data = dfdatainx( bp, i );

            if( data >= dp->image_max )
               color = NUM_GRAY-1;
            else if( data <= dp->image_min )
               color = 0;
            else
               color = (data - dp->image_min ) / block;

            fprintf( fp, "%02x", gray_map[color]);
            i++;   /* increment index */

         }
         fprintf( fp, "\n");
      }
      fprintf( fp, "grestore\n");
   }

   /*-------------------------
   ** display the color scale
   */
   fprintf( fp, "%% Print a color scale\ngsave\n");
   fprintf( fp, "%d %d translate %% translate orgin to lower-left\n",(int)(7*72), 72/2);
   fprintf( fp, "%d %d scale     %% size the image\n", 72/4, 72*6);
   fprintf( fp, "%d %d bitdumpproc\n", 1, 256);
   for( i=255; i>=0; i--)
      fprintf( fp, "%02x\n", gray_map[i]);
   fprintf( fp, "grestore\n");
   fprintf( fp, "%d %d %d %d boxproc stroke\n", 7*72, 72/2, 72/4, 72*6);

   fontscale = 8;
   fprintf( fp, "/Helvetica findfont %d scalefont setfont\n", fontscale);
   fprintf( fp, "%d %d (%g) ltextproc\n", (int)(7.30*72), (int)(72*0.5-fontscale),
                                          dp->image_min);
   fprintf( fp, "%d %d (%g) ltextproc\n", (int)(7.30*72), (int)(72*6.5), dp->image_max);
   fprintf( fp, "%d %d (%g) ltextproc\n", (int)(7.30*72), (int)(72*3.5),
            dp->image_min + (dp->image_max-dp->image_min)/2.0);
   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  print_color_image() - output color postscript to display image
**--------------------------------------------------------------------------------
*/
int print_color_image(
   FILE *fp,                   /* out to this file pointer */
   struct dpy_t    *dp         /* display pointer */
)
{
   int  i,
        pix_wid,
        pix_hgt,
        fontscale;

   struct df_buf_t *bp = &Buffer[dp->bufinx];

   /*------------------------------------------------------------
   ** determine the size of the image to be displayed
   */
   {
      int da_wid, da_hgt;

      gdk_drawable_get_size( dp->data_drawingarea->window, &da_wid, &da_hgt);

      if( dp->image_zoom > 0 )
      {
         pix_wid = da_wid/dp->image_zoom;
         pix_hgt = da_hgt/dp->image_zoom;
      }
      else
      {
         int zoom = abs(dp->image_zoom) + 1;
         pix_wid = da_wid*zoom;
         pix_hgt = da_hgt*zoom;
      }

      pix_wid = MIN( pix_wid, bp->naxis1 - dp->image_offx );
      pix_hgt = MIN( pix_hgt, bp->naxis2 - dp->image_offy );
   }

   /*------------------------------------------------------------
   ** Dump image as postscript
   */
   {
      int x, y,
          len,
          color;
      float block,
            data;

      fprintf( fp, "%% Print a bitmap\ngsave\n");
      fprintf( fp, "%d %d translate %% translate orgin to lower-left of image\n", 72/2, 72/2);
      len = MAX(pix_wid, pix_hgt);
      fprintf( fp, "%d %d scale  %% image size\n", (72*6*pix_wid)/len, (72*6*pix_hgt)/ len);
      fprintf( fp, "%d %d colordumpproc\n", pix_wid, pix_hgt);

      block = (dp->image_max-dp->image_min) /  ((float)CM_NUM_RW_COLORS);
      for( y = dp->image_offy; y < dp->image_offy+pix_hgt; y++ )
      {
         i = (y * bp->naxis1) + dp->image_offx;

         for( x = dp->image_offx; x < dp->image_offx+pix_wid; x++ )
         {
            data = dfdatainx( bp, i);

            if( data >= dp->image_max )
               color = CM_NUM_COLORS-1;      /* last color */
            else if( data <= dp->image_min )
               color =  CM_NUM_STATIC_COLORS; /* first color */
            else
               color = ((data - dp->image_min ) / block) + CM_NUM_STATIC_COLORS;


            if( CM.visual->type == GDK_VISUAL_TRUE_COLOR )
               fprintf( fp, "%06x", color_ps_rgb( CM.colors[color].red,
						CM.colors[color].green, CM.colors[color].blue));
            else
               fprintf( fp, "%02x%02x%02x",
                  CM.colors[color].red/256,
                  CM.colors[color].green/256,
                  CM.colors[color].blue/256);

            i++;   /* increment index */

         }
         fprintf( fp, "\n");
      }
      fprintf( fp, "grestore\n");
   }
   /*-------------------------
   ** display the color scale
   */
   fprintf( fp, "%% Print a color scale\ngsave\n");
   fprintf( fp, "%d %d translate %% translate orgin to lower-left\n",(int)(7*72), 72/2);
   fprintf( fp, "%d %d scale     %% size the image\n", 72/4, 72*6);
   fprintf( fp, "%d %d colordumpproc\n", 1, CM_NUM_RW_COLORS);
   for( i=CM_NUM_COLORS-1; i >= CM_NUM_STATIC_COLORS; i-- )
   {
      if( CM.visual->type == GDK_VISUAL_TRUE_COLOR )
			fprintf( fp, "%06x", color_ps_rgb( CM.colors[i].red,
						CM.colors[i].green, CM.colors[i].blue));
      else
         fprintf( fp, "%02x%02x%02x\n",
                  CM.colors[i].red/256,
                  CM.colors[i].green/256,
                  CM.colors[i].blue/256);
   }

   fprintf( fp, "grestore\n");
   fprintf( fp, "%d %d %d %d boxproc stroke\n", 7*72, 72/2, 72/4, 72*6);

   fontscale = 8;
   fprintf( fp, "/Helvetica findfont %d scalefont setfont\n", fontscale);
   fprintf( fp, "%d %d (%g) ltextproc\n", (int)(7.30*72), (int)(72*0.5-fontscale),
                                          dp->image_min);
   fprintf( fp, "%d %d (%g) ltextproc\n", (int)(7.30*72), (int)(72*6.5), dp->image_max);
   fprintf( fp, "%d %d (%g) ltextproc\n", (int)(7.30*72), (int)(72*3.5),
            dp->image_min + (dp->image_max-dp->image_min)/2.0);

   return ERR_NONE;
}


/*-----------------------------------------------------------------------
**   color_ps_rgb( ) - converts the hex BGR (used in screen colormaps)
**      to RGB (need by postscript output).
**-----------------------------------------------------------------------
*/
uint32_t color_ps_rgb( int red, int green, int blue )
{
   uint32_t newcolor;
   newcolor = ( (red   << 16) & 0x00ff00ff ) | 
              ( (green <<  8) & 0x0000ff00 ) | 
              (  blue         & 0x000000ff ); 
   return newcolor;
}

/*--------------------------------------------------------------------------------
**  print_histogram() - Print the histogram
**--------------------------------------------------------------------------------
*/
int print_histogram(
   FILE *fp,                   /* out to this file pointer */
   struct dpy_t    *dp         /* display pointer */
)
{
   int fontscale,
       bmargin, tmargin, rmargin, lmargin,
       xmin, xmax, ymin, ymax,
       x1, x2, y1, y2,
       step, i,
       rc;
   float max_percent,
         f;

   struct hist_info hist;        /* histogram info */

   /*-----------------------------------
   ** Get Histogram info
   */
   if( ERR_NONE != (rc = get_histogram_info( dp, &hist )) )
      return rc;

   /*--------------------------
   **  Dimension graph area.
   */
   tmargin = 6.5 * 72;      /* define margin to be used for graph area */
   bmargin = 0.5 * 72;
   lmargin = 0.5 * 72;
   rmargin = 8.0 * 72;

   xmin =  1.5 * 72;
   xmax =  7.5 * 72;
   ymin =  1.5 * 72;
   ymax =  6.0 * 72;

   /* display header */
   fontscale = 20;
   fprintf( fp, "/Helvetica findfont %d scalefont setfont\n", fontscale);
   fprintf( fp, "%d %d (Histogram) rtextproc\n", xmax, ymax+1);

   fontscale = 12;
   fprintf( fp, "/Helvetica findfont %d scalefont setfont\n", fontscale);

   fprintf( fp, ".8 setgray\n");
   fprintf( fp, "%d %d %d %d boxproc fill\n", xmin, ymax, xmax-xmin, ymin-ymax);
   fprintf( fp, "0 setgray\n");
   fprintf( fp, "%d %d %d %d boxproc stroke\n", xmin, ymax, xmax-xmin, ymin-ymax);

   /*-----------------------------------
   ** Find max percent to nearest 5%
   */
   max_percent = (100.0*hist.max_bin_value) / hist.pixel_total;
   max_percent = (floor(max_percent/5.0)+1) * 5.0;

   /*-----------------------------------
   ** Label Y axis
   */
   fprintf( fp, "%% Label Y Axis\n");
   x1 = xmin - (0.2 * 72);
   step = (max_percent > 25 ? 10 : 5 );
   for( i=0; i<= max_percent; i+=step )
   {
      y1 = (int) map( i, 0.0, max_percent, ymin, ymax);
     fprintf( fp, "%d %d %d %d lineproc stroke\n", x1, y1, xmax, y1);
      fprintf( fp, "%d %d (%3d%%) rtextproc\n", x1, y1, i);
   }
   y1 = ymin + (ymax-ymin)/2;
   x1 = .75 * 72;
   fprintf( fp, "gsave\n");
   fprintf( fp, "%d %d translate\n", x1, y1);
   fprintf( fp, "90 rotate\n");
   fprintf( fp, "0 0 (Percent) ltextproc\n");
   fprintf( fp, "grestore\n");

   /*-----------------------------
   **  Draw X axis.
   */
   fprintf( fp, "%% Label X Axis\n");
   for( i=0; i <= 100; i += 25)
   {
      f = map( i, 0, 100, dp->image_min, dp->image_max );
      x1 = (int) map( i, 0, 100, xmin, xmax);
      y1 = ymin - ( 0.2 * 72 );
      fprintf( fp, "%d %d %d %d lineproc stroke\n", x1, ymin, x1, y1);
      y1 -= fontscale;
      fprintf( fp, "%d %d (%1.0f) ctextproc\n", x1, y1, f);
   }
   y1 -= fontscale;
   fprintf( fp, "%d %d (Pixel Values) ctextproc\n", xmin+(xmax-xmin)/2, y1);

   /*------------------
   **  Draw histogram
   */
   for( i=0; i < hist.num_of_bins; i++)
   {
      x1 = map( i, 0, hist.num_of_bins, xmin, xmax);
      x2 = map( i+1, 0, hist.num_of_bins, xmin, xmax);
      y2 = map( 100.0*hist.bins[i]/hist.pixel_total, 0, max_percent, ymin, ymax);
      if( y2 > ymin )
      {
         fprintf( fp, ".5 setgray\n");
         fprintf( fp, "%d %d %d %d boxproc fill\n", x1, ymin, x2-x1, y2-ymin);
         fprintf( fp, "0 setgray\n");
         fprintf( fp, "%d %d %d %d boxproc stroke\n", x1, ymin, x2-x1, y2-ymin);
      }

   }

   x1 = 5 * 72;
   y1 = bmargin + 2*fontscale;
   fprintf( fp, "%d %d (%5.1f%% of the data is graphed.) ltextproc\n",
                 x1, y1, 100.0*hist.pixel_graph/hist.pixel_total);
   y1 -= fontscale;
   fprintf( fp, "%d %d (%5.1f%% is less than low range.) ltextproc\n",
                 x1, y1, 100.0*hist.pixel_low/hist.pixel_total);
   y1 -= fontscale;
   fprintf( fp, "%d %d (%5.1f%% is greater than high range.) ltextproc\n",
                 x1, y1, 100.0*hist.pixel_high/hist.pixel_total);

   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  print_linecut() - Print the linecut
**--------------------------------------------------------------------------------
*/
int print_linecut(
   FILE *fp,                   /* out to this file pointer */
   struct dpy_t    *dp         /* display pointer */
)
{
   int fontscale,
       xmin, xmax, ymin, ymax,
       x1, x2, y1, y2,
       idata, i,
       rc;
   float f;

   struct df_buf_t *bp;
   struct lcut_info lcut;        /* linecut info */

   bp = &Buffer[dp->bufinx];    /* buffer pointer */

   /*-----------------------------------
   ** Get linecut info
   */
   if( ERR_NONE != (rc = get_linecut_info( dp, &lcut )) )
   {
      return rc;
   }

   /*--------------------------
   **  Dimension graph area.
   */
   xmin =  1.5 * 72;
   xmax =  7.5 * 72;
   ymin =  1.5 * 72;
   ymax =  6.0 * 72;

   /* display header */
   fontscale = 12;

   fprintf( fp, "/Helvetica findfont %d scalefont setfont\n", fontscale);
   fprintf( fp, "1 setgray\n");
   fprintf( fp, "%d %d %d %d boxproc fill\n", xmin, ymax, xmax-xmin, ymin-ymax);
   fprintf( fp, "0 setgray\n");
   fprintf( fp, "%d %d %d %d boxproc stroke\n", xmin, ymax, xmax-xmin, ymin-ymax);
   fprintf( fp, "%d %d (LineCut Axis: X %d  Y %d) rtextproc\n",
                 xmax, ymax + 2*fontscale, dp->lcut_x, dp->lcut_y);
   /*
   **  Label Y axis.
   */
   fprintf( fp, "%% Label Y Axis\n");
   for( i=0; i <= 100; i+=25 )
   {
      /*
      **  Values of pixels for NAXIS1 (Red)
      */
      x1 = xmin - (0.1 * 72);
      y1 = map( i, 0, 100, ymin, ymax) + 0.5;
      fprintf( fp, "%d %d %d %d lineproc stroke\n", xmin, y1, x1, y1);

      f = map( i, 0, 100, lcut.range_min, lcut.range_max);
      fprintf( fp, "%d %d (%3.1f) rtextproc\n", x1, y1, f);
      /*
      **  Index of pixel for NAXIS2 (GREEN)
      */
      x1 = xmax+(0.1*72);
      fprintf( fp, "%d %d %d %d lineproc stroke\n", xmax, y1, x1, y1);
      idata = map( i, 0, 100, lcut.ymax, lcut.ymin);
      fprintf( fp, "%d %d (%3d) ltextproc\n", x1, y1, idata);
   }
   y1 = ymin + (ymax-ymin)/2;
   x1 = .75 * 72;
   fprintf( fp, "gsave\n");
   fprintf( fp, "%d %d translate\n", x1, y1);
   fprintf( fp, "90 rotate\n");
   fprintf( fp, "0 0 (Values along X Axis) ctextproc\n");
   fprintf( fp, "grestore\n");

   y1 = ymin + (ymax-ymin)/2;
   x1 = 8.2 * 72;
   fprintf( fp, "gsave\n");
   fprintf( fp, "%d %d translate\n", x1, y1);
   fprintf( fp, "90 rotate\n");
   fprintf( fp, "0 0 (%d, Y) ctextproc\n", dp->lcut_y);
   fprintf( fp, "grestore\n");

   /*
   **  Draw X axis.
   */
   fprintf( fp, "%% Label X Axis\n");
   for( i=0; i <= 100; i+=25 )
   {
      /*
      **  Indexes of pixels for NAXIS1 (Red)
      */
      x1 = map( i, 0, 100, xmin, xmax);
      y1 = ymin - ( 0.1 * 72 );
      fprintf( fp, "%d %d %d %d lineproc stroke\n", x1, ymin, x1, y1);
      y1 -= fontscale;

      idata = map( i, 0, 100, lcut.xmin, lcut.xmax);
      fprintf( fp, "%d %d (%3d) ctextproc\n", x1, y1, idata);
      /*
      **  Values of pixel for NAXIS2 (Green)
      */
      y1 = ymax + ( 0.1 * 72 );
      fprintf( fp, "%d %d %d %d lineproc stroke\n", x1, ymax, x1, y1);

      f = map(i, 0, 100, lcut.range_min, lcut.range_max);
      fprintf( fp, "%d %d (%3.1f) ctextproc\n", x1, y1, f);
   }

   x1 = xmin+(xmax-xmin)/2;
   y1 = 0.5 * 72;
   fprintf( fp, "%d %d (X, %d) ctextproc\n", x1, y1, dp->lcut_x);
   y1 = ymax + 2*fontscale;
   fprintf( fp, "%d %d (Values along Y Axis) rtextproc\n", x1, y1);

   /*
   **  Draw Profile for NAXIS1. (Red)
   */
   x2 = xmin;
   y2 = ymin;

   for( i = lcut.xmin; i <= lcut.xmax; i++ )
   {
      x1 = map( i+0.5, lcut.xmin, lcut.xmax, xmin, xmax);

      f = dfdataxy( bp, i, lcut.yaxis);

      if( f > lcut.range_max )
         y1 = ymax;
      else if( f < lcut.range_min )
         y1 = ymin;
      else
         y1 = map( f, lcut.range_min, lcut.range_max, ymin, ymax);

      fprintf( fp, "%d %d %d %d lineproc stroke\n", x2, y2, x2, y1);
      fprintf( fp, "%d %d %d %d lineproc stroke\n", x2, y1, x1, y1);

      x2 = x1;
      y2 = y1;
   }

   /*
   ** Draw Profile for NAXIS2 (Green).
   */
   x2 = xmin;
   y2 = ymax;

   for( i = lcut.ymin;  i <= lcut.ymax; i++)
   {
      y1 = map( i+0.5, lcut.ymin, lcut.ymax, ymax, ymin);

      f = dfdataxy( bp, lcut.xaxis, i);

      if( f > lcut.range_max )
         x1 = xmax;
      else if( f < lcut.range_min )
         x1 = xmin;
      else
         x1 = map( f, lcut.range_min, lcut.range_max, xmin, xmax);

      fprintf( fp, "%d %d %d %d lineproc stroke\n", x2, y2, x2, y1);
      fprintf( fp, "%d %d %d %d lineproc stroke\n", x2, y1, x1, y1);

      x2 = x1;
      y2 = y1;
   }

   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  print_noise() - noise print procedure.   hack
**--------------------------------------------------------------------------------
*/
int print_noise(
   FILE *fp,                  /* out to this file pointer */
   struct dpy_t *dp           /* display pointer */
)
{
   int xmin, xmax,          /* Graph area min, max, & size in pixels */
       ymin, ymax,
       x1, y1, y2,
       min_rmar,
       max_rmar,
       mean_rmar,
       std_rmar,
       n_rmar,
       mod,
       nrows,
       fontscale;
   float  range_min, range_max;

   struct df_buf_t *bp;

   struct noise_t *noisep;
   struct noise_t fnoise;

   bp = &Buffer[dp->bufinx];

   /*
   **  Allocate memory for data.
   */
   if( NULL == ( noisep = (struct noise_t *) calloc( dp->noise_mod, sizeof(struct noise_t)) ) )
   {
      return ERR_MEM_ALLOC;
   }
   memset( (char*) &fnoise, 0x00, sizeof(fnoise));

   /*
   **  Calculate noise
   */
   calc_noise( noisep, &fnoise, dp, bp );

   /* display header */
   fontscale = 14;
   xmin = 1.5 * 72;
   ymin = 6.0 * 72;
   fprintf( fp, "/Helvetica findfont %d scalefont setfont\n", fontscale);
   fprintf( fp, "%d %d (Noise Plots) ltextproc\n", xmin, ymin);

   /*
   ** Display text of max, min, mean, std, ...
   */
   min_rmar  = 1.7*72;
   max_rmar  = 2.3*72;
   mean_rmar = 2.87*72;
   std_rmar  = 3.4*72;
   n_rmar    = 4*72;

   fontscale = 10;
   ymin = 5.5 * 72;
   fprintf( fp, "/Helvetica findfont %d scalefont setfont\n", fontscale);
   fprintf( fp, "%d %d (Min) rtextproc\n", min_rmar, ymin);
   fprintf( fp, "%d %d (Max) rtextproc\n", max_rmar, ymin);
   fprintf( fp, "%d %d (Mean) rtextproc\n", mean_rmar, ymin);
   fprintf( fp, "%d %d (STD) rtextproc\n", std_rmar, ymin);
   fprintf( fp, "%d %d (N) rtextproc\n", n_rmar, ymin);

   ymin -= fontscale;
   fprintf( fp, "%d %d (Array) ltextproc\n", 72/2, ymin);
   fprintf( fp, "%d %d (%3.1f) rtextproc\n", min_rmar, ymin, fnoise.min);
   fprintf( fp, "%d %d (%3.1f) rtextproc\n", max_rmar, ymin, fnoise.max);
   fprintf( fp, "%d %d (%3.1f) rtextproc\n", mean_rmar, ymin, fnoise.mean);
   fprintf( fp, "%d %d (%3.1f) rtextproc\n", std_rmar, ymin, fnoise.std);
   fprintf( fp, "%d %d (%d) rtextproc\n", n_rmar, ymin, fnoise.N);
   ymin -= fontscale;

   for( mod=0; mod < dp->noise_mod; mod++)
   {
      ymin -= fontscale;
      fprintf( fp, "%d %d (Col %02d) ltextproc\n", 72/2, ymin, mod);
      fprintf( fp, "%d %d (%3.1f) rtextproc\n", min_rmar, ymin, noisep[mod].min);
      fprintf( fp, "%d %d (%3.1f) rtextproc\n", max_rmar, ymin, noisep[mod].max);
      fprintf( fp, "%d %d (%3.1f) rtextproc\n", mean_rmar, ymin, noisep[mod].mean);
      fprintf( fp, "%d %d (%3.1f) rtextproc\n", std_rmar, ymin, noisep[mod].std);
      fprintf( fp, "%d %d (%d) rtextproc\n", n_rmar, ymin, noisep[mod].N);
   }

   /* draw min, max, mean, graph */
   ymin = 3.5*72;
   ymax = 6.0*72;
   xmin = 5.0*72;
   xmax = 8.0*72;

   fprintf( fp, ".8 setgray\n");
   fprintf(fp, "%d %d %d %d boxproc stroke\n", xmin, ymax, xmax-xmin, ymin-ymax);
   fprintf( fp, "0 setgray\n");

   if( dp->noise_autoscale )
   {
      range_min = fnoise.min;
      range_max = fnoise.max;
   }
   else
   {
      range_min = dp->noise_g1_min;
      range_max = dp->noise_g1_max;
   }
   nrows = dp->noise_mod+2;

   /* label Y axis */
   fprintf( fp, "%d %d (%3.1f) rtextproc\n", xmin, ymin, range_min);
   fprintf( fp, "%d %d (%3.1f) rtextproc\n", xmin, ymax, range_max);

   /* graph array stats */
   x1 = map( 1, 0 , nrows, xmin, xmax);
   y1 = map( fnoise.min, range_min, range_max, ymin, ymax);
   y2 = map( fnoise.max, range_min, range_max, ymin, ymax);
   fprintf( fp, "%d %d %d %d lineproc stroke\n", x1, y1, x1, y2);
   fprintf( fp, "%d %d %d %d lineproc stroke\n", x1-5, y1, x1+5, y1);
   fprintf( fp, "%d %d %d %d lineproc stroke\n", x1-5, y2, x1+5, y2);
   y1 = map( fnoise.mean, range_min, range_max, ymin, ymax);
   fprintf(fp, "%d %d %d %d boxproc stroke\n", x1-2, y1-2, 4, 4);

   /* graph column stats */
   for( mod=0; mod < dp->noise_mod; mod++)
   {
      x1 = map( mod+2, 0 , nrows, xmin, xmax);
      y1 = map( noisep[mod].min, range_min, range_max, ymin, ymax);
      y2 = map( noisep[mod].max, range_min, range_max, ymin, ymax);
      fprintf( fp, "%d %d %d %d lineproc stroke\n", x1, y1, x1, y2);
      fprintf( fp, "%d %d %d %d lineproc stroke\n", x1-5, y1, x1+5, y1);
      fprintf( fp, "%d %d %d %d lineproc stroke\n", x1-5, y2, x1+5, y2);
      y1 = map( noisep[mod].mean, range_min, range_max, ymin, ymax);
      fprintf(fp, "%d %d %d %d boxproc stroke\n", x1-2, y1-2, 4, 4);
   }

   /*
   **  Draw std graph.
   */
   /* get range */
   if( dp->noise_autoscale )
   {
      range_min = fnoise.std;
      range_max = fnoise.std;
      for( mod=0; mod < dp->noise_mod; mod++)
      {
         if( noisep[mod].std < range_min ) range_min = noisep[mod].std;
         if( noisep[mod].std > range_max ) range_max = noisep[mod].std;
      }
   }
   else
   {
      range_min = dp->noise_g2_min;
      range_max = dp->noise_g2_max;
   }

   /* draw boarder & y axis */
   ymin = 0.5*72;
   ymax = 3.0*72;
   xmin = 5.0*72;
   xmax = 8.0*72;

   fprintf( fp, ".8 setgray\n");
   fprintf(fp, "%d %d %d %d boxproc stroke\n", xmin, ymax, xmax-xmin, ymin-ymax);
   fprintf( fp, "0 setgray\n");

   fprintf( fp, "%d %d (%3.1f) rtextproc\n", xmin, ymin, range_min);
   fprintf( fp, "%d %d (%3.1f) rtextproc\n", xmin, ymax, range_max);
   /* Draw array std */

   x1 = map( 1, 0, nrows, xmin, xmax);
   y1 = map( fnoise.std, range_min, range_max, ymin, ymax);
   fprintf(fp, "%d %d %d %d boxproc stroke\n", x1-2, y1-2, 4, 4);

   for( mod=0; mod < dp->noise_mod; mod++)
   {
      x1 = map( mod+2, 0, nrows, xmin, xmax);
      y1 = map( noisep[mod].std, range_min, range_max, ymin, ymax);
      fprintf(fp, "%d %d %d %d boxproc stroke\n", x1-2, y1-2, 4, 4);
   }

   /*
   ** free memory
   */
   free( (char*) noisep);
   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  print_xcut() - xcut print procedure.
**--------------------------------------------------------------------------------
*/
int print_xcut(
        FILE *fp,                  /* out to this file pointer */
        struct dpy_t *dp           /* display pointer */
)
{
  int  fontscale,
       x1, y1, x2, y2,
       xmin, xmax,
       ymin, ymax,
       bmargin, tmargin, rmargin, lmargin,
       num_xcut_items,
       inx,
       i;
   float fdata,
         range_min, range_max;
   char buf[80];

   struct df_buf_t *bp;
   struct xcut_buf_t * xcut_buf;

   bp = &Buffer[dp->bufinx];

   /*
   **  Allocate memory for bins.
   */
   if( NULL == ( xcut_buf = (struct xcut_buf_t *) malloc( sizeof(struct xcut_buf_t)*NUM_PIXEL)) )
      return ERR_MEM_ALLOC;
   /*
   **  Using a digital differential analyser (DDA) algorithm, cycle through the data
   **  and place points in buffer, the min and max.
   */
   num_xcut_items = xcut_line ( dp->xcut_xbeg, dp->xcut_ybeg,
                                dp->xcut_xend, dp->xcut_yend,
                                bp, xcut_buf, NUM_PIXEL );

   /* Get Y scale & blocking factor */

   if( dp->xcut_autoscale )
   {
      range_min = DF_MAX_SIGNED_INT32;
      range_max = DF_MIN_SIGNED_INT32;
      for( i=0; i < num_xcut_items; i++)
      {
         fdata = xcut_buf[i].value;
         if( fdata < range_min ) range_min = fdata;
         if( fdata > range_max ) range_max = fdata;
      }
   }
   else
   {
      range_min = dp->xcut_min;
      range_max = dp->xcut_max;
   }

   if( (range_max-range_min) < 0.01 )
   {
      range_min += 0.01;
      range_min -= 0.01;
   }

   /*
   **  Dimension graph area.
   */
   tmargin = 6.5 * 72;      /* define margin to be used for graph area */
   bmargin = 0.5 * 72;
   lmargin = 0.5 * 72;
   rmargin = 8.0 * 72;

   xmin =  1.5 * 72;
   xmax =  7.5 * 72;
   ymin =  1.0 * 72;
   ymax =  6.0 * 72;

   /* display header */
   fontscale = 18;
   fprintf( fp, "/Helvetica findfont %d scalefont setfont\n", fontscale);
   fprintf( fp, "%d %d (XCUT (%d,%d) to (%d,%d)) ltextproc\n", xmin, ymax+1,
                dp->xcut_xbeg, dp->xcut_ybeg, dp->xcut_xend, dp->xcut_yend );

   fontscale = 12;
   fprintf( fp, "/Helvetica findfont %d scalefont setfont\n", fontscale);

   fprintf( fp, ".8 setgray\n");
   fprintf( fp, "%d %d %d %d boxproc fill\n", xmin, ymax, xmax-xmin, ymin-ymax);
   fprintf( fp, "0 setgray\n");
   fprintf( fp, "%d %d %d %d boxproc stroke\n", xmin, ymax, xmax-xmin, ymin-ymax);
   /*
   **  Label X Axis
   */
   fprintf( fp, "%% Label X Axis\n");
   for( i=0; i <= 100; i += 25)
   {
      x1 =  rint( map( i, 0.0, 100.0, xmin, xmax));
      y1 = ymin - ( 0.2 * 72 );
      fprintf( fp, "%d %d %d %d lineproc stroke\n", x1, ymin, x1, y1);
      y1 -= fontscale;
      inx = map( i, 0.0, 100.0, 0, num_xcut_items-1);
      fprintf( fp, "%d %d (%d,%d) ctextproc\n", x1, y1, xcut_buf[inx].x, xcut_buf[inx].y);
   }
   y1 -= fontscale;
   fprintf( fp, "%d %d (Pixel X,Y Location) ctextproc", xmin+(xmax-xmin)/2, y1);
   /*
   **  Draw Y Label
   */
   fprintf( fp, "%% Label Y Axis\n");
   x1 = xmin - (0.2 * 72);
   for( i=0; i<= 100; i+=25 )
   {
      y1 =  rint( map( i, 0.0, 100.0, ymin, ymax));
      fprintf( fp, "%d %d %d %d lineproc stroke\n", x1, y1, xmin, y1);
      fdata = map(i, 0.0, 100.0, range_min, range_max);
      double2str_len( buf, fdata, 6, 2);
      fprintf( fp, "%d %d (%s) rtextproc\n", x1, y1, buf);
   }
   y1 = ymin + (ymax-ymin)/2;
   x1 = .75 * 72;
   fprintf( fp, "gsave\n");
   fprintf( fp, "%d %d translate\n", x1, y1);
   fprintf( fp, "90 rotate\n");
   fprintf( fp, "0 0 (Pixel Values) ltextproc\n");
   fprintf( fp, "grestore\n");
   /*---------------------------------------
   **  Draw XLineCut
   */
   x2 = xmin;
   y2 = ymin;
   for( i=0; i < num_xcut_items; i++)
   {
      x1 = map( i+1, 0, num_xcut_items, xmin, xmax);
		y1 = map( xcut_buf[i].value, range_min, range_max, ymin, ymax);

      fprintf( fp, "%d %d %d %d lineproc stroke\n", x2, y2, x2, y1);
      fprintf( fp, "%d %d %d %d lineproc stroke\n", x2, y1, x1, y1);

      x2 = x1;
      y2 = y1;
   }
   fprintf( fp, "%d %d %d %d lineproc stroke\n", x2, y2, xmax, ymin);

   /*---------------------------------------
   ** For print, we always do the guassian fit
   */
#if USE_GSLFIT
   {
      int err;
      int use_x_axis;
      fitgsl_data *dat;
      float R[4];

      dat = fitgsl_alloc_data( num_xcut_items );  // allocate temporary data buffers

      // normally fit using x axis, unless y-axis is longer (which usually
      // means the used did a more vertical cut
      use_x_axis = TRUE;
      if( abs(dp->xcut_yend - dp->xcut_ybeg) > abs(dp->xcut_xend - dp->xcut_xbeg) )
         use_x_axis = FALSE;

      for( i=0; i<num_xcut_items; i++ )           // copy input dat dat.
      {
         dat->pt[i].x = (use_x_axis ? xcut_buf[i].x :  xcut_buf[i].y);
         dat->pt[i].y = xcut_buf[i].value;
      }
      err = fitgsl_lm( dat, R, FALSE );      // call routine to fit data.
      if( err )
         cc_printf( W.cmd.main_console, CM_BLUE, "fitgsl_lm() returned non-zero status\n");

      /*--------------------------
      ** output text information
      */
		fontscale = 12;
		fprintf( fp, "/Helvetica findfont %d scalefont setfont\n", fontscale);
      fprintf( fp, "%d %d (%s) rtextproc\n", xmax, ymax+1, 
               "y =  b + p * exp[0.5*((x-c)/w)^2] ");
      x1 = xmax;
      y1 = ymax;
      sprintf( buf, "Base = %5.2f ", R[FITGSL_B_INDEX]);
      fprintf( fp, "%d %d (%s) rtextproc\n", x1, y1-=fontscale, buf);
      sprintf( buf, "Center = %5.2f ", R[FITGSL_C_INDEX]);
      fprintf( fp, "%d %d (%s) rtextproc\n", x1, y1-=fontscale, buf);
      sprintf( buf, "Peak = %5.2f ", R[FITGSL_P_INDEX]);
      fprintf( fp, "%d %d (%s) rtextproc\n", x1, y1-=fontscale, buf);
      sprintf( buf, "Width = %5.2f ", R[FITGSL_W_INDEX]);
      fprintf( fp, "%d %d (%s) rtextproc\n", x1, y1-=fontscale, buf);
      sprintf( buf, "FWHM = %5.2f ", 2.35482*R[FITGSL_W_INDEX]);
      fprintf( fp, "%d %d (%s) rtextproc\n", x1, y1-=fontscale, buf);
      sprintf( buf, "Seeing = %5.2f ", 2.35482*R[FITGSL_W_INDEX]*bp->arcsec_pixel);
      fprintf( fp, "%d %d (%s) rtextproc\n", x1, y1-=fontscale, buf);
      sprintf( buf, "Area = %5.2f ", sqrt(2*M_PI) * R[FITGSL_P_INDEX] * R[FITGSL_W_INDEX]);
      fprintf( fp, "%d %d (%s) rtextproc\n", x1, y1-=fontscale, buf);


      /*--------------------------
      ** Draw graph of equation
      */
      for( i=0;  i < num_xcut_items-1; i++ )
      {
         /* point 1 */
         x1 = map( i, 0, num_xcut_items-1, xmin, xmax);
         fdata = fitgsl_fx( R[FITGSL_B_INDEX], R[FITGSL_P_INDEX],
                            R[FITGSL_C_INDEX], R[FITGSL_W_INDEX], dat->pt[i].x);
         y1 = map( fdata, range_min, range_max, ymin, ymax);

         /* point 2 */
         x2 = map( i+1, 0, num_xcut_items-1, xmin, xmax);
         fdata = fitgsl_fx( R[FITGSL_B_INDEX], R[FITGSL_P_INDEX],
                            R[FITGSL_C_INDEX], R[FITGSL_W_INDEX], dat->pt[i+1].x);
         y2 = map( fdata, range_min, range_max, ymin, ymax);

         fprintf( fp, "%d %d %d %d lineproc stroke\n", x1, y1, x2, y2);
      }

      /* Free allocated data */
      fitgsl_free_data(dat);
   }
#endif // USE_GSLFIT

   /*
   **  free memory.
   */
   free( (char*) xcut_buf);
   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  print_aofig_gray() - outputs aofigure as a gray scale.
**--------------------------------------------------------------------------------
*/
int print_aofig_gray(
   FILE *fp,                   /* out to this file pointer */
   struct dpy_t    *dp         /* display pointer */
)
{
   int  beg, end, i,
        fontscale;
   struct df_buf_t *bp;        /* buffer pointer */
	polygon_t *ptable;

   #define NUM_GRAY 256  /* 256 is the number of gray scale used of ps dump */
   short gray_map[NUM_GRAY];

   bp = &Buffer[dp->bufinx];

   /*-----------------------------
   ** setup a gray scale colormap
   */
   beg = (CM.center * NUM_GRAY) - (CM.width * NUM_GRAY);
   end = (CM.center * NUM_GRAY) + (CM.width * NUM_GRAY);

   /* Fill in point before beg with 0 (black) */
   for( i=0; i < beg && i < NUM_GRAY; i++)
      gray_map[i] = ( Lc.cminverse ? 255 : 0);
   /* Map the color 0 to 255 between beg and end */
   for( i=MAX(beg,0); i < end && i < NUM_GRAY; i++)
   {
      if( Lc.cminverse )
         gray_map[i] = map(i, beg, end, 255, 0);
      else
         gray_map[i] = map(i, beg, end, 0, 255);
   }
   /* Fill in point after end with 255 (white) */
   for( i=end; i < NUM_GRAY; i++)
      gray_map[i] = ( Lc.cminverse ? 0 : 255);

   /*-------------------------
   ** display the color scale
   */
   fprintf( fp, "%% Print a color scale\ngsave\n");
   fprintf( fp, "%d %d translate %% translate orgin to lower-left\n",(int)(7*72), 72/2);
   fprintf( fp, "%d %d scale     %% size the image\n", 72/4, 72*6);
   fprintf( fp, "%d %d bitdumpproc\n", 1, 256);
   for( i=255; i>=0; i--)
      fprintf( fp, "%02x\n", gray_map[i]);
   fprintf( fp, "grestore\n");
   fprintf( fp, "%d %d %d %d boxproc stroke\n", 7*72, 72/2, 72/4, 72*6);

   fontscale = 8;
   fprintf( fp, "/Helvetica findfont %d scalefont setfont\n", fontscale);
   fprintf( fp, "%d %d (%g) ltextproc\n", (int)(7.30*72), (int)(72*0.5-fontscale),
                                          dp->image_min);
   fprintf( fp, "%d %d (%g) ltextproc\n", (int)(7.30*72), (int)(72*6.5), dp->image_max);
   fprintf( fp, "%d %d (%g) ltextproc\n", (int)(7.30*72), (int)(72*3.5),
            dp->image_min + (dp->image_max-dp->image_min)/2.0);

   /*-------------------------
   ** display figure.
   */
   if( dp->aofig_format == AOFIG_FORMAT_DM )
      ptable = AO_DM_Table;
   else
      ptable = AO_Sensor_Table;

   {
      int xmin, xmax, ymin, ymax,
         aofig_x, aofig_y,
         e, p, x, y, c;
         float block, data;

      xmin = 1.0*72;
      xmax = 6.0*72;
      ymin = 1.0*72;
      ymax = 6.0*72;

		fprintf( fp, "0 setgray\n");
		fprintf( fp, "%d %d %d %d boxproc stroke\n", xmin, ymax, xmax-xmin, ymin-ymax);

		aofig_x = dp->aofig_x;
      aofig_y = dp->aofig_y;

      block = (dp->image_max - dp->image_min) / ((float)NUM_GRAY);

      for( e=0; e < AOFIG_NUM_ELEMENTS; e++ )
      {
         // Determine the data represent
         data = dfdataxy( bp, aofig_x, aofig_y );
         if( dp->aofig_data )
            aofig_y++;
         else
            aofig_x++;

         // translate data value to color.
         if( data >= dp->image_max )
            c = NUM_GRAY-1;  /* last color */
         else if( data <= dp->image_min )
            c = 0;
         else
            c = ( (data - dp->image_min) / block);

         // filled polygon
		   fprintf( fp, "%6.4f setgray\n", gray_map[c]/255.0);  
		   fprintf( fp, "[");
         for( p=0; p < ptable[e].n; p++)
         {
				x = rint( map( ptable[e].pt[p].x, 0, 1000, xmin, xmax));
				y = rint( map( ptable[e].pt[p].y, 0, 1000, ymin, ymax));
		      fprintf( fp, "[%d %d]", x, y);
         }
		   fprintf( fp, "] polygon_proc fill \n");
      }
   }


   return ERR_NONE;
}

/*----------------------------------------------------------------
**  print_sa - spectra graph type A print procedure
**----------------------------------------------------------------
*/
int  print_sa(
   FILE *fp,                   /* out to this file pointer */
   struct dpy_t    *dp         /* display pointer */
)
{
   int    x1, y1, x2, y2,
          xmin, xmax,      /* Graph area min, max, & size in pixels */
          ymin, ymax,
          fontscale,
          cnt,
          inx,
          l, i;

   int    numgraph, igraph;
   float  globol_min, globol_max,
          yscale_min, yscale_max,
          sum, n, sdd, mean, std;

   char buf[80];

   struct table_t {
      int   begrow;
      int   endrow;
      float mean[NUM_PIXEL];
      int   cnt[NUM_PIXEL];
      float max;
      float min;
   };

   struct table_t *object;
   struct table_t *sky;
	struct df_buf_t *bp;        /* buffer pointer */

	bp = &Buffer[dp->bufinx];

   /* Allocate memory */
   numgraph = (int) ceil( ((double)dp->sa_objbin_max-dp->sa_objbin_min+1) /
                          dp->sa_rows_per_bin );

   sky    = (struct table_t *) calloc( 1, sizeof(struct table_t));
   object = (struct table_t *) calloc( numgraph, sizeof(struct table_t));

   if( object==NULL || sky==NULL )
   {
      if( object != NULL) free( object );
      if( sky    != NULL) free( sky    );
      perror("Memory allocation error");
      return ERR_MEM_ALLOC;
   }

   /*
   **  Dimension graph area.
   */
   xmin =  72;
   xmax =  7.4*72;
   ymin =  72;
   ymax =  6.0*72;

   /*
   **  Display header
   */
   fontscale = 10;
   fprintf( fp, "/Helvetica findfont %d scalefont setfont\n", fontscale);
   fprintf( fp, "%d %d (SPECTRA A) rtextproc\n", (int)(7.3*72), (int)(6.1*72));
   /*
   **  Get the mean values for the sky
   */
   if( dp->sa_subtractsky )
   {
      sky->min = DF_MAX_SIGNED_INT32;
      sky->max = DF_MIN_SIGNED_INT32;
      sky->begrow = MAX(0, dp->sa_skybin_min);
      sky->endrow = MIN(dp->sa_skybin_max, bp->naxis2-1);

      for( x1 = MAX(0, dp->sa_xmin);
           x1 <= MIN(dp->sa_xmax, bp->naxis1-1); x1++)
      {
         /*  Sum the data along the x axis */
         sum = 0;
         cnt = 0;
         for( y1 = sky->begrow; y1 <= sky->endrow; y1++)
         {
            inx = y1 * bp->naxis1 + x1;
				sum += dfdatainx( bp, inx);
            cnt++;
         }

         sky->mean[x1] = ( cnt>0 ? sum/cnt : 0);
         sky->cnt[x1] = cnt;

         sky->min = MIN( sky->min, sky->mean[x1]);
         sky->max = MAX( sky->max, sky->mean[x1]);
      }
   }
   /*
   **  Get all the mean values for the object bins.
   */
   globol_min = DF_MAX_SIGNED_INT32;
   globol_max = DF_MIN_SIGNED_INT32;
   for( igraph=0; igraph < numgraph; igraph++)
   {
      object[igraph].min = DF_MAX_SIGNED_INT32;
      object[igraph].max = DF_MIN_SIGNED_INT32;
      object[igraph].begrow = dp->sa_objbin_min + (dp->sa_rows_per_bin * igraph);
      object[igraph].endrow = MIN(MIN(object[igraph].begrow+dp->sa_rows_per_bin-1, dp->sa_objbin_max),
                                  bp->naxis2-1);

      for( x1 = MAX(0, dp->sa_xmin);
           x1 <= MIN(dp->sa_xmax, bp->naxis1-1); x1++)
      {
         sum = 0;
         cnt = 0;
         for( y1 = object[igraph].begrow; y1 <= object[igraph].endrow; y1++)
         {
            inx = y1 * bp->naxis1 + x1;
				sum += dfdatainx( bp, inx );
            cnt++;
         }

         /* Find object mean value */
         object[igraph].mean[x1] = ( cnt > 0 ? sum/cnt : 0);
         object[igraph].cnt[x1]  = cnt;

         /* Subtract the sky value */
         if( dp->sa_subtractsky )
            object[igraph].mean[x1] -= sky->mean[x1];

         object[igraph].min = MIN( object[igraph].min, object[igraph].mean[x1]);
         object[igraph].max = MAX( object[igraph].max, object[igraph].mean[x1]);
      }

      globol_min = MIN( globol_min, object[igraph].min);
      globol_max = MAX( globol_max, object[igraph].max);
   }
   /*
   **  Display Stats for each set of data.
   */
   if( dp->sa_stats )
   {
      fontscale = 12;
      fprintf( fp, "/Courier findfont %d scalefont setfont\n", fontscale);
      x1 = 3 * 72;
      y1 = 5 * 72;
      fprintf( fp, "%d %d ( Rows     N     Mean   STD  Ratio) rtextproc\n", x1, y1);

      for( igraph=0; igraph < numgraph; igraph++)
      {
         /* Calculate stats */
         n = sum = sdd = 0;
         for( x1 = MAX(0, dp->sa_xmin);
              x1 <= MIN(dp->sa_xmax, bp->naxis1-1); x1++)
         {
            n++;
            sum += object[igraph].mean[x1];
            sdd += object[igraph].mean[x1] * object[igraph].mean[x1];
         }
         mean = ( n >=1 ? sum/n : 0);
         if( n >=2 )
         {
            sdd -= n * (mean*mean);
            std = sqrt(sdd / (n-1));
         }
         else
            sdd = std = 0;
         /* Display data    */

         sprintf(buf, "%3d-%3d  %3.0f  %5.0f %5.2f %5.2f",
                 object[igraph].begrow, object[igraph].endrow, n, mean, std, mean/std);
         y1 += fontscale;
         fprintf( fp, "%d %d (%s) rtextproc\n", x1, y1, buf);

      }
   }
   /*
   **  Display Graph.
   */
   if( !dp->sa_stats )
   {
      /*
      **  Display X axis labels
      */
      fontscale = 10;
      fprintf( fp, "/Helvetica findfont %d scalefont setfont\n", fontscale);

      fprintf( fp, "%% Label X Axis\n");
      fprintf( fp, "%d %d %d %d lineproc stroke\n", xmin, ymin,  xmax, ymin);
      fprintf( fp, "%d %d %d %d lineproc stroke\n", xmin, ymax,  xmax, ymax);

      y1 = ymin - fontscale;
      y2 = y1 - fontscale;
      
      for( i=0; i <= 100; i+=25 )
      {
         l = map( i, 0.0, 100.0, dp->sa_xmin, dp->sa_xmax);
         x1 = map( i, 0, 100, xmin, xmax);

         fprintf( fp, "%d %d %d %d lineproc stroke\n", x1, ymax, x1, ymax+fontscale);
         fprintf( fp, "%d %d %d %d lineproc stroke\n", x1, ymin, x1, y1);
         fprintf( fp, "%d %d (%d) ctextproc\n", x1, y2, l);
      }
      /*
      ** Graph each set of data
      */
      for( igraph=0; igraph < numgraph; igraph++)
      {
         fprintf( fp, "%% Graph Data Set %d\n", igraph);
         fprintf( fp, ".%d setgray\n", (igraph % 2 ? 2 : 4) );
         /*
         **  Determine demension of graph area. Only Y's get adjusted.
         **  And draw Y axis.
         */
         ymin = 72;
         ymax = 6 * 72;
         l = ( (float)(ymin-ymax+1)/numgraph);
         ymax += l * igraph;
         ymin = ymax + l;

         /*  Draw the Y Axis & Labels */
         if( dp->sa_yautoscale == 2 )       /* Globol autoscale */
         {
            yscale_min = globol_min;
            yscale_max = globol_max;

         }
         else if( dp->sa_yautoscale == 1 )  /* Local autoscale */
         {
            yscale_min = object[igraph].min;
            yscale_max = object[igraph].max;
         }
         else                                      /* Fixed scale */
         {
            yscale_min = dp->sa_ymin;
            yscale_max = dp->sa_ymax;
         }
         if( (yscale_max-yscale_min) < 3)
         {
            yscale_min--;
            yscale_max++;
         }

         x1 = xmax + fontscale;

         fprintf( fp, "%d %d %d %d lineproc stroke\n",  xmax, ymin, x1, ymin);
         fprintf( fp, "%d %d %d %d lineproc stroke\n",  xmax, ymin, xmax, ymax);
         fprintf( fp, "%d %d %d %d lineproc stroke\n",  xmax, ymax, x1, ymax);
         fprintf( fp,  "%d %d (%2.0f) ltextproc\n", x1, ymin, yscale_min);
         fprintf( fp,  "%d %d (%2.0f) ltextproc\n", x1, ymax-fontscale, yscale_max);
         /*
         **  Graph the data
         **  Remember: XScale refers to naxis2, YScale refers to data values.
         */
         i = MAX( 0, dp->sa_xmin);             /* Get x,y of first data point */
         x1 = map( i, dp->sa_xmin, dp->sa_xmax+1, xmin, xmax);
         y1 = map( object[igraph].mean[i], yscale_min, yscale_max, ymin, ymax);
         for( ; i <= MIN( bp->naxis1-1, dp->sa_xmax); i++)
         {
            x2 = map( i+1, dp->sa_xmin, dp->sa_xmax+1, xmin, xmax);
            y2 = map( object[igraph].mean[i], yscale_min, yscale_max, ymin, ymax);

            fprintf( fp, "%d %d %d %d lineproc stroke\n", x1, y1, x1, y2);
            fprintf( fp, "%d %d %d %d lineproc stroke\n", x1, y2, x2, y2);

            x1 = x2;
            y1 = y2;
         }
         fprintf( fp, "1 setgray\n");
      }
   }
   /*
   **  clean up
   */
   free( (char*) object );
   free( (char*) sky );
   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  print_sb() - spectra graph type B print procedure.
**--------------------------------------------------------------------------------
*/
int print_sb( 
   FILE *fp,                   /* out to this file pointer */
   struct dpy_t    *dp         /* display pointer */
)
{
  int     x1, y1, x2, y2,
          xmin, xmax,      /* Graph area min, max, & size in pixels */
          ymin, ymax,
          imax, imin,
          object,
          fontscale,
          inx,
          l, i, cnt;

   float  *sky_mean,
          *obj_mean,
          num_sky_rows,
          num_obj_rows,
          diff_ymin, diff_ymax,
          data_ymin, data_ymax,
          fdata, fsum;

	struct df_buf_t *bp;        /* buffer pointer */

	bp = &Buffer[dp->bufinx];

   /*  Allocate Temporary buffers */
   if( NULL == ( sky_mean = (float*) malloc( sizeof(float)*NUM_PIXEL) ) )
   {
      return ERR_MEM_ALLOC;
   }
   if( NULL == ( obj_mean = (float*) malloc( sizeof(float)*NUM_PIXEL) ) )
   {
      free( (char*) sky_mean );
      return ERR_MEM_ALLOC;
   }

   /*
   **  Display header & X axis labels
   */
   fontscale = 10;
   fprintf( fp, "/Helvetica findfont %d scalefont setfont\n", fontscale);
   fprintf( fp, "%d %d (SPECTRA B) rtextproc\n", (int)(7.3*72), (int)(6.1*72));

   xmin =  72;
   xmax =  7.4*72;
   ymin =  72;
   ymax =  6.0*72;

   fprintf( fp, "%% Label X Axis\n");
   fprintf( fp, "%d %d %d %d lineproc stroke\n", xmin, ymax, xmax, ymax);
   fprintf( fp, "%d %d %d %d lineproc stroke\n", xmin, ymin, xmax, ymin);
   y1 = ymin - fontscale;
   y2 = y1 - fontscale;
   for( i=0; i <= 100; i+=25 )
   {
      l = map(  i, 0.0, 100.0,
               dp->sb_xmin, dp->sb_xmax+1);

      x1 = map( i, 0.0, 100.0, xmin, xmax);
      fprintf( fp, "%d %d %d %d lineproc stroke\n", x1, ymax, x1, ymax+fontscale);
      fprintf( fp, "%d %d %d %d lineproc stroke\n", x1, ymin, x1, y1);
      fprintf( fp, "%d %d (%d) ctextproc\n", x1, y2, l);
   }
   /*
   **  Find the mean for obj and sky bins.
   **  1st, sum the pixel by columns.
   */
   memset( (char*) sky_mean, 0x00, sizeof(float)*NUM_PIXEL);
   memset( (char*) obj_mean, 0x00, sizeof(float)*NUM_PIXEL);

   for( y1 = dp->sb_objbin_min;
        y1 <= dp->sb_objbin_max && y1 < bp->naxis2;
        y1++)
      for( x1 = dp->sb_xmin;
           x1 <= dp->sb_xmax && x1 < bp->naxis1;
           x1++)
      {
         inx = y1 * bp->naxis1 + x1;
			obj_mean[x1] += dfdatainx( bp, inx);
      }

   for( y1 = dp->sb_skybin_min;
        y1 <= dp->sb_skybin_max && y1 < bp->naxis2;
        y1++)
      for( x1 = dp->sb_xmin;
           x1 <= dp->sb_xmax && x1 < bp->naxis1;
           x1++)
      {
         inx = y1 * bp->naxis1 + x1;
			sky_mean[x1] += dfdatainx( bp, inx);
      }

   /*
   **  Divide totals by number of row & find max, min. Also, divide by coadds
   */
   num_sky_rows = dp->sb_skybin_max - dp->sb_skybin_min +1;
   num_obj_rows = dp->sb_objbin_max - dp->sb_objbin_min +1;
   if( dp->sb_yautoscale )
   {
      data_ymin = diff_ymin = DF_MAX_SIGNED_INT32;
      data_ymax = diff_ymax = DF_MIN_SIGNED_INT32;
   }
   else
   {
      data_ymin = dp->sb_data_ymin;
      data_ymax = dp->sb_data_ymax;
      diff_ymin = dp->sb_diff_ymin;
      diff_ymax = dp->sb_diff_ymax;
   }

   for( x1 = dp->sb_xmin; x1 <= dp->sb_xmax; x1++)
   {
      /*  Divide by number of rows */
      sky_mean[x1] /= num_sky_rows;
      obj_mean[x1] /= num_obj_rows;

      fdata = obj_mean[x1] - sky_mean[x1];

      if( dp->sb_yautoscale )
      {
         data_ymin = MIN( data_ymin, MIN(sky_mean[x1], obj_mean[x1]));
         data_ymax = MAX( data_ymax, MAX(sky_mean[x1], obj_mean[x1]));

         diff_ymin = MIN( diff_ymin, fdata);
         diff_ymax = MAX( diff_ymax, fdata);
      }
   }
   /*
   **  Draw the Reduced Graph.
   */
   if( dp->sb_showgraph & 0x01 )
   {
      /*  Dimension area for graph */
      ymin = 72;
      ymax = 6 * 72;
      if( dp->sb_showgraph & 0x02 )
         ymin = 3.6 * 72;

      /*  Draw Y Axis Scale  */
      x1 = 7.4 * 72;
      x2 = 7.5 * 72;
      fprintf( fp, "%d %d %d %d lineproc stroke\n", x1, ymin, x1, ymax);
      for( i=0; i <= 100; i+=25 )
      {
         l = map( i, 0.0, 100.0, diff_ymin, diff_ymax);
         y1 = map( i, 0.0, 100.0, ymin, ymax);

         fprintf( fp, "%d %d %d %d lineproc stroke\n", x1, y1, x2, y1);
         fprintf( fp, "%d %d (%d) ltextproc\n", x2, y1, l);
      }

      /*  Graph the Data  */
      fsum = 0;
      cnt = 0;
      /* Graph the points from imin to imax */
      imin = MAX( 0, dp->sb_xmin);
      imax = MIN( bp->naxis1-1, dp->sb_xmax);

      i = imin;                   /* get x, y of first data point */
      fdata = obj_mean[i] - sky_mean[i];
      x1 = map( i, dp->sb_xmin, dp->sb_xmax+1, xmin, xmax);
      y1 = map( fdata, diff_ymin, diff_ymax, ymin, ymax);
      for( i=imin; i<=imax; i++)
      {
         fdata = obj_mean[i] - sky_mean[i];
         fsum += fdata; cnt++;

         x2 = map( i+1, dp->sb_xmin, dp->sb_xmax+1, xmin, xmax);
         y2 = map( fdata, diff_ymin, diff_ymax, ymin, ymax);

         fprintf( fp, "%d %d %d %d lineproc stroke\n", x1, y1, x1, y2);
         fprintf( fp, "%d %d %d %d lineproc stroke\n", x1, y2, x2, y2);

         x1 = x2;
         y1 = y2;
      }
      /* display label */
      if( cnt )
      {
         y1 = map( fsum/cnt, diff_ymin, diff_ymax, ymin, ymax);
         fprintf( fp, "%d %d (Obj-Sky) ltextproc\n", (int)(0.5*72), y1);
      }

   }

   /*
   **  Draw the Obj & Sky Graph.
   */
   if( dp->sb_showgraph & 0x06 )
   {
      /*  Dimension area for graph */
      ymin = 72;
      ymax = 6.0 * 72;
      if( dp->sb_showgraph & 0x01 )
         ymax = 3.4 * 72;

      /*  Draw Y Axis Scale  */
      x1 = 7.4 * 72;
      x2 = 7.5 * 72;
      fprintf( fp, "%d %d %d %d lineproc stroke\n", x1, ymin, x1, ymax);
      for( i=0; i <= 100; i+=25 )
      {
         l = map(  i, 0.0, 100.0, data_ymin, data_ymax);
         y1 = map( i, 0.0, 100.0, ymin, ymax);
         fprintf( fp, "%d %d %d %d lineproc stroke\n", x1, y1, x2, y1);
         fprintf( fp, "%d %d (%d) ltextproc\n", x2, y1, l);
       }

      /*  Graph the Data  */
      /* Graph the points from imin to imax */
      imin = MAX( 0, dp->sb_xmin);
      imax = MIN( bp->naxis1-1, dp->sb_xmax);
      /* 2 passes to graph the obj & sky data */
      for( object=0; object <= 1; object++)
      {
         if( ((dp->sb_showgraph & 0x02) && object ) ||
             ((dp->sb_showgraph & 0x04) && !object) )
         {
            fsum = 0; cnt = 0;

            i = imin;          /* get x, y of first data point */
            fdata = (object ? obj_mean[i] : sky_mean[i]);
            x1 = map( i, dp->sb_xmin, dp->sb_xmax+1, xmin, xmax);
            y1 = map( fdata, data_ymin, data_ymax, ymin, ymax); 

            for( i = imin; i<=imax; i++)
            {
               fdata = (object ? obj_mean[i] : sky_mean[i]);
               fsum += fdata; cnt++;
               x2 = map( i+1, dp->sb_xmin, dp->sb_xmax+1, xmin, xmax);
               y2 = map( fdata, data_ymin, data_ymax, ymin, ymax);

               fprintf( fp, "%d %d %d %d lineproc stroke\n", x1, y1, x1, y2);
               fprintf( fp, "%d %d %d %d lineproc stroke\n", x1, y2, x2, y2);

               x1 = x2;
               y1 = y2;
            }
            /* Draw labels */
            if( cnt )
            {
               y1 = map( fsum/cnt, data_ymin, data_ymax, ymin, ymax);
               fprintf( fp, "%d %d (%s bin) ltextproc\n", (int)(0.5*72), y1,
                        (object? "Obj" : "Sky"));
            }
         }
      }
   }
   /*
   **  clean up
   */
   free( (char*) obj_mean );
   free( (char*) sky_mean );
   return ERR_NONE;
}

