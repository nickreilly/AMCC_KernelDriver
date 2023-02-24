/********************************************************************************
**           cm.c - color map routines for dynamic coloring of pixels          **
**
** Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php
** Copyright (C) 2012 <NASA IRTF. http://irtfweb.ifa.hawaii.edu>
********************************************************************************/

#define EXTERN extern

/*----------------------------
**  Includes
**----------------------------
*/
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#include <gtk/gtk.h>

#include "dv.h"           /* DV MAIN APPLICATION  */

/************************************************************************
** function for color setup and managing of the colormap.
*************************************************************************
*/

static const gchar* visual_names[] = { "static gray", "grayscale", "static color",
        "pseudo color", "true color", "direct color", };
static const gchar* byte_order[] = { "LSB", "MSB" };
static GdkVisual * cm_get_visual( int pc_ok, int tc_ok, int verbose );

/*------------------------------------------------------------------
**  cm_setup( ) - initializes and setup colors for this application
**------------------------------------------------------------------
*/
int cm_setup(
   int pc_ok,        /* if FALSE, won't try pseudo color */
   int tc_ok,        /* if FALSE, won't try true color */
   int v             /* verbose */
)
{
   int rc,
       i;

   /*-------------------------------------------------------
   ** query for supported visual. This application supports:
   **   PseudoColor 8 bit;
   */
   if( (CM.visual = cm_get_visual( pc_ok, tc_ok, v )) == NULL )
      return -1;
   if( v ) printf("Visual: type:%s depth:%d byte_order=%d(%s) cmSize=%d bitRGB=%d \n",
      visual_names[CM.visual->type], CM.visual->depth, CM.visual->byte_order, 
		byte_order[CM.visual->byte_order],
      CM.visual->colormap_size, CM.visual->bits_per_rgb);

   // swap long values for gtk_image_() stuff if X11 and CPU byteorder is different 
   CM.swap_rgb = (CM.visual->byte_order == cpu_msb() ? 0 : 1 );

   /*-------------------------------------------------------
   ** Call function to allocate/setup colormap.
   */
   CM.center = 0.5;
   CM.width  = 0.5;
   CM.colormap = gdk_colormap_get_system();

   if( CM.visual->type == GDK_VISUAL_PSEUDO_COLOR)
      rc = cm_setup_pseudoColor( v );
   else if( CM.visual->type == GDK_VISUAL_TRUE_COLOR)
      rc = cm_setup_trueColor( v );
   else
     rc = -1;

    if( rc )       /* return on error */
       return rc;

   /*------------------------------------
   ** initialize colormap to gray scale.
   */
   for( i=0; i<=CM_BLUE_INX; i++)
   {
      CM.cgraph[i][CM_X][0] = 0;
      CM.cgraph[i][CM_Y][0] = 0;
      CM.cgraph[i][CM_X][1] = 1;
      CM.cgraph[i][CM_Y][1] = 1;
      CM.cgraph_num_ele[i]  = 2;
   }

   cm_set_colormap( &CM );
   return 0;
}

/*-------------------------------------------------------------------------
**  cm_setup_pseudoColor() - initialize color cells pseudo color displays
**     This is a helper function for cm_setup().
**-------------------------------------------------------------------------
*/
int cm_setup_pseudoColor( int v /*verbose */ )
{
   int i, rc;
   gboolean success[CM_NUM_COLORS];

   CM.max_intensity = 65535; /* color intensity ranges from 0 to max_intensity */

   rc = gdk_colormap_alloc_colors( CM.colormap, CM.colors, CM_NUM_COLORS, TRUE, FALSE, success );
   if( v ) printf("1st try: gdk_colormap_alloc_color()=%d\n", rc);
   if( rc != CM_NUM_COLORS )
   {
      if( v ) printf("allocating new colormap\n");
      /* allocating new colormap */
      if( !(CM.colormap = gdk_colormap_new( CM.visual, FALSE)) )
      {
         if( v ) printf("gdk_colormap_new() - failed. exiting..\n");
         return -1;
      }
      gtk_widget_set_default_colormap( CM.colormap );

      /* try again with you own private colormap */
      rc = gdk_colormap_alloc_colors( CM.colormap, CM.colors, CM_NUM_COLORS, TRUE, FALSE, success );
      if( v ) printf("2nd try: gdk_colormap_alloc_color()=%d\n", rc);
      if( rc != CM_NUM_COLORS )
      {
         if( v ) printf("gdk_colormap_alloc_colors() - failed. exiting..\n");
         return -1;
      }
   }

   for(i=0;i<CM_NUM_STATIC_COLORS; i++)
   {
      CM.colors[i].red    = CM.max_intensity * CM_static_colors_def[i].red;
      CM.colors[i].green  = CM.max_intensity * CM_static_colors_def[i].green;
      CM.colors[i].blue   = CM.max_intensity * CM_static_colors_def[i].blue;
      gdk_color_change( CM.colormap, &CM.colors[i]);
   }
   return 0;
}

/*-------------------------------------------------------------------------
**  cm_setup_trueColor() - initialize color cells for true color displays
**     This is a helper function for cm_setup().
**-------------------------------------------------------------------------
*/
int cm_setup_trueColor( int v /* verbose */ )
{
   int i;

   if( v ) printf("executing cm_setup_trueColor( )...\n");

   /*----------------------------
   ** initialize fixed colors.
   ** and initalize ALL pixel values in colors[].
   */
   CM.max_intensity = 65535; /* color intensity ranges from 0 to max_intensity */
   for(i=0;i<CM_NUM_STATIC_COLORS; i++)
   {
      CM.colors[i].red    = CM.max_intensity * CM_static_colors_def[i].red;
      CM.colors[i].green  = CM.max_intensity * CM_static_colors_def[i].green;
      CM.colors[i].blue   = CM.max_intensity * CM_static_colors_def[i].blue;
      CM.colors[i].pixel  = cm_pixel_from_rgb( CM.colors[i].red, CM.colors[i].green, 
                                               CM.colors[i].blue, CM.visual );
   }

   return 0;
}

/*---------------------------------------------------------------------------
**  cm_pixel_from_rgb( ) - determines a pixel value from components.
**---------------------------------------------------------------------------
*/
uint32_t cm_pixel_from_rgb( int red, int green, int blue, GdkVisual * visual  )
{
  uint32_t pixel;
  pixel =  ( (red   >> (16 - visual->red_prec  ) ) << visual->red_shift   ) |
           ( (green >> (16 - visual->green_prec) ) << visual->green_shift ) |
           ( (blue  >> (16 - visual->blue_prec ) ) << visual->blue_shift  ) ;
   return pixel;
}

/*---------------------------------------------------------------------------
**  cm_get_visual( ) - Query the system to obtain a support visual
**---------------------------------------------------------------------------
*/
GdkVisual * cm_get_visual(
   int pc_ok,   /* if FALSE, won't try pseudo color */
   int tc_ok,   /* if FALSE, won't try true color */
   int verbose  /* verbose */
)
{
    GdkVisual *v;
    GdkVisualType *vt;
    int *depths;
    int count, i;

   if( verbose )
   {
      /* show supported visuals */
      gdk_query_visual_types( &vt, &count);
      if( verbose ) 
      {
			printf("Number of visual types %d :", count);
			for( i=0; i < count; i++ )
				printf("(%s)", visual_names[vt[i]]);
			printf("\n");
      }

      /* show supported depth */
      gdk_query_depths( &depths, &count);
      if( verbose ) 
      {
			printf("Number of depths: %d :", count);
			for( i=0; i < count; i++ )
				printf("(%d)", depths[i]);
			printf("\n");
      }

      /* display system visual  */
      v = gdk_visual_get_system();
      if( verbose )
         printf("gdk_visual_get_system(): type:%s depth:%d cmSize=%d bitRGB=%d \n",
			visual_names[v->type], v->depth, v->colormap_size, v->bits_per_rgb);

      /* gdk_visual_get_best()  */
      v = gdk_visual_get_best();
      if( verbose )
         printf("gdk_visual_get_best(): type:%s depth:%d cmSize=%d bitRGB=%d \n",
			visual_names[v->type], v->depth, v->colormap_size, v->bits_per_rgb);

      v = gdk_visual_get_best_with_both( 8, GDK_VISUAL_PSEUDO_COLOR );
      if( verbose )
      {
			if( v == NULL )
				printf("Unsupported: 8-bit PseudoColor\n");
			else
				printf("Supported: type:%s depth:%d cmSize=%d bitRGB=%d \n",
				 visual_names[v->type], v->depth, v->colormap_size, v->bits_per_rgb);
      }

      v = gdk_visual_get_best_with_type( GDK_VISUAL_TRUE_COLOR );
      if( verbose )
      {
			if( v == NULL )
				printf("Unsupported: TrueColor\n");
			else
			{
				printf("Supported: type:%s depth:%d cmSize=%d bitRGB=%d \n",
					 visual_names[v->type], v->depth, v->colormap_size, v->bits_per_rgb);
				printf("C: [mask shift prec] sample \n");
				printf("R: [ 0x%08x %d %d ] 0x%08x \n",
					v->red_mask, v->red_shift, v->red_prec, cm_pixel_from_rgb( 0xffff, 0, 0, v ));
				printf("G: [ 0x%08x %d %d ] 0x%08x \n",
					v->green_mask, v->green_shift, v->green_prec, cm_pixel_from_rgb( 0, 0xffff, 0, v ));
				printf("B: [ 0x%08x %d %d ] 0x%08x \n",
					v->blue_mask, v->blue_shift, v->blue_prec, cm_pixel_from_rgb( 0, 0, 0xffff, v ));
			}
		}
   }

   /* 1st try to get 8-bit pseudo_color */
   if( pc_ok )
      if( (v = gdk_visual_get_best_with_both( 8, GDK_VISUAL_PSEUDO_COLOR ))  )
         return v;

   /* next try true color */
   if( tc_ok )
	{
      if( (v = gdk_visual_get_best_with_both( 24, GDK_VISUAL_TRUE_COLOR ))  )
         return v;

      if( (v = gdk_visual_get_best_with_type( GDK_VISUAL_TRUE_COLOR ))  )
         return v;
	}

   return NULL;
}

/*-----------------------------------------------------------------------
**  cm_set_colormap() - install colorsmap colors
**-----------------------------------------------------------------------
*/
int cm_set_colormap( struct cm_t *cm )
{
   int beg, end, size, mid_point,
       rs, re,
       color,
       red, grn, blu,
       i, imax;

   beg = CM_NUM_STATIC_COLORS;
   end = CM_NUM_COLORS-1;
   size = CM_NUM_RW_COLORS;
   mid_point  = beg + ( cm->center * size );
   rs = mid_point - ( cm->width * size);  /* RealStart - after applying center&width */
   re = mid_point + ( cm->width * size);  /* RealEnd - after applying center&width */
#if 0
   printf("Center=%f  Width=%f\n", cm->center, cm->width);
   printf("beg=%d end=%d size=%d\n", beg, end, size);
   printf("rs=%d re=%d\n", rs, re);
#endif


   /* fill in all points before rs */
   imax = MIN(rs-1, end);
#if 0
   printf("BEG: %d..%d\n", beg, imax);
#endif
   red = cm->cgraph[CM_RED_INX][CM_Y][0] * cm->max_intensity;
   grn = cm->cgraph[CM_GREEN_INX][CM_Y][0] * cm->max_intensity;
   blu = cm->cgraph[CM_BLUE_INX][CM_Y][0] * cm->max_intensity;
   for( i = beg; i <= imax; i++ )
   {
      CM.colors[i].red   = red;
      CM.colors[i].green = grn;
      CM.colors[i].blue  = blu;
   }

   /*
   ** Fill in all points between rs & re based on cgraph data points.
   */
#if 0
   printf("FILL: %d..%d\n", rs, re);
#endif
   for( color=0; color < 3; color++ )
   {
      for( i=0; i < cm->cgraph_num_ele[color]-1; i++)
      {
         cm_fill( rs, re,
                  cm->cgraph[color][CM_X][i], cm->cgraph[color][CM_X][i+1],
                  cm->cgraph[color][CM_Y][i], cm->cgraph[color][CM_Y][i+1],
                  cm, color, beg, end);
      }
   }


   /* fill in all points after re */
#if 0
   printf("END: %d..%d\n", re+1, end);
#endif

   red = cm->cgraph[CM_RED_INX][CM_Y][cm->cgraph_num_ele[CM_RED_INX]-1] * cm->max_intensity;
   grn = cm->cgraph[CM_GREEN_INX][CM_Y][cm->cgraph_num_ele[CM_GREEN_INX]-1] * cm->max_intensity;
   blu = cm->cgraph[CM_BLUE_INX][CM_Y][cm->cgraph_num_ele[CM_BLUE_INX]-1] * cm->max_intensity;
   for( i = re+1; i <= end; i++ )
   {
      CM.colors[i].red   = red;
      CM.colors[i].green = grn;
      CM.colors[i].blue  = blu;
   }

   if( CM.visual->type == GDK_VISUAL_PSEUDO_COLOR )
   {
      /* modidy the system colormap */
      for(i=CM_NUM_STATIC_COLORS; i<CM_NUM_COLORS; i++)
         gdk_color_change( CM.colormap, &CM.colors[i]);
   }
   else if( CM.visual->type == GDK_VISUAL_TRUE_COLOR )
   {
      /* Store RGB mask in pixel variable */
      for(i=CM_NUM_STATIC_COLORS; i<CM_NUM_COLORS; i++)
		{
			CM.colors[i].pixel  = cm_pixel_from_rgb( CM.colors[i].red, CM.colors[i].green,
                                                  CM.colors[i].blue, CM.visual );
		}
   }

   return 0;
}

/*--------------------------------------------------------------------------------
**  cm_fill() - fills an array based on parameters.
**     for PseudoColor the xcolor structure are filled with its color definitions.
**     for TrueColor the cminfo->colors will be filled with the RGB truecolor value.
**--------------------------------------------------------------------------------
*/
void cm_fill(
   int rs,        /* Identifies the current segment this function will deal with */
   int re,
   double ps,     /* select a  segment of [rs..re] to be filled. ps/pe range is [0...1]. */
   double pe,
   double vs,     /* Will fill [ps..pe] with values [vs..ve]*intensity. vs/ve ranges is [0..1] */
   double ve,
   struct cm_t *cm,
   int color,        /* Color to fill. R=0, G=1, B=2     */
   int beg,          /* beg & end of array. Actually fills are clip within this range */
   int end
)
{
   int   is, ie,     /* starting and ending index positions */
         i;
#if 0
   printf("cmfill( %d, %d, %f, %f, %f, %f, %d, %d, %d)\n", rs,re,ps,pe,vs,ve,color,beg,end);
#endif
   /* is/ie = segment in [rs..re] to work with. Find in by applying [ps..pe]. */
   i = (re-rs);  /* size of segment */
   is = (i * ps) + rs;
   ie = (i * pe) + rs;

   /* clip [is..ie] with [beg..end] */
   is = ( is < beg ? beg : is);
   ie = ( ie > end ? end : ie);

   for( i=is; i<=ie; i++)            /* fill in values */
   {
      int c;
      c = map( i, is, ie, vs, ve ) * cm->max_intensity;

      if( color == 0 )     CM.colors[i].red   = c;
      else if( color == 1) CM.colors[i].green = c;
      else if( color == 2) CM.colors[i].blue  = c;
   }
}

/*--------------------------------------------------------------------------------
** cm_free() - before exiting you application, call this function to deallocate
** and colors or colormap allocated by cm_setup();
**--------------------------------------------------------------------------------
*/
int cm_free( void )
{
   if( CM.visual->type == GDK_VISUAL_PSEUDO_COLOR)
   {
      gdk_colormap_free_colors( CM.colormap, CM.colors, CM_NUM_COLORS);
   }

   return ERR_NONE;
}


/*--------------------------------------------------------------------------------
** cm_swap_int32() - If swap_me is true, then sl will be reordered. 
**--------------------------------------------------------------------------------
*/
uint32_t cm_swap_int32( uint32_t sl, int swap_me )
{
	uint32_t dl;
	char * s;
	char * d;
   if( swap_me )
   {
		s = (char*) &sl;
		d = (char*) &dl;
		d[0] = s[3];
		d[1] = s[2];
		d[2] = s[1];
		d[3] = s[0];
		return dl;
	}
   else
      return sl;       
}

