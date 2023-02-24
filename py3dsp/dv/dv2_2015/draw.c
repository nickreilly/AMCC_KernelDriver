/**************************************************************************
**  draw.c - holds the drawing area redraw routines
**
** Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php
** Copyright (C) 2012 <NASA IRTF. http://irtfweb.ifa.hawaii.edu>
***************************************************************************
*/

#define EXTERN extern
#define DEBUG 0

/*-----------------------------
**  Standard include files
**-----------------------------
*/
#include <sys/time.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <semaphore.h>
#include <mqueue.h>


#include <gdk/gdkx.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#if USE_GSLFIT
#include "fitgsl.h"
#endif // USE_GSLFIT

/*-----------------------------
**  Non-standard include files
**-----------------------------
*/

#include "dv.h"


static int Box_x,     /* Used in dpydata_*_event() function to keep track of */
           Box_y;     /* begining of the XOR boxes.                          */

/********************************************************************************/
/*  Colormap_w functions - drawing_area repaint, event, etc.                    */
/********************************************************************************/

/*----------------------------------------------------------------------------
**  colormap_configure_event() - configure event handler for Colormap_w
**----------------------------------------------------------------------------
*/
int colormap_configure_event( GtkWidget *widget, GdkEventConfigure *event )
{
   return TRUE;
}

/*----------------------------------------------------------------------------
**  colormap_expose_event() - expose event handler for Colormap_w
**----------------------------------------------------------------------------
*/
int colormap_expose_event( GtkWidget *widget, GdkEventExpose *event )
{
   colormap_redraw( widget );
   return FALSE;
}

/*----------------------------------------------------------------------------
**  call_colormap_redraw() - dv code calls this to refresh the colormap area.
**----------------------------------------------------------------------------
*/
void call_colormap_redraw( void )
{  
   colormap_redraw( W.colormap );
}

/*----------------------------------------------------------------------------
**  colormap_redraw() - display a colormap in the window.
**----------------------------------------------------------------------------
*/
void colormap_redraw( GtkWidget *da )
{  
   int i,
       color,
       da_wid, da_hgt;
   GdkImage *image;

   /* Check that the drawing area has been initialized. */
   if ( da->window == NULL )
      return;

   gdk_drawable_get_size( da->window, &da_wid, &da_hgt);

   image = gdk_image_new( GDK_IMAGE_NORMAL, CM.visual, da_wid, da_hgt );
   if( image == NULL )
      return;
#if DEBUG
   printf("colormap_redraw(): image: %dx%dx%d\n", image->width, image->height, image->depth );
#endif 
   if( CM.visual->type == GDK_VISUAL_PSEUDO_COLOR)
   {
      unsigned char *cbuf = (unsigned char *) image->mem;

      /* fill in 1st scan line */
      for( i=0; i< da_wid; i++)
      {
         color = map( i, 0, da_wid-1, CM_NUM_STATIC_COLORS, CM_NUM_COLORS-1);
         cbuf[i] =  CM.colors[color].pixel;
      }
      /* copy 1st line to fill hgt of image buffer */
      for( i=0; i< da_hgt; i++)
         memcpy( cbuf+(i*da_wid), cbuf, da_wid);
   }
   else if( CM.visual->type == GDK_VISUAL_TRUE_COLOR)
   {
      uint32_t *lbuf = (uint32_t *) image->mem;
      /* fill in 1st scan line */
      for( i=0; i< da_wid; i++)
      {
         color = map( i, 0, da_wid-1, CM_NUM_STATIC_COLORS, CM_NUM_COLORS-1);
         lbuf[i] =  cm_swap_int32( CM.colors[color].pixel, CM.swap_rgb );
      }
      /* copy 1st line to fill hgt of image buffer */
      for( i=1; i< da_hgt; i++)
         memcpy( lbuf+(i*da_wid), lbuf, da_wid*sizeof(uint32_t));
   }

   gdk_draw_image( da->window, W.nor_gc, image, 0, 0, 0, 0, da_wid, da_hgt);
   //gdk_image_destroy( image );
   g_object_unref( image );
}


/********************************************************************************/
/*  dpytitle drawingArea's signal functions - drawing_area repaint, event, etc. */
/********************************************************************************/
/*----------------------------------------------------------------------------
**  dpytitle_configure_event() - configure event handler for dpytitle Drawing Area
**----------------------------------------------------------------------------
*/
int dpytitle_configure_event( GtkWidget *widget, GdkEventConfigure *event )
{
   return TRUE;
}
/*----------------------------------------------------------------------------
**  dpytitle_expose_event() - expose event handler for dpytitle Drawing Area
**----------------------------------------------------------------------------
*/
int dpytitle_expose_event( GtkWidget *widget, GdkEventExpose *event )
{
   dpytitle_redraw( widget );
   return FALSE;
}

/*----------------------------------------------------------------------------
**  call_dpytitle_redraw() - dv code calls this to refresh a dpytitle area.
**----------------------------------------------------------------------------
*/
void call_dpytitle_redraw( int dpinx )
{
   dpytitle_redraw( Dpy[dpinx].title_drawingarea );
}

/*-------------------------------------------------------------------------------
**  redraw_dpytitle_for_bufinx() - Calls the redraw function for the dpytitle_w
**      for all dpy using bufinx.
**-------------------------------------------------------------------------------
*/
void redraw_dpytitle_for_bufinx( int bufinx )
{
   int dpinx;

   for( dpinx = 0; dpinx < Lc.num_dpy; dpinx++ )
      if( Dpy[dpinx].bufinx == bufinx )
         call_dpytitle_redraw( dpinx );
}

/*----------------------------------------------------------------------------
**  dpytitle_redraw() - DPY.Title area's main redraw function.
**----------------------------------------------------------------------------
*/
void dpytitle_redraw( GtkWidget *da )
{
   int dpinx,
       da_wid, da_hgt;
   char buf[80];
   struct dpy_t *dp;
   struct df_buf_t *bp;

   /* Check that the drawing area has been initialized. */
   if ( da->window == NULL )
      return;

   /* get display / data buffer info */
   dpinx = (intptr_t) g_object_get_data( G_OBJECT(da), DPYINX_DATA_KEY);
   dp = &Dpy[dpinx];
   bp = &Buffer[dp->bufinx];
   gdk_drawable_get_size( da->window, &da_wid, &da_hgt);

   /* clears window  or set to yellow for active display */
   if( dpinx == Lc.active )
   {
      gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_YELLOW] );
      gdk_draw_rectangle( da->window, W.nor_gc, TRUE, 0, 0, da_wid, da_hgt);
   }
   else
      gdk_window_clear( da->window );

   /* display information */
   if( bp->status == DF_EMPTY )
      sprintf( buf, "%d. %s = Empty buffer (%dx%d)", dpinx, buffer_selection[dp->bufinx],
         da_wid, da_hgt);
   else
      sprintf( buf, "%d. %s = %s/%d ", dpinx, buffer_selection[dp->bufinx],
         bp->filename,  (int)(Lc.divbydivisor ? bp->divisor : 1));

	gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_BLACK] );
#if USE_PANGO
	pango_layout_set_text (  W.fixed_font.layout, buf, -1);
	gdk_draw_layout (da->window, W.nor_gc, 2, 0, W.fixed_font.layout);
#else
   gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_BLACK] );
   gdk_draw_string( da->window, W.fixed_font, W.nor_gc, 2, 0, buf );
#endif
}

/********************************************************************************/
/*  dpytitle IO events                                                          */
/********************************************************************************/

/*------------------------------------------------------------------------
**  dpytitle_button_press_event() - catches mouse button presses....
**------------------------------------------------------------------------
*/
int dpytitle_button_press_event (GtkWidget *da, GdkEventButton *event, gpointer data )
{
   char buf[40];
   int dpinx = (intptr_t) data;

   sprintf( buf, "Active %d ", dpinx );
   cmd_execute( W.cmd.main_console, buf, FALSE );
   return TRUE;
}

/********************************************************************************/
/*  dpydata drawingArea's signal functions - drawing_area repaint, event, etc.  */
/********************************************************************************/
/*----------------------------------------------------------------------------
**  dpydata_configure_event() - configure event handler for dpydata Drawing Area
**----------------------------------------------------------------------------
*/
int dpydata_configure_event( GtkWidget *widget, GdkEventConfigure *event )
{
   return TRUE;
}
/*----------------------------------------------------------------------------
**  dpydata_expose_event() - expose event handler for dpydata Drawing Area
**----------------------------------------------------------------------------
*/
int dpydata_expose_event( GtkWidget *widget, GdkEventExpose *event )
{
   //static int i;
   //printf("dpydata_expose_event() %d type=%d\n", i++, event->type);

   dpydata_redraw( widget );
   return FALSE;
}

/*----------------------------------------------------------------------------
**  call_dpydata_redraw() - dv code calls this to refresh a dpydata area.
**----------------------------------------------------------------------------
*/
void call_dpydata_redraw( int dpinx )
{
   dpydata_redraw( Dpy[dpinx].data_drawingarea );
}

/*-------------------------------------------------------------------------------
**  redraw_dpydata_for_bufinx() - Calls the redraw function for the dpytitle_w
**      for all dpy using bufinx.
**-------------------------------------------------------------------------------
*/
void redraw_dpydata_for_bufinx( int bufinx )
{
   int dpinx;

   for( dpinx = 0; dpinx < Lc.num_dpy; dpinx++ )
      if( Dpy[dpinx].bufinx == bufinx )
         call_dpydata_redraw( dpinx );
}

/*---------------------------------------------------------------------------
** redraw_dpytype_stats_for_bufinx( int bufinx )
**-------------------------------------------------------------------------------
*/
void redraw_dpytype_stats_for_bufinx( int bufinx )
{
   int dpinx;

   for( dpinx = 0; dpinx < Lc.num_dpy; dpinx++ )
      if( (Dpy[dpinx].bufinx == bufinx)  && (Dpy[dpinx].dpytype==DPYTYPE_STATS) )
         call_dpydata_redraw( dpinx );
}

/*----------------------------------------------------------------------------
**  dpydata_redraw() - DPY.Title area's main redraw function.
**----------------------------------------------------------------------------
*/
void dpydata_redraw( GtkWidget *da )
{
   int dpinx;
   struct dpy_t *dp;
   struct df_buf_t *bp;

   //printf("dypdata_draw()\n");

   /* Check that the drawing area has been initialized. */
   if ( da->window == NULL )
      return;

   /* get display / data buffer info */
	dpinx = (intptr_t) g_object_get_data( G_OBJECT(da), DPYINX_DATA_KEY);
   dp = &Dpy[dpinx];
   bp = &Buffer[dp->bufinx];

   /* display information */
   if( bp->status == DF_EMPTY )
	{
	   // don't erase the pointer display.
      if ( Dpy[dpinx].dpytype != DPYTYPE_POINTER )
			draw_X( da, dp, bp );
	}
   else
   {
      switch( Dpy[dpinx].dpytype )
      {
         case DPYTYPE_IMAGE:
            draw_image( da, dp, bp );
            break;

         case DPYTYPE_HEADER:
            draw_header( da, dp, bp );
            break;

         case DPYTYPE_HISTOGRAM:
            draw_histogram( da, dp, bp );
            break;

         case DPYTYPE_LINECUT:
            draw_linecut( da, dp, bp );
            break;

         case DPYTYPE_XLINECUT:
            draw_xcut( da, dp, bp );
            break;

         case DPYTYPE_NOISE:
            draw_noise( da, dp, bp );
            break;

         case DPYTYPE_POINTER:
            draw_pointer( da, dp, bp );
            break;

         case DPYTYPE_STATS:
            draw_stats( da, dp, bp );
            break;

         case DPYTYPE_AOFIG:
            draw_aofig( da, dp, bp );
            break;

         case DPYTYPE_SA:
            draw_sa( da, dp, bp );
            break;

         case DPYTYPE_SB:
            draw_sb( da, dp, bp );
            break;

         default:
            draw_X( da, dp, bp );
            break;
      }
   }
   /*
   ** Update scroll bars...
   */
   update_scrollbars( dpinx );

   /*
   ** Update Dpytitle display
   */
   call_dpytitle_redraw( dpinx );
}

/********************************************************************************/
/*  dpydata IO events                                                           */
/********************************************************************************/

/*------------------------------------------------------------------------
**  dpydata_button_press_event() - catches mouse button presses....
**------------------------------------------------------------------------
*/
int dpydata_button_press_event (GtkWidget *da, GdkEventButton *event, gpointer data )
{
   int x, y,              /* xy location of pointer */
       da_wid, da_hgt;
   char buf[60];

   struct dpy_t *dp    = &Dpy[(intptr_t)data ];
   struct df_buf_t *bp = &Buffer[dp->bufinx];

   /* handle event for DPYTYPE_IMAGE and non-empty buffers, otherwise return */
   if( (dp->dpytype != DPYTYPE_IMAGE) || (bp->status == DF_EMPTY ))
      return TRUE;

   /* get info about event */
   x = event->x;
   y = event->y;

   /* get info drawing area */
   gdk_drawable_get_size( da->window, &da_wid, &da_hgt );

   /* convert (x,y) to pixel location; return if pointer is outside image  */
   {
      float pixel_wid = dp->image_zoom > 0 ? dp->image_zoom : 1.0/(1.0+abs(dp->image_zoom));
      x = dp->image_offx + ( x / pixel_wid);
      y = dp->image_offy + ( y / pixel_wid);

      if( !INRANGE( 0, x, bp->naxis1-1) || !INRANGE(0, y, bp->naxis2-1))
         return TRUE;
   }

   /*--------------------------------------------
   ** Screen for event and respond...
   */
   if( event->button == 1 )
   {
      /* Pushing button 1, centers the box at the cursor location.
      ** Also, let's grab the keyboard focus.
      */
      int wid = Stats[dp->bufinx].objwid;
      int hgt = Stats[dp->bufinx].objhgt;
      sprintf( buf, "StatsObjBox  %d %d %d %d %c ", x - (wid/2), y-(hgt/2), wid, hgt,  'A'+dp->bufinx);
      cmd_execute( W.cmd.main_console, buf, TRUE );

      if( Lc.gbox_cmd_right_click )
		{
			/* trigger the callback for the GuideBox A button */
			/* This will issue the 'GuideBox command using above StatsObjBox */
			inst_subarray_cb( da, (gpointer)3);
		}

      gtk_widget_grab_focus( da );
   }
   else if( event->button == 2 )
   {
      /* Pushing button 2, initializes the upper-left position of the Box
      */
      Box_x = x;
      Box_y = y;

#if 0
      /*-------------------------------------------------
      ** Update TCS offset coordinates.
      */
      Lc.offset_beg_x = x;
      Lc.offset_beg_y = y;
      if( Lc.usefitsanglescale )
      {
         Lc.offset_angle      = bp->pos_angle;
         Lc.offset_platescale = bp->arcsec_pixel;
      }
      cal_offset_radec( );
      update_offset_widgets();
#endif
   }
   return TRUE;
}

/*------------------------------------------------------------------------
**  dpydata_motion_notify_event() - catches mouse motion events....
**------------------------------------------------------------------------
*/
int dpydata_motion_notify_event (GtkWidget *da, GdkEventMotion *event, gpointer data )
{

   int dpinx = (intptr_t) data;
   struct dpy_t *dp    = &Dpy[dpinx];
   struct df_buf_t *bp = &Buffer[dp->bufinx];

   if( (bp->status == DF_EMPTY )) /* if empty buffers, return */
      return TRUE;

   if( dp->dpytype == DPYTYPE_IMAGE) 
		dpydata_motion_notify_event_for_ImageDpyType ( da, event, dpinx, dp, bp );
   if( dp->dpytype == DPYTYPE_AOFIG) 
		dpydata_motion_notify_event_for_AOFigDpyType ( da, event, dpinx, dp, bp );

	return TRUE;
}

/*------------------------------------------------------------------------
**  dpydata_motion_notify_event_for_ImageDpyType() - handles motion events for DpyType Image
**------------------------------------------------------------------------
*/

int dpydata_motion_notify_event_for_ImageDpyType ( 
   GtkWidget *da, 
   GdkEventMotion *event, 
   int dpinx,     
   struct dpy_t *dp,
   struct df_buf_t *bp
)
{
   int x, y,              /* xy location of pointer */
       da_wid, da_hgt,
       l;
   char buf[60];
   GdkModifierType state;
   struct ds_t ds_var;
   struct ds_t *ds;

   if ( da->window == NULL )
      return TRUE;


   // get drawing area infor   
   gdk_drawable_get_size( da->window, &da_wid, &da_hgt );

   /* get info about event */
   if (event->is_hint)
      gdk_window_get_pointer (event->window, &x, &y, &state);
   else
   {
      x = event->x;
      y = event->y;
      state = event->state;
   }

   /* convert (x,y) to pixel location; return if pointer is outside image  */
   {
      float pixel_wid = dp->image_zoom > 0 ? dp->image_zoom : 1.0/(1.0+abs(dp->image_zoom));
      x = dp->image_offx + ( x / pixel_wid);
      y = dp->image_offy + ( y / pixel_wid);

      if( !INRANGE( 0, x, bp->naxis1-1) || !INRANGE(0, y, bp->naxis2-1))
         return TRUE;
   }

   /*--------------------------------------------
   ** Screen for event and respond...
   */
   if( (state & (GDK_BUTTON2_MASK|GDK_SHIFT_MASK)) == (GDK_BUTTON2_MASK|GDK_SHIFT_MASK) )
   {
      /* Dragging the mouse while hold down button 2 & the shift Key,
      ** changes the xor line.
      */
      sprintf( buf, "StatsXORLine %d %d %d %d %c ", Box_x, Box_y, x, y, 'A'+dp->bufinx);
      cmd_execute( W.cmd.main_console, buf, TRUE );

      /*-------------------------------------------------
      ** Update offset coordinates.
      */
      Lc.offset_beg_x = Box_x;
      Lc.offset_beg_y = Box_y;
      Lc.offset_end_x = x;
      Lc.offset_end_y = y;
      if( Lc.usefitsanglescale )
      {
         Lc.offset_angle      = bp->pos_angle;
         Lc.offset_platescale = bp->arcsec_pixel;
      }
      cal_offset_radec( );
      update_offset_widgets();
   }
   else if( state & GDK_BUTTON2_MASK )
   {
      /* Dragging the mouse while hold down button 2, change the wid & hgt of the objbox.
      */
      int wid = x-Box_x+1;
      int hgt = y-Box_y+1;
      if( wid > 0 && hgt > 0 && !Stats[dp->bufinx].fixedWH)
      {
         sprintf( buf, "StatsObjBox %d %d %d %d %c ", Box_x, Box_y, x-Box_x+1, y-Box_y+1, 'A'+dp->bufinx);
         cmd_execute( W.cmd.main_console, buf, FALSE );
      }

      /*-------------------------------------------------
      ** Update offset coordinates.
      */
#if 0
       Lc.offset_end_x = x;
      Lc.offset_end_y = y;
      if( Lc.usefitsanglescale )
      {
         Lc.offset_angle      = bp->pos_angle;
         Lc.offset_platescale = bp->arcsec_pixel;
      }
      cal_offset_radec( );
      update_offset_widgets();
#endif
   }
   else if( state & GDK_BUTTON3_MASK )
   {
      /* mouse motion with Button 3 pressed, changes the colormap
      ** width and center variables. This is only practical for
      ** PSEUDO COLOR.
      **
      ** For TRUE COLOR, lets stretch and slide using the Range variables.
      */
      if( CM.visual->type == GDK_VISUAL_PSEUDO_COLOR )
      {
			float f;

			f = (float) event->x / da_wid;
			if( INRANGE( 0.0, f, 1.0 ))
				CM.center = f;

			f = (float) event->y / da_hgt;
			if( INRANGE( 0.01, f, 3.0 ))
				CM.width = f;

			cm_set_colormap( &CM );
      }
      else if ( CM.visual->type == GDK_VISUAL_TRUE_COLOR )
      {
         float center, width;
         float mean, min, max, sd3;

         center = (float) event->x / da_wid;
         width = (float) event->y / da_hgt;

         /* normal scale */
         sd3 = bp->stddev*3;
         mean = bp->mean;
         min = mean - sd3;
         max = mean + sd3;

         /* adjust for center/width */
         mean = map( center, 0.0, 1.0, min, max);
         sd3 =  map( width, 0.0, 1.0, 0, bp->stddev*6 );
         min = mean - sd3;
         max = mean + sd3;

         /* change range for this buffer */
         sprintf( buf, "ImageRange %5.3f %5.3f %d ", min, max, dpinx );
			cmd_execute( W.cmd.main_console, buf, FALSE );

         /* update any histogram image */
			for( l=0; l<Lc.num_dpy; l++)
			{
				if( (Dpy[l].dpytype == DPYTYPE_HISTOGRAM) && (Dpy[l].hist_auto_range==TRUE))
				{
					sprintf( buf, "ImageRange %5.3f %5.3f %d ", min, max, l );
			      cmd_execute( W.cmd.main_console, buf, FALSE );
				}
			}
      }
   }
   else
   {
      /* plain mouse motion, displays "[x,y] = value" in  title
      ** And Update the Pointer Display (if configured).
      */
      int   xp;
      float data = dfdataxy( bp, x, y );

      if( Lc.usehex )
         sprintf( buf, " [%3d,%3d]=0x%08x ", x, y, (int32_t)data);
      else
      {
         if( INRANGE(-10.0, data, 10.0))
            sprintf( buf, " [%3d,%3d]=%8.4f ", x, y, data);
         else if( INRANGE( -1000.0, data, 1000.0))
            sprintf( buf, " [%3d,%3d]=%8.2f ", x, y, data);
         else
            sprintf( buf, " [%3d,%3d]=%8.1f ", x, y, data);
      }

      l = strlen( buf );
      xp = da_wid - ((l-1)*W.fixed_font_wid);

      /* draw text on status window */
      if( dpinx == Lc.active )
      {
          gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_YELLOW]);
          gdk_draw_rectangle( dp->title_drawingarea->window, W.nor_gc, TRUE, xp, 1,
                   l*W.fixed_font_wid, W.fixed_font_hgt);
      }
      else
         gdk_window_clear_area( dp->title_drawingarea->window, xp, 1,
         l*W.fixed_font_wid, W.fixed_font_hgt);

      ds = init_ds( &ds_var, dp->title_drawingarea, W.nor_gc, 
                    &W.fixed_font, W.fixed_font_wid, W.fixed_font_hgt);
		da_set_foreground( ds, CM_BLACK );
		draw_text(ds, U_PIXEL, xp, 0,  buf);

      /*-------------------------------------------------
      ** Find display with pointer_display, and redraw.
      */
      for( l=0; l<Lc.num_dpy; l++)
      {
         if( (Dpy[l].dpytype == DPYTYPE_POINTER))
            draw_pointer_update( Dpy[l].data_drawingarea, l, x, y, dpinx );
      }
   }

   return TRUE;
}

/*------------------------------------------------------------------------
**  dpydata_motion_notify_event_for_AOFigDpyType() - handles motion events for DpyType AOFig
**------------------------------------------------------------------------
*/

int dpydata_motion_notify_event_for_AOFigDpyType ( 
   GtkWidget *da, 
   GdkEventMotion *event, 
   int dpinx,     
   struct dpy_t *dp,
   struct df_buf_t *bp
)
{
   int x, y,              /* xy location of pointer */
       inx,
       da_wid, da_hgt,
       char_wid, char_hgt,
       l;
   polygon_t * ptable;
   GdkModifierType state;

   struct ds_t ds_var;
   struct ds_t *ds;

   /* feedback for DM & Sensor display, not text */
	ptable = NULL;
   if( dp->aofig_format == AOFIG_FORMAT_DM )
      ptable = AO_DM_Table;
   else if( dp->aofig_format == AOFIG_FORMAT_SENSOR )
      ptable = AO_Sensor_Table;
   else if( dp->aofig_format == AOFIG_FORMAT_TEXT )
      return TRUE;

   /* get info about event */
   if (event->is_hint)
      gdk_window_get_pointer (event->window, &x, &y, &state);
   else
   {
      x = event->x;
      y = event->y;
      state = event->state;
   }

   /*--------------------------------------------
   ** basic DA stuff 
   */
	gdk_drawable_get_size( da->window, &da_wid, &da_hgt );
   char_wid = W.fixed_font_wid;
   char_hgt = W.fixed_font_hgt;

   {
      double polygon[5][2];
      double point[2];
      int    found,
             fx, fy, fw, fh,
             e, p;

      // the figure is in this x,y,w,h box
      fx = char_wid/2;
      fy = char_hgt/2;
      fw = da_wid - char_wid;
      fh = da_hgt - char_hgt;
      fw = MIN( fw, fh);   // the figure square is square.
      fh = MIN( fw, fh);

      found = FALSE;
      for( e=0; (e < AOFIG_NUM_ELEMENTS) && (!found); e++ )
      {
         // Map polygon definition to pixel in drawing area.
         for( p=0; p < ptable[e].n; p++)
         {
				polygon[p][0] = map( ptable[e].pt[p].x, 0, 1000, fx, fx+fw);
				polygon[p][1] = map( ptable[e].pt[p].y, 0, 1000, fy, fy+fh);
         }

         // Is point in current polygon (ptable[e]) ?
         point[0] = x;
         point[1] = y;
         if( PointInPolygon( polygon, ptable[e].n, point))
            found = TRUE;
      }

      if( !found )
		{
			return TRUE;
		}

      inx = (e-1);  // inx the index to into the ao diagram.
   }

   /*--------------------------------------------
   ** convert xy from pixel values to data[x,y]
   */
   {
      int data_inx;

      data_inx = (dp->aofig_y * bp->naxis1 ) + dp->aofig_x;
      if(  dp->aofig_data )
         data_inx += inx*bp->naxis1;  // increment by Y
      else
         data_inx += inx;             // increment by X

      x = data_inx % bp->naxis1;      // convert to data[x,y]
      y = data_inx / bp->naxis1;

   }

   /*--------------------------------------------
   ** display location and data value.
   */
   {
      /* plain mouse motion, displays "[x,y] = value" in  title
      ** And Update the Pointer Display (if configured).
      */
      int   xp;
      char buf_val[40];
      char buf[60];


      double2str_len( buf_val, dfdataxy( bp, x, y ), 9, 2);
		sprintf( buf, " (%d) [%3d,%3d]=%s", inx, x, y, buf_val);

      l = strlen( buf );
      xp = da_wid - ((l-1)*W.fixed_font_wid);

      /* draw text on status window */
      if( dpinx == Lc.active )
      {
          gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_YELLOW]);
          gdk_draw_rectangle( dp->title_drawingarea->window, W.nor_gc, TRUE, xp, 1,
                   l*W.fixed_font_wid, W.fixed_font_hgt);
      }
      else
         gdk_window_clear_area( dp->title_drawingarea->window, xp, 1,
         l*W.fixed_font_wid, W.fixed_font_hgt);

      ds = init_ds( &ds_var, dp->title_drawingarea, W.nor_gc, 
                    &W.fixed_font, W.fixed_font_wid, W.fixed_font_hgt);
		da_set_foreground( ds, CM_BLACK );
		draw_text(ds, U_PIXEL, xp, 0, buf);
   }

   return TRUE;
}

/*------------------------------------------------------------------------
**  dpydata_key_press_event() - catches keyboard presses....
**------------------------------------------------------------------------
*/
int dpydata_key_press_event (GtkWidget *da, GdkEventKey *event, gpointer data )
{
   int x, y,              /* xy location of pointer */
       dx, dy,
       da_wid, da_hgt;
   GdkModifierType state;
   char buf[60];

   struct dpy_t *dp    = &Dpy[(intptr_t) data ];
   struct df_buf_t *bp = &Buffer[dp->bufinx];

   /* handle event for DPYTYPE_IMAGE and non-empty buffers, otherwise return */
   if( (dp->dpytype != DPYTYPE_IMAGE) || (bp->status == DF_EMPTY ))
      return TRUE;

   /* get info about event & drawing area */
   gdk_window_get_pointer (event->window, &x, &y, &state);
   gdk_drawable_get_size( da->window, &da_wid, &da_hgt );

   /* convert (x,y) to pixel location; return if pointer is outside image  */
   {
      float pixel_wid = dp->image_zoom > 0 ? dp->image_zoom : 1.0/(1.0+abs(dp->image_zoom));
      x = dp->image_offx + ( x / pixel_wid);
      y = dp->image_offy + ( y / pixel_wid);

      if( !INRANGE( 0, x, bp->naxis1-1) || !INRANGE(0, y, bp->naxis2-1))
         return TRUE;
   }

   /*--------------------------------------------
   ** Screen for event and respond...
   */
   /* dx, dy signal a change in position of the objbox */
   dx = 0;
   dy = 0;

   switch (event->keyval)
   {
      case GDK_Up:
         /* Up arrow move the objbox 1 pixel up */
         dy = -1;
      break;

      case GDK_Down:
         /* Down arrow move the objbox 1 pixel down */
         dy = 1;
      break;

      case GDK_Left:
         /* Left arrow move the objbox 1 pixel left */
         dx = -1;
      break;

      case GDK_Right:
         /* Right arrow move the objbox 1 pixel right */
         dx = 1;
      break;

      case 'L':
      case 'l':
         sprintf( buf, "LCutXY %d %d ", x, y);
         cmd_execute( W.cmd.main_console, buf, TRUE );
      break;

      case 'F':
      case 'f':
			/*-------------------------------------------------
			** Update offset coordinates.
			*/
			Lc.offset_beg_x = x;
			Lc.offset_beg_y = y;
			if( Lc.usefitsanglescale )
			{
				Lc.offset_angle      = bp->pos_angle;
				Lc.offset_platescale = bp->arcsec_pixel;
			}
			cal_offset_radec( );
			update_offset_widgets();
      break;

      case 'T':
      case 't':
			/*-------------------------------------------------
			** Update offset coordinates.
			*/
			Lc.offset_end_x = x;
			Lc.offset_end_y = y;
			if( Lc.usefitsanglescale )
			{
				Lc.offset_angle      = bp->pos_angle;
				Lc.offset_platescale = bp->arcsec_pixel;
			}
			cal_offset_radec( );
			update_offset_widgets();
   }

   /* if dx, dy has changed, move objbox */
   if( dx || dy )
   {
      sprintf( buf, " StatsObjBox %d %d %d %d %c ",
         Stats[dp->bufinx].objx+dx, Stats[dp->bufinx].objy+dy,
         Stats[dp->bufinx].objwid, Stats[dp->bufinx].objhgt,
         'A'+dp->bufinx);
      cmd_execute( W.cmd.main_console, buf, FALSE );
   }

   return TRUE; /* TRUE prevent these keypress event from affecting the focus. */
}

/*------------------------------------------------------------------------
**  dpydata_focus_in/out_event() -
**------------------------------------------------------------------------
*/
int dpydata_focus_in_event (GtkWidget *da, GdkEventFocus *event, gpointer data )
{
   GTK_WIDGET_SET_FLAGS( da, GTK_HAS_FOCUS);
   return FALSE;
}
int dpydata_focus_out_event (GtkWidget *da, GdkEventFocus *event, gpointer data )
{
   GTK_WIDGET_UNSET_FLAGS( da, GTK_HAS_FOCUS);
   return TRUE;
}

/********************************************************************************/
/*  General datadata drawing routines.                                          */
/********************************************************************************/

/*----------------------------------------------------------------------------
**  draw_X() - clear display & draw a frame on outer edge.
**----------------------------------------------------------------------------
*/
void draw_X( GtkWidget *da, struct dpy_t *dp, struct df_buf_t *bp )
{
   int da_wid, da_hgt;

	//printf("draw_X\n");

   /* Check that the drawing area has been initialized. */
   if ( da->window == NULL )
      return;

   gdk_drawable_get_size( da->window, &da_wid, &da_hgt);

   /* clears window */
   gdk_window_clear( da->window );

   /* draw a frame in window  */
   gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_BLACK] );
   gdk_draw_rectangle( da->window, W.nor_gc, FALSE, 0, 0, da_wid-1, da_hgt-1);
}

/*----------------------------------------------------------------------------
**  draw_no_data() - clear display & say file contains no pixel data.
**----------------------------------------------------------------------------
*/
void draw_no_data( GtkWidget *da, struct dpy_t *dp, struct df_buf_t *bp )
{
   int da_wid, da_hgt;
   struct ds_t ds_var;
   struct ds_t *ds;

   /* Check that the drawing area has been initialized. */
   if ( da->window == NULL )
      return;

   // some basic stuff for DA's
   gdk_drawable_get_size( da->window, &da_wid, &da_hgt );
   ds = init_ds( &ds_var, da, W.nor_gc, &W.fixed_font, W.fixed_font_wid, W.fixed_font_hgt);

   /* clears window */
   gdk_window_clear( da->window );

   /* draw a frame in window  */
   gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_BLACK] );
   gdk_draw_rectangle( da->window, W.nor_gc, FALSE, 0, 0, da_wid-1, da_hgt-1);

      
   draw_text(ds, U_PIXEL, 2*W.fixed_font_wid,  2*W.fixed_font_hgt, "Data Buffer has NO pixel data.");
   return;
}


/*----------------------------------------------------------------------------
**  draw_image() - Display the FITS data as an Image.
**----------------------------------------------------------------------------
*/
void draw_image( GtkWidget *da, struct dpy_t *dp, struct df_buf_t *bp )
{
   int dpinx,
       da_wid, da_hgt;
   GdkImage *image;

   struct ds_t ds_var;
   struct ds_t *ds;

   /* Check that the drawing area has been initialized. */
   if ( da->window == NULL )
      return;

	dpinx = (intptr_t) g_object_get_data( G_OBJECT(da), DPYINX_DATA_KEY);
   gdk_drawable_get_size( da->window, &da_wid, &da_hgt);
   ds = init_ds( &ds_var, da, W.nor_gc, &W.fixed_font, W.fixed_font_wid, W.fixed_font_hgt);

   // no data? 
	if( bp->N == 0 ) 
   {
      draw_no_data( da, dp, bp);
      return;
	}

   /*----------------------------------------
   ** check/set autoscaling 
   */
   autoscale_image_range( dp, bp );

   /*-----------------------------------------------------------
   ** allocate image buffer & call appropriate drawing routine.
   */

   image = gdk_image_new( GDK_IMAGE_NORMAL, CM.visual, da_wid, da_hgt );
   if( image == NULL )
      return;
   // printf("draw_image(): image: %dx%dx%d\n", image->width, image->height, image->depth );

   if( (CM.visual->type == GDK_VISUAL_PSEUDO_COLOR)  && (dp->image_zoom >= 1) )
      draw_image_plus_zoom_pseudocolor( dp, bp, da_wid, da_hgt, (unsigned char*)image->mem);
   else if( (CM.visual->type == GDK_VISUAL_TRUE_COLOR)  && (dp->image_zoom >= 1) )
      draw_image_plus_zoom_truecolor( dp, bp, da_wid, da_hgt, (int32_t*)image->mem);
   else if( dp->image_zoom < 0 )
      draw_image_minus_zoom( dp, bp, da_wid, da_hgt, (unsigned char*)image->mem);
   else
     goto LNotSupported;

   /* transfer image to window & destroy image */
   gdk_draw_image( da->window, W.nor_gc, image, 0, 0, 0, 0, da_wid, da_hgt);
   //gdk_image_destroy( image );
   g_object_unref( image );

   /*---------------------------------------
   **  Draw the XOR box and line
   */
   draw_xor_box_on_dpy( dpinx, Stats[dp->bufinx].objx,   Stats[dp->bufinx].objy,
                               Stats[dp->bufinx].objwid, Stats[dp->bufinx].objhgt);
   draw_xor_line_on_dpy( dpinx, Stats[dp->bufinx].ln_xbeg, Stats[dp->bufinx].ln_ybeg,
                                Stats[dp->bufinx].ln_xend, Stats[dp->bufinx].ln_yend);

   /*---------------------------------------
   **  Draw rotation & platescale map. At 35,35 draw a north and 
	**  east line 25 pixel in lenght.
   */
	if( Lc.image_compass_show )
	{
		double x,y, a, b;
		int ns_dir;

      // we can flip the NS axis by setting the value for ns_dir to 1 or -1.
		ns_dir = (Lc.image_compass_flipNS ? -1 : 1 );  

	   gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_GREEN] );

      /* draw north line & 'N'*/
		rotate_pt( &x, &y, 0,  ns_dir*25, bp->pos_angle);
		gdk_draw_line( da->window, W.nor_gc, 35, 35, 35+x, 35-y);
		rotate_pt( &a, &b, -5*ns_dir, -5*ns_dir, bp->pos_angle);
		gdk_draw_line( da->window, W.nor_gc, 35+x, 35-y, 35+x+a, 35-(y+b));
		rotate_pt( &a, &b, +5*ns_dir, -5*ns_dir, bp->pos_angle);
		gdk_draw_line( da->window, W.nor_gc, 35+x, 35-y, 35+x+a, 35-(y+b));

		rotate_pt( &x, &y, 0, ns_dir*30, bp->pos_angle);
      draw_text(ds, U_PIXEL, 35+x-(W.fixed_font_wid/2), 35-y-(W.fixed_font_hgt/2), "N");

      /* draw east */
		rotate_pt( &x, &y, -25, 0, bp->pos_angle);
		gdk_draw_line( da->window, W.nor_gc, 35, 35, 35+x, 35-y);

		rotate_pt( &a, &b, +5, +5, bp->pos_angle);
		gdk_draw_line( da->window, W.nor_gc, 35+x, 35-y, 35+x+a, 35-(y+b));
		rotate_pt( &a, &b, +5, -5, bp->pos_angle);
		gdk_draw_line( da->window, W.nor_gc, 35+x, 35-y, 35+x+a, 35-(y+b));

		rotate_pt( &x, &y, -30, 0, bp->pos_angle);
      draw_text(ds, U_PIXEL, 35+x-(W.fixed_font_wid/2), 35-y-(W.fixed_font_hgt/2), "E");
	}

   /*--------------------------------------------------------------------------
   **  if the data buffer gbox_enable is TRUE, draw the guidebox on the image
   */
	if( Lc.image_show_gbox && bp->gbox_enable )
	{
	   draw_box_on_dpy( dpinx, bp->gbox_dim[0], bp->gbox_dim[1],
			              bp->gbox_dim[2], bp->gbox_dim[3], CM_GREEN);
      /* gbox to is a 'X' */
      //printf("gbox_to %3.1f, %3.1f \n", bp->gbox_to[0], bp->gbox_to[1]);
	   draw_line_on_dpy( dpinx, bp->gbox_to[0]+0.5, bp->gbox_to[1]-1.5,
			                      bp->gbox_to[0]+0.5, bp->gbox_to[1]+2.5, CM_GREEN);
	   draw_line_on_dpy( dpinx, bp->gbox_to[0]-1.5, bp->gbox_to[1]+0.5,
			                      bp->gbox_to[0]+2.5, bp->gbox_to[1]+0.5, CM_GREEN);
      /* gbox from */
	   draw_box_on_dpy( dpinx, bp->gbox_from[0], bp->gbox_from[1], 1, 1, CM_MAGENTA);
	   draw_line_on_dpy( dpinx, bp->gbox_from[0]+0.5, bp->gbox_from[1]+0.5,
			                      bp->gbox_to[0]+0.5,   bp->gbox_to[1]+0.5, CM_MAGENTA);
	}

   return;

LNotSupported:
   //gdk_image_destroy( image );
   g_object_unref( image );

   /* clears window */
   gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_BLACK] );
   gdk_draw_rectangle( da->window, W.nor_gc, TRUE, 0, 0, da_wid, da_hgt);

	draw_text(ds, U_PIXEL, 10, 20, 
      "depth=%d  Zoom=%d - This mode is not yet supported.", image->depth, dp->image_zoom );
}

/*----------------------------------------------------------------------------
**  autoscale_image_range() - autoscales the image_min,max.
**----------------------------------------------------------------------------
*/
void autoscale_image_range( struct dpy_t *dp, struct df_buf_t *bp )
{
   float sd3, diff, h;

   if( !dp->image_autoscale )
      return;
	if( bp->status == DF_EMPTY )
      return;

   sd3 = bp->stddev*3;

	dp->image_min = bp->mean - sd3;
	dp->image_max = bp->mean + sd3;

   diff = dp->image_max - dp->image_min;
	if( diff < 1.0 )
	{
	   h = 1.0 - diff;
		dp->image_max += h/2; 
		dp->image_min -= h/2;
	}
}

/*----------------------------------------------------------------------
** draw_image_plus_zoom_pseudocolor() - Handles 8bit pseudocolor,
**  positive zooming for draw_image().
**  This function fill in the image memory buffer for display.
**----------------------------------------------------------------------
*/
void draw_image_plus_zoom_pseudocolor(
   struct dpy_t *dp,          /* pointer to Dpy[] */
   struct df_buf_t *bp,       /* pointer to Buffer[] */
   int da_wid,                /* size of draw area */
   int da_hgt,
   unsigned char * image_buf  /* allocated memory of buffer to fill */
)
{
   int    ximg, yimg,        /* x & y position in the image buffer  */
          wid_pix,           /* Size of screen pixels               */
          xarray, yarray,    /* controls which pixels to draw       */
          color,             /* color of the pixel                  */
          inx,               /* index to Buffer[bufinx].data[]       */
          wid,
          bitmap_pad,
          i;
   float  block;                  /* amount of count per color    */

   unsigned char * start_line;

   block = (dp->image_max - dp->image_min) / ((float)CM_NUM_RW_COLORS);
   wid_pix = dp->image_zoom;

   /* find bitmap_pad - amount of padding need per image->mem scan line, which are mod_32 in size */
   /* Padding is need since image->mem is an array of char (8bits) */
   i = da_wid % sizeof(int32_t);
   bitmap_pad =  (i ? sizeof(int32_t)-i : 0 );

   /* Adjust scroll bar parameters to fit to minimumize blank space */
   i = ( da_wid - ((bp->naxis1 - dp->image_offx ) * wid_pix) ) / wid_pix;
   if( i > 0 )
      dp->image_offx = MAX( 0, dp->image_offx-i);
   i = ( da_hgt - ((bp->naxis2 - dp->image_offy ) * wid_pix) ) / wid_pix;
   if( i > 0 )
      dp->image_offy = MAX( 0, dp->image_offy-i);

   /*---------------------------------------------------
   ** Loop through data & build display in XImage buffer.
   */
   for( yarray = dp->image_offy, yimg = 0;
        yarray < bp->naxis2 && yimg < da_hgt;
        yarray++ )
   {
      inx = yarray * bp->naxis1 + dp->image_offx;
      start_line = image_buf + (yimg * (da_wid+bitmap_pad));

      /*---------------------------------------
      **  Fill in the first row of the line.
      */
      for( xarray = dp->image_offx, ximg = 0;
           xarray < bp->naxis1 && ximg < da_wid;
           xarray++, inx ++)
      {
         float data = dfdatainx( bp, inx );

         if( data >= dp->image_max )
            color =  CM_NUM_COLORS-1;  /* last color */
         else if( data <= dp->image_min )
            color = CM_NUM_STATIC_COLORS; /* first color */
         else
            color = ( (data - dp->image_min) / block) + CM_NUM_STATIC_COLORS;

         wid = ( ximg+wid_pix < da_wid ) ? wid_pix : da_wid-ximg; /* screen pixel per data pixel */
         memset( (char*) start_line+ximg, CM.colors[color].pixel, wid ); /* apply to image */
         ximg += wid;     /* update ximg */
      }

      /* black out rest of scan line */
		if( (da_wid-ximg) > 0 )
			memset( (char*)start_line+ximg, CM.colors[CM_BLACK].pixel, da_wid-ximg);

      /* Update yimg and copy wid_pix-1 lines for zoom. */
      yimg++;
      for( i=1; i < wid_pix && yimg < da_hgt; i++)
      {
         int size = da_wid+bitmap_pad;
         memcpy( (char*)start_line+(i*size), (char*) start_line, da_wid);
         yimg++;
      }
   }

   /*--------------------------------------------------------------
   ** clear rest of buffer if image is smaller than drawing area
   */
   if( yimg < da_hgt )
   {
      int size = da_wid+bitmap_pad;
      memset( (char*)image_buf+(yimg*size), CM.colors[CM_BLACK].pixel, (da_wid*da_hgt)-(yimg*size));
   }
}

/*-------------------------------------------------------------------------------
** draw_image_plus_zoom_truecolor() - Handles true color, positive zooming
**    for draw_image().
**-------------------------------------------------------------------------------
*/
void draw_image_plus_zoom_truecolor(
   struct dpy_t *dp,          /* pointer to Dpy[] */
   struct df_buf_t *bp,       /* pointer to Buffer[] */
   int da_wid,                /* size of draw area */
   int da_hgt,
   int32_t * image_buf  /* allocated memory of buffer to fill */
)
{
   int    ximg, yimg,        /* x & y position in the image buffer  */
          wid_pix,           /* Size of screen pixels               */
          xarray, yarray,    /* controls which pixels to draw       */
          color,             /* color of the pixel                  */
          inx,               /* index to Buffer[bufinx].data[]       */
          wid,
          i;
   float  block;                  /* amount of count per color    */

   int32_t * start_line;

   block = (dp->image_max - dp->image_min) / ((float)CM_NUM_RW_COLORS);
   wid_pix = dp->image_zoom;

   /* Adjust scroll bar parameters to fit to minimumize blank space */
   i = ( da_wid - ((bp->naxis1 - dp->image_offx ) * wid_pix) ) / wid_pix;
   if( i > 0 )
      dp->image_offx = MAX( 0, dp->image_offx-i);
   i = ( da_hgt - ((bp->naxis2 - dp->image_offy ) * wid_pix) ) / wid_pix;
   if( i > 0 )
      dp->image_offy = MAX( 0, dp->image_offy-i);

   /*---------------------------------------------------
   ** Loop through data & build display in XImage buffer.
   */
   for( yarray = dp->image_offy, yimg = 0;
        yarray < bp->naxis2 && yimg < da_hgt;
        yarray++ )
   {
      inx = yarray * bp->naxis1 + dp->image_offx;
      start_line = image_buf + (yimg * da_wid);

      /*---------------------------------------
      **  Fill in the first row of the line.
      */
      for( xarray = dp->image_offx, ximg = 0;
           xarray < bp->naxis1 && ximg < da_wid;
           xarray++, inx ++)
      {
         float data = dfdatainx( bp, inx );

         if( data >= dp->image_max )
            color =  CM_NUM_COLORS-1;  /* last color */
         else if( data <= dp->image_min )
            color = CM_NUM_STATIC_COLORS; /* first color */
         else
            color = ( (data - dp->image_min) / block) + CM_NUM_STATIC_COLORS;

         wid = ( ximg+wid_pix < da_wid ) ? wid_pix : da_wid-ximg; /* screen pixel per data pixel */
         i32_set( (uint32_t*)start_line+ximg, 
                  cm_swap_int32( CM.colors[color].pixel, CM.swap_rgb), 
                  wid ); /* apply to image */
         ximg += wid;     /* update ximg */
      }

      /* black out rest of scan line */
		if( (da_wid-ximg) > 0 )
			i32_set( (uint32_t*)start_line+ximg, 
			         cm_swap_int32( CM.colors[CM_BLACK].pixel, CM.swap_rgb), 
			         da_wid-ximg);

      /* Update yimg and copy wid_pix-1 lines for zoom. */
      yimg++;
      for( i=1; i < wid_pix && yimg < da_hgt; i++)
      {
         memcpy( start_line+(i*da_wid), start_line, da_wid*sizeof(int32_t));
         yimg++;
      }
   }

   /*--------------------------------------------------------------
   ** clear rest of buffer if image is smaller than drawing area
   */
   if( yimg < da_hgt )
   {
      memset( image_buf+(yimg*da_wid), 
				  cm_swap_int32( CM.colors[CM_BLACK].pixel, CM.swap_rgb), 
              ((da_wid*da_hgt)-(yimg*da_wid))*sizeof(int32_t) );
   }
}

/*----------------------------------------------------------------------
** draw_image_minus_zoom() - Handles negative zooming for draw_image()
**   This function fill in the XImage memory buffer for display.
**----------------------------------------------------------------------
*/
void draw_image_minus_zoom(
   struct dpy_t *dp,       /* pointer to Dpy[]    */
   struct df_buf_t *bp,    /* pointer to Buffer[] */
   int  da_wid,            /* size of draw area   */
   int  da_hgt,
   unsigned char * image_buf  /* allocated memory of XImage buffer to  fill */
)
{
   int yarray, xarray,    /* reference to data */
       xda,  yda,         /* reference in drawing area */
       x, y,
       zoom, color, i;

   double  data,
           block;          /* amount of count per color */

   uint32_t * i32_buf;

   i32_buf = (uint32_t *)image_buf; /* for true color use i32_buf[] */
   block = (dp->image_max - dp->image_min) / ((float)CM_NUM_RW_COLORS);
   zoom = abs(dp->image_zoom) + 1;

   /* Adjust scroll bar parameters to fit to minimumize blank space */
   i = ( da_wid - ((bp->naxis1 - dp->image_offx ) / zoom) ) * zoom;
   if( i > 0 )
      dp->image_offx = MAX( 0, dp->image_offx-i);
   i = ( da_hgt - ((bp->naxis2 - dp->image_offy ) / zoom) ) * zoom;
   if( i > 0 )
      dp->image_offy = MAX( 0, dp->image_offy-i);

   /*---------------------------------------------------
   ** Loop through data & build display in XImage buffer.
   */
   yarray = dp->image_offy;
   for( yda = 0; (yda < da_hgt) && (yarray < bp->naxis2); yda++ )
   {
      xarray = dp->image_offx;
      for( xda = 0; (xda < da_wid) && (xarray < bp->naxis1); xda++ )
      {
         /* sum image data for this pixel */
         data = 0;
         for( y=0; y<zoom && yarray+y < bp->naxis2; y++ )
         {
            i = (yarray * bp->naxis1) + xarray;
            for( x=0; x<zoom && xarray+x < bp->naxis1; x++ )
               data += dfdatainx( bp, i++ );
         }
         xarray += zoom; /* Increment X pixel reference */

         /* Divided data by number of pixel */
         data /= (float) (zoom*zoom);

         /* find color and store value in image buffer */
         if( data >= dp->image_max )
            color = CM_NUM_COLORS-1;      /* last color */
         else if( data <= dp->image_min )
            color = CM_NUM_STATIC_COLORS; /* first color */
         else
            color = ( (data - dp->image_min) / block) + CM_NUM_STATIC_COLORS;

         if( CM.visual->type == GDK_VISUAL_PSEUDO_COLOR )
            image_buf[ yda*da_wid + xda] = CM.colors[color].pixel;
         else
            i32_buf[ yda*da_wid + xda] = 
               cm_swap_int32( CM.colors[color].pixel, CM.swap_rgb);
      }

      /* clear rest of row */
      if( CM.visual->type == GDK_VISUAL_PSEUDO_COLOR )
         memset( image_buf+(yda*da_wid+xda), CM.colors[CM_BLACK].pixel, da_wid-xda);
      else
         i32_set( i32_buf+(yda*da_wid+xda), 
                  cm_swap_int32(CM.colors[CM_BLACK].pixel, CM.swap_rgb), 
                  da_wid-xda);

      yarray += zoom; /* Increment Y pixel reference */
   }

   /* clear rest of image buf */
   if( CM.visual->type == GDK_VISUAL_PSEUDO_COLOR )
      memset( image_buf+(yda*da_wid), CM.colors[CM_BLACK].pixel, (da_hgt-yda)*da_wid);
   else
      i32_set( i32_buf+(yda*da_wid), 
               cm_swap_int32(CM.colors[CM_BLACK].pixel, CM.swap_rgb),
               (da_hgt-yda)*da_wid);
}

/*--------------------------------------------------------------
** i32_set() - set the value of an array of int32_t in memory.
*/
void i32_set( uint32_t * buf, uint32_t value, int n)
{
   int i;

   if( n < 0 ) return;

    for( i=0; i<n; i++)
       *buf++ = value;
}


/*----------------------------------------------------------------------------
**  draw_header() - Display the FITS Header information.
**----------------------------------------------------------------------------
*/
void draw_header( GtkWidget *da, struct dpy_t *dp, struct df_buf_t *bp )
{
   char buf[85];
   int row,
       line,
       l,
       char_wid, char_hgt,
       da_wid, da_hgt;
   char * cptr;
   struct df_fheader_t *fhdr; /* Pointer to fits header structure */

   struct ds_t ds_var;
	struct ds_t *ds;

   /* Check that the drawing area has been initialized. */
   if ( da->window == NULL )
      return;

   ds = init_ds( &ds_var, da, W.nor_gc, &W.fixed_font, W.fixed_font_wid, W.fixed_font_hgt);
   char_wid = W.fixed_font_wid;
   char_hgt = W.fixed_font_hgt;
   gdk_drawable_get_size( da->window, &da_wid, &da_hgt);

   /* clears window */
   gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_BLACK] );
   gdk_draw_rectangle( da->window, W.nor_gc, TRUE, 0, 0, da_wid, da_hgt);

   row  = 0;
   line = 0;
   da_set_foreground( ds, CM_GREEN);
   da_set_background( ds, CM_BLACK);

   /* loop throught fits head & display each line */
   fhdr = bp->fheader;
   while( (fhdr != NULL) && (row < da_hgt) )
   {
      cptr = fhdr->buf;
      for( l=0; (l < (DF_FITS_RECORD_LEN/80)) && (row < da_hgt); l++ )
      {
         if( line >= dp->header_row )
         {
            memcpy( buf, cptr+dp->header_col, 80-dp->header_col);
            buf[80 - dp->header_col] = 0x00;
				draw_image_text(ds, U_PIXEL,  2, row, buf );
				row += char_hgt;
         }
         line++;
         cptr += 80;
      }
      fhdr = fhdr->next;  /* get next header block  */
   }
}

/*----------------------------------------------------------------------
** get_histogram_info() - Calculates the histogram hist info.
**----------------------------------------------------------------------
*/
int get_histogram_info(
   struct dpy_t *dp,         /* display parmaters - Must have value fits buffer  */
   struct hist_info *hist    /* This function returns this histogram info */
)
{
   int i,
       x_beg, x_numpixel,
       y_beg, y_numpixel,
		 min_hist_bin,
       x1, y1;

   struct df_buf_t *bp;
   bp = &Buffer[dp->bufinx];    /* buffer pointer */


   /* get range of pixel to consider for histogram */
   if( dp->hist_area )
   {
      x_beg = Stats[dp->bufinx].objx;
      x_numpixel = MIN( Stats[dp->bufinx].objwid, Buffer[dp->bufinx].naxis1-x_beg);
      x_numpixel = MAX( 1, x_numpixel);
      y_beg = Stats[dp->bufinx].objy;
      y_numpixel = MIN( Stats[dp->bufinx].objhgt, Buffer[dp->bufinx].naxis1-y_beg);
      y_numpixel = MAX( 1, y_numpixel);
   }
   else
   {
      x_beg = 0;
      x_numpixel = bp->naxis1;
      y_beg = 0;
      y_numpixel = bp->naxis2;
   }

   /*
   **  Initizlize the hist_info structure.
   */
   hist->pixel_low = 0;
   hist->pixel_high = 0;
   hist->pixel_graph = 0;
   hist->pixel_total = 0;
   hist->max_bin_value = 0;

   hist->num_of_bins = dp->hist_bin;
	min_hist_bin = ceil(dp->image_max-dp->image_min);  // make number_of_bin >= range.
	if(  hist->num_of_bins  > min_hist_bin )
      hist->num_of_bins = min_hist_bin;
   memset( hist->bins, 0, hist->num_of_bins*sizeof(hist->bins[0]));

   /*--------------------------------------------------
   ** Loop through pixel and count the bins
   */
   for( y1 = y_beg; y1 < y_beg+y_numpixel; y1++)
   {
      i = y1 * bp->naxis1 + x_beg;
      for( x1 = x_beg; x1 < x_beg+x_numpixel; x1++)
      {
         float data;
         int bin;

         data = dfdatainx( bp, i);

         bin = (int)rint(map( data, dp->image_min, dp->image_max, 0, hist->num_of_bins-1));
         if( bin < 0 )
            hist->pixel_low++;
         else if ( bin >= hist->num_of_bins )
            hist->pixel_high++;
         else
         {
            hist->pixel_graph++;
            hist->bins[bin]++;
            hist->max_bin_value = MAX( hist->max_bin_value, hist->bins[bin]);
         }

         i++;
         hist->pixel_total++;
      }
   }

   return ERR_NONE;
}

/*----------------------------------------------------------------------
** draw_histogram() - Display a histogram in the drawing area.
**----------------------------------------------------------------------
*/
void draw_histogram( GtkWidget *da, struct dpy_t *dp, struct df_buf_t *bp )
{
   int i, l,
       char_wid, char_hgt,
       da_wid, da_hgt,
       x1, y1, x2, y2,
       xmin, xmax, ymin, ymax;

   float max_percent,
         f;
   char buf[85];
   struct hist_info hist;        /* histogram info */

   struct ds_t ds_var;
   struct ds_t *ds;
      
   // no data? 
	if( bp->N == 0 ) 
   {
      draw_no_data( da, dp, bp);
      return;
	}

   // some basic stuff for DA's
   gdk_drawable_get_size( da->window, &da_wid, &da_hgt );
   ds = init_ds( &ds_var, da, W.nor_gc, &W.fixed_font, W.fixed_font_wid, W.fixed_font_hgt);
   char_wid = W.fixed_font_wid;
   char_hgt = W.fixed_font_hgt;

   /*----------------------------------------
   ** check/set autoscaling 
   */
   autoscale_image_range( dp, bp );


   gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_BLACK] );
   gdk_draw_rectangle( da->window, W.nor_gc, TRUE, 0, 0, da_wid, da_hgt );
   gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_YELLOW] );

   /*-----------------------------------
   ** Get Histogram info
   */
   if( ERR_NONE != get_histogram_info( dp, &hist ))
   {
		draw_text(ds, U_PIXEL, char_wid, char_hgt, "OPPS, get_histogram_info(): error!");
      return;
   }

   /*-----------------------------------
   ** Find max percent to nearest 5%
   */
   max_percent = (100.0*hist.max_bin_value) / hist.pixel_total;
   max_percent = (floor(max_percent/5.0)+1) * 5.0;

   /*--------------------------------------
   **  Graph boundaries
   */
   xmin = 6*char_wid;
   xmax = da_wid - 2*char_wid;
   ymin = da_hgt - 2*char_hgt;
   ymax = char_hgt;

   gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_GRAY]);    /* Graph is gray */
   gdk_draw_rectangle( da->window, W.nor_gc, TRUE, xmin, ymax, xmax-xmin+1, ymin-ymax+1);
   gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_WHITE]);   /* boarder is white */
   gdk_draw_rectangle( da->window, W.nor_gc, FALSE, xmin, ymax, xmax-xmin, ymin-ymax);

   gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_MAGENTA]); /* Display header  */
   strcpy( buf, "HISTOGRAM"); 
	l = strlen(buf);
   x1 = (da_wid - l*char_wid) / 2;
	draw_text(ds, U_PIXEL, x1, 0, buf);

   /*--------------------------------------
   **  label Y axis
   */
   gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_WHITE]);   /* boarder is white */
   x1 = xmin - char_wid;
   for( f = 0; f <= max_percent; f += (max_percent/4) )
   {
      y1 = map( f, 0, max_percent, ymin, ymax);
      gdk_draw_line( da->window, W.nor_gc, x1, y1, xmax, y1);

      sprintf( buf, "%1.0f%%", f); l = strlen(buf);
      x2 = xmin - ((l+1)*char_wid);
		draw_text(ds, U_PIXEL, x2, y1-char_hgt, buf);
   }

   /*--------------------------------------
   **  label X axis
   */
   /*
   **  Draw X axis.
   */
   y1 = ymin+char_hgt/2;
   y2 = y1;
   for( i=0; i<=100; i+=25 )
   {
      l = map( i, 0.0, 100.0, dp->image_min, dp->image_max );
      sprintf( buf, "%d", l); l = strlen(buf);

      x1 = map( i, 0, 100, xmin, xmax);
      gdk_draw_line( da->window, W.nor_gc, x1, ymin, x1, y1);
      x1 -= (l*char_wid)/2;
		draw_text(ds, U_PIXEL,  x1, y2, buf);
   }

   /*------------------------------------------
   **  Draw histogram
   */
   for( i=0; i < hist.num_of_bins; i++)
   {
      x1 = map( i, 0, hist.num_of_bins, xmin, xmax);
      x2 = map( i+1, 0, hist.num_of_bins, xmin, xmax);
      y2 = map( 100.0*hist.bins[i]/hist.pixel_total, 0, max_percent, ymin, ymax);

      gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_MAGENTA]);
      gdk_draw_rectangle( da->window, W.nor_gc, TRUE, x1, y2, x2-x1, ymin-y2);
      gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_BLUE]);
      gdk_draw_rectangle( da->window, W.nor_gc, FALSE, x1, y2, x2-x1, ymin-y2);
   }

   /* display pixel information */
   y1 = ymax + 1;
   gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_BLACK]);
   sprintf( buf, "Pixed Graphed = %4.1f%%", (100.0*hist.pixel_graph)/hist.pixel_total);
   l = strlen(buf);
   x1 = xmax - (l*char_wid) - 1;
	draw_text(ds, U_PIXEL, x1, y1, buf);

   sprintf( buf, "Too Low = %4.1f%%", (100.0*hist.pixel_low)/hist.pixel_total);
   l = strlen(buf);
   x1 = xmax - (l*char_wid) - 1;
	draw_text(ds, U_PIXEL, x1, y1+=char_hgt, buf);

   sprintf( buf, "Too High = %4.1f%%", (100.0*hist.pixel_high)/hist.pixel_total);
   l = strlen(buf);
   x1 = xmax - (l*char_wid) - 1;
	draw_text(ds, U_PIXEL, x1, y1+=char_hgt, buf);
}


/*----------------------------------------------------------------------
** get_linecut_info() - Calculates the histogram hist info.
**----------------------------------------------------------------------
*/
int get_linecut_info(
   struct dpy_t *dp,         /* display parmaters - Must have value fits buffer  */
   struct lcut_info *lcut    /* This function returns this linecut info */
)
{
   int i,
       x1, y1;
   float f;

   struct df_buf_t *bp;
   struct stats_t  *sp;

   bp = &Buffer[dp->bufinx];    /* buffer pointer */
   sp = &Stats[dp->bufinx];     /* stats info */

   /* get range of pixel to consider for histogram */
   lcut->xmin = 0;
   lcut->xmax = bp->naxis1-1;
   lcut->ymin = 0;
   lcut->ymax = bp->naxis2-1;
   if( dp->lcut_area )
   {
      if( (sp->objx >= 0) &&
          ((sp->objx+sp->objwid) < bp->naxis1) &&
          (sp->objwid >= 1 ))
      {
         lcut->xmin = sp->objx;
         lcut->xmax = sp->objx + sp->objwid - 1;
      }

      if( (sp->objy >= 0) &&
          ((sp->objy+sp->objhgt) < bp->naxis2) &&
          (sp->objhgt >= 1 ))
      {
         lcut->ymin = sp->objy;
         lcut->ymax = sp->objy + sp->objhgt - 1;
      }
   }

   /*
   **  Check the axis where the line cut occur
   */
   lcut->xaxis = dp->lcut_x;
   lcut->yaxis = dp->lcut_y;
   if( !INRANGE( lcut->xmin, lcut->xaxis, lcut->xmax))
      lcut->xaxis = lcut->xmin + (lcut->xmax - lcut->xmin)/2;
   if( !INRANGE( lcut->ymin, lcut->yaxis, lcut->ymax))
      lcut->yaxis = lcut->ymin + (lcut->ymax - lcut->ymin)/2;

   /*
   **  Find Min & Max Range for graph's scale
   */
   if( dp->lcut_autoscale )
   {
      lcut->range_min = DF_MAX_SIGNED_INT32;
      lcut->range_max = DF_MIN_SIGNED_INT32;

      for( y1 = lcut->ymin;  y1 <= lcut->ymax; y1++)
      {
         i = y1 * bp->naxis1 + lcut->xmin;
         for( x1 = lcut->xmin;  x1 <= lcut->xmax; x1++)
         {
            f = dfdatainx( bp, i++ );

            if( lcut->range_min  > f ) lcut->range_min  = f;
            if( lcut->range_max  < f ) lcut->range_max  = f;
         }
      }
   }
   else
   {
     lcut->range_min = dp->lcut_min;
     lcut->range_max = dp->lcut_max;
   }

#if DEBUG
   printf(" lcut->xmin  %d\n", lcut->xmin);
   printf(" lcut->xmax  %d\n", lcut->xmax);
   printf(" lcut->ymin  %d\n", lcut->ymin);
   printf(" lcut->ymax  %d\n", lcut->ymax);

   printf(" lcut->xaxis %d\n", lcut->xaxis);
   printf(" lcut->yaxis %d\n", lcut->yaxis);

   printf(" lcut->r_min %3.1f\n", lcut->range_min);
   printf(" lcut->r_max %3.1f\n", lcut->range_max);
#endif

   return ERR_NONE;
}

/*----------------------------------------------------------------------
** draw_linecut() - Display a linecut in the drawing area.
**----------------------------------------------------------------------
*/
void draw_linecut( GtkWidget *da, struct dpy_t *dp, struct df_buf_t *bp )
{
   int i, l,
       char_wid, char_hgt,
       da_wid, da_hgt,
       x1, y1, x2, y2,
       xmin, xmax, ymin, ymax,
       idata;

   float f;
   char buf[85];
   struct lcut_info lcut;        /* linecut info */

   struct ds_t ds_var;
   struct ds_t *ds;
      
   // no data? 
	if( bp->N == 0 ) 
   {
      draw_no_data( da, dp, bp);
      return;
   }

   // some basic stuff for DA's
   gdk_drawable_get_size( da->window, &da_wid, &da_hgt );
   ds = init_ds( &ds_var, da, W.nor_gc, &W.fixed_font, W.fixed_font_wid, W.fixed_font_hgt);
   char_wid = W.fixed_font_wid;
   char_hgt = W.fixed_font_hgt;

   gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_BLACK] );
   gdk_draw_rectangle( da->window, W.nor_gc, TRUE, 0, 0, da_wid, da_hgt );
   gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_YELLOW] );

   /*-----------------------------------
   ** Get linecut info
   */
   if( ERR_NONE != get_linecut_info( dp, &lcut ))
   {
      draw_text(ds, U_PIXEL, char_wid, char_hgt, "get_linecut_info(): error!");
      return;
   }

   /*--------------------------------------
   **  Graph boundaries
   */
   xmin = 7*char_wid;
   xmax = da_wid - 4*char_wid;
   ymin = da_hgt - 2*char_hgt;
   ymax = 3*char_hgt;

   gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_GRAY]);    /* Canvas is gray */
   gdk_draw_rectangle( da->window, W.nor_gc, TRUE, 0, 0, da_wid, da_hgt);
   gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_BLACK]);   /* Graph is black */
   gdk_draw_rectangle( da->window, W.nor_gc, TRUE, xmin, ymax, xmax-xmin+1, ymin-ymax+1);
   gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_WHITE]);   /* Border is white */
   gdk_draw_rectangle( da->window, W.nor_gc, FALSE, xmin, ymax, xmax-xmin, ymin-ymax);

   sprintf( buf, "LINECUT at %d, %d", lcut.xaxis, lcut.yaxis);
   l = strlen(buf);
   x1 = (da_wid - l*char_wid)/2;
	draw_text( ds, U_PIXEL, x1, 0, buf);

   /*
   **  Label Y axis.
   */
   x1 = xmin - char_wid;        /* Scale markings */
   for( i=0; i <= 100; i+=25 )
   {
      gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_WHITE]);
      y1 = map( i, 0, 100, ymin, ymax) + 0.5;
      gdk_draw_line( da->window, W.nor_gc, xmin-char_wid, y1, xmin, y1);
      gdk_draw_line( da->window, W.nor_gc, xmax+char_wid, y1, xmax, y1);
      /*
      **  Values of pixels for NAXIS1 (Red)
      */
      f = map(i, 0, 100, lcut.range_min, lcut.range_max);
      sprintf( buf, "%3.0f", f); l = strlen(buf);
      x2 = xmin - ((l+1)*char_wid);
      gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_RED]);
		draw_text( ds, U_PIXEL, x2, y1-char_hgt, buf );
      /*
      **  Index of pixel for NAXIS2 (GREEN)
      */
      idata = map( i, 0, 100, lcut.ymax, lcut.ymin)+0.5;
      sprintf( buf, "%d", idata ); l = strlen( buf );
      gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_GREEN]);
		draw_text( ds, U_PIXEL, xmax+char_wid, y1-char_hgt, buf );
   }

   /*
   **  Draw X axis.
   */
   y1 = ymax-char_hgt/2;               /* Text for top    labels    */
   y2 = ymin+(char_hgt/2)+char_hgt;    /* Text for bottom labels    */
   for( i=0; i <= 100; i+=25 )
   {
      /* Line Markings */
      gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_WHITE]);
      x1 = map( i, 0, 100, xmin, xmax);
      l = char_hgt/2;
      gdk_draw_line( da->window, W.nor_gc, x1, ymin, x1, ymin+l);
      gdk_draw_line( da->window, W.nor_gc, x1, ymax, x1, ymax-l);
      /*
      **  Indexes of pixels for NAXIS1 (Red)
      */
      idata = map( i, 0, 100, lcut.xmin, lcut.xmax)+0.5;
      sprintf( buf, "%d", idata );
      l = strlen( buf );
      x2 = x1 - ((l+1)*char_wid)/2;
      gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_RED]);
		draw_text( ds, U_PIXEL,  x2, y2-char_hgt, buf );
      /*
      **  Values of pixel for NAXIS2 (Green)
      */
      f = map( i, 0, 100, lcut.range_min, lcut.range_max);
      sprintf( buf, "%3.0f", f); l = strlen(buf);
      x2 = x1 - ((l+1)*char_wid)/2;
      gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_GREEN]);
		draw_text( ds, U_PIXEL, x2, y1-char_hgt, buf );
   }
   /*
   ** Draw Profile for NAXIS1 (Red).
   */
   gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_RED]);
   x2 = xmin;
   y2 = ymin;

   for( i = lcut.xmin;  i <= lcut.xmax; i++)
   {
      x1 = map( i+0.5, lcut.xmin, lcut.xmax, xmin, xmax);

      f = dfdataxy( bp, i, lcut.yaxis);

      if( f > lcut.range_max )
         y1 = ymax;
      else if( f < lcut.range_min )
         y1 = ymin;
      else
         y1 = map( f, lcut.range_min, lcut.range_max, ymin, ymax);

      gdk_draw_line( da->window, W.nor_gc, x2, y2, x2, y1);
      gdk_draw_line( da->window, W.nor_gc, x2, y1, x1, y1);

      x2 = x1;
      y2 = y1;
   }
   /*
   ** Draw Profile for NAXIS2 (Green).
   */
   gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_GREEN]);
   x2 = xmin;
   y2 = ymax;

   for( i = lcut.ymin;  i <= lcut.ymax; i++)
   {
      //y1 = map( i+0.5, lcut.ymin, lcut.ymax, ymin, ymax);
      y1 = map( i+0.5, lcut.ymin, lcut.ymax, ymax, ymin);

      f = dfdataxy( bp, lcut.xaxis, i);

      if( f > lcut.range_max )
         x1 = xmax;
      else if( f < lcut.range_min )
         x1 = xmin;
      else
         x1 = map( f, lcut.range_min, lcut.range_max, xmin, xmax);

      gdk_draw_line( da->window, W.nor_gc, x2, y2, x1, y2);
      gdk_draw_line( da->window, W.nor_gc, x1, y2, x1, y1);

      x2 = x1;
      y2 = y1;
   }
}


/*----------------------------------------------------------------------
** draw_noise() - Display a linecut in the drawing area.            hack0
**----------------------------------------------------------------------
*/
void draw_noise( GtkWidget *da, struct dpy_t *dp, struct df_buf_t *bp )
{
   int    x1, y1, y2,
          xmin, xmax,          /* Graph area min, max, & size in pixels */
          ymin, ymax,
          char_wid,
          char_hgt,
          da_wid, da_hgt,
          mod,
          nrows,
          l,
          row, col;
   float  range_min, range_max;
   char   buf[85];

   struct noise_t *noisep;
   struct noise_t fnoise;

   struct ds_t ds_var;
   struct ds_t *ds;
      
   // no data? 
	if( bp->N == 0 ) 
   {
      draw_no_data( da, dp, bp);
      return;
   }

   // some basic stuff for DA's
   gdk_drawable_get_size( da->window, &da_wid, &da_hgt );
   ds = init_ds( &ds_var, da, W.nor_gc, &W.fixed_font, W.fixed_font_wid, W.fixed_font_hgt);
   char_wid = W.fixed_font_wid;
   char_hgt = W.fixed_font_hgt;

   /*
   **  Allocate memory for data.
   */
   if( NULL == ( noisep = (struct noise_t *) calloc( dp->noise_mod, sizeof(struct noise_t)) ) )
   {
      printf("Memory allocation error\n");
      return;
   }
   memset( (char*) &fnoise, 0x00, sizeof(fnoise));

   /*
   **  Calculate noise
   */
   calc_noise( noisep, &fnoise, dp, bp );

   gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_BLACK]);
   gdk_draw_rectangle( da->window, W.nor_gc, TRUE, 0, 0, da_wid, da_hgt);

   /*--------------------------------------------------------------
   ** Display noise as text.
   */
   row = dp->noise_row;
   col = dp->noise_col;

   if ( !dp->noise_graph_type )
   {

      gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_GREEN]);
      sprintf(buf, "Min       Max       Mean      STD       N");
      draw_text( ds, U_PIXEL, 8*char_wid,    0, buf+col);

      draw_text( ds, U_PIXEL, 0,  char_hgt, "Array");

      gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_YELLOW]);
      sprintf(buf, "%7.1f   %7.1f   %8.2f  %7.2f   %d",
                    fnoise.min, fnoise.max, fnoise.mean, fnoise.std, fnoise.N);
      draw_text( ds, U_PIXEL,  8*char_wid,  char_hgt, buf+col);

      gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_CYAN]);
      for( mod=0; (mod+row < dp->noise_mod) && (mod < da_hgt/char_hgt-1); mod++)
      {
         gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_GREEN]);
         sprintf( buf, "Col %2d", mod+row);
			draw_text( ds, U_PIXEL,   0,  (2+mod)*char_hgt, buf );

         gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_CYAN]);
         sprintf(buf, "%7.1f   %7.1f   %8.2f  %7.2f   %d",
                      noisep[mod+row].min, noisep[mod+row].max, noisep[mod+row].mean,
                      noisep[mod+row].std, noisep[mod+row].N);
			draw_text( ds, U_PIXEL,   8*char_wid,  (2+mod)*char_hgt, buf+col);
      }
   }

   /*----------------------------------------------------------
   **  Draw Noise Graph.
   */
   if ( dp->noise_graph_type )
   {

      ymax = char_hgt;
      if( (da_hgt - ymax) < 60 )
              return;
      ymin = ymax + ((da_hgt-ymax)/2);
      xmin = 10*char_wid;
      xmax = da_wid - 2*char_wid;

      gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_GRAY]);
      gdk_draw_rectangle( da->window, W.nor_gc, FALSE, xmin, ymax, xmax-xmin, ymin-ymax );

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
      gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_WHITE]);
      sprintf( buf, "%3.1f", range_min); l = strlen(buf);
      x1 = xmin - ((l+1)*char_wid);
		draw_text( ds, U_PIXEL,   x1, ymin-char_hgt, buf );
      sprintf( buf, "%3.1f", range_max); l = strlen(buf);
      x1 = xmin - ((l+1)*char_wid);
		draw_text( ds, U_PIXEL,    x1, ymax, buf);

      /* Draw array stats */
      gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_YELLOW]);
      x1 = map( 1, 0, nrows, xmin, xmax);
      y1 = map( fnoise.min, range_min, range_max, ymin, ymax);
      y2 = map( fnoise.max, range_min, range_max, ymin, ymax);
      gdk_draw_line( da->window, W.nor_gc, x1, y1, x1, y2);
      gdk_draw_line( da->window, W.nor_gc, x1-2, y1, x1+2, y1);
      gdk_draw_line( da->window, W.nor_gc, x1-2, y2, x1+2, y2);
      y1 = map( fnoise.mean, range_min, range_max, ymin, ymax);
      gdk_draw_rectangle( da->window, W.nor_gc, FALSE, x1-1, y1-1, 3, 3);

      /* Draw stats for each column */
      gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_CYAN]);
      for( mod=0; mod < dp->noise_mod; mod++)
      {
          x1 = map( mod+2, 0, nrows, xmin, xmax);
          y1 = map( noisep[mod].min, range_min, range_max, ymin, ymax);
          y2 = map( noisep[mod].max, range_min, range_max, ymin, ymax);
          gdk_draw_line( da->window, W.nor_gc, x1, y1, x1, y2);
          gdk_draw_line( da->window, W.nor_gc, x1-2, y1, x1+2, y1);
          gdk_draw_line( da->window, W.nor_gc, x1-2, y2, x1+2, y2);
          y1 = map( noisep[mod].mean, range_min, range_max, ymin, ymax);
          gdk_draw_rectangle( da->window, W.nor_gc, FALSE, x1-1, y1-1, 3, 3);
      }

      /*
      **  Draw std graph.
      */
      ymax = ymin + char_hgt;
      ymin = da_hgt - char_hgt;

      gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_GRAY]);
      gdk_draw_rectangle( da->window, W.nor_gc, FALSE, xmin, ymax, xmax-xmin, ymin-ymax );

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

      /* label Y axis */
      gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_WHITE]);
      sprintf( buf, "%3.1f", range_min); l = strlen(buf);
      x1 = xmin - ((l+1)*char_wid);
		draw_text( ds, U_PIXEL, x1, ymin-char_hgt, buf );
      sprintf( buf, "%3.1f", range_max); l = strlen(buf);
      x1 = xmin - ((l+1)*char_wid);
		draw_text( ds, U_PIXEL, x1, ymax, buf );

      /* Draw array std */
      gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_YELLOW]);
      x1 = map( 1, 0, nrows, xmin, xmax);
      y1 = map( fnoise.std, range_min, range_max, ymin, ymax);
      gdk_draw_point( da->window, W.nor_gc, x1, y1 );
      gdk_draw_rectangle( da->window, W.nor_gc, FALSE, x1-2, y1-2, 5, 5);

      gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_CYAN]);
      for( mod=0; mod < dp->noise_mod; mod++)
      {
         x1 = map( mod+2, 0, nrows, xmin, xmax);
         y1 = map( noisep[mod].std, range_min, range_max, ymin, ymax);
         gdk_draw_point( da->window, W.nor_gc, x1, y1 );
         gdk_draw_rectangle( da->window, W.nor_gc, FALSE, x1-2, y1-2, 5, 5);

      }
   }

   /* free memory  */
   free( (char*) noisep);
}

/*----------------------------------------------------------------------
** calc_noise() - Perform noise calculations.                      hack4
**----------------------------------------------------------------------
*/
void calc_noise( struct noise_t *noisep, struct noise_t *fnoise,
                 struct dpy_t *dp, struct df_buf_t *bp )
{
   int i,
       mod,
       x1, y1,
       x_beg, x_numpixel,
       y_beg, y_numpixel;
   double ddata, d;

   /*
   **  Get pixel domain
   */
   if( dp->noise_area )
   {
      i = dp->bufinx;
      x_beg = Stats[i].objx;
      x_numpixel = MIN(Stats[i].objwid, bp->naxis1-x_beg);
      x_numpixel = MAX(1, x_numpixel);

      y_beg = Stats[i].objy;
      y_numpixel = MIN(Stats[i].objhgt, bp->naxis2-y_beg);
      y_numpixel = MAX(1, y_numpixel);
   }
   else
   {
      x_beg = 0;
      x_numpixel = bp->naxis1;
      y_beg = 0;
      y_numpixel = bp->naxis2;
   }
   /*
   ** loop through data & calculate stats
   */
   fnoise->min = DF_MAX_SIGNED_INT32;
   fnoise->max = DF_MIN_SIGNED_INT32;

   for( mod=0; mod < dp->noise_mod; mod++)
   {
      noisep[mod].min = DF_MAX_SIGNED_INT32;
      noisep[mod].max = DF_MIN_SIGNED_INT32;
   }

        /* PASS 1 */
   for( y1 = y_beg; y1 < y_beg+y_numpixel; y1++)
   {
      i = y1 * bp->naxis1 + x_beg;
      for( x1 = x_beg; x1 < x_beg+x_numpixel; x1++)
      {
         ddata = dfdatainx( bp, i);

			if( ddata > fnoise->max ) fnoise->max = ddata;
			if( ddata < fnoise->min ) fnoise->min = ddata;
			fnoise->mean += ddata;
			fnoise->N++;

			// mod or channel mode?
			if( dp->noise_mode==0 )
				mod = i % dp->noise_mod;
			else
				mod = x1 / dp->noise_size;
			if( ddata > noisep[mod].max ) noisep[mod].max = ddata;
			if( ddata < noisep[mod].min ) noisep[mod].min = ddata;
			noisep[mod].mean += ddata;
			noisep[mod].N++;

          i++;
       }
    }

    /* calculate means */
    if( fnoise->N > 0 ) fnoise->mean /= fnoise->N;
    for( mod=0; mod < dp->noise_mod; mod++)
       if( noisep[mod].N > 0 ) noisep[mod].mean /= noisep[mod].N;


   /* PASS 2 */
   for( y1 = y_beg; y1 < y_beg+y_numpixel; y1++)
   {
      i = y1 * bp->naxis1 + x_beg;
      for( x1 = x_beg; x1 < x_beg+x_numpixel; x1++)
      {
         ddata = dfdatainx( bp, i );

			d = ddata - fnoise->mean;
			fnoise->std += d*d;

			// mod or channel mode?
			if( dp->noise_mode==0 )
				mod = i % dp->noise_mod;
			else
				mod = x1 / dp->noise_size;
			d = ddata - noisep[mod].mean;
			noisep[mod].std += d*d;
         i++;
      }
   }

   if( fnoise->N > 1 ) fnoise->std = sqrt(fnoise->std / (fnoise->N-1));
   for( mod=0; mod < dp->noise_mod; mod++)
      if( noisep[mod].N > 1 ) noisep[mod].std = sqrt(noisep[mod].std / (noisep[mod].N-1));

}


/*----------------------------------------------------------------------
** draw_xcut() - Display a xcut graph.                              hack1
**----------------------------------------------------------------------
*/
void draw_xcut( GtkWidget *da, struct dpy_t *dp, struct df_buf_t *bp )
{
   int    x1, y1, x2, y2,
          xmin, xmax,          /* Graph area min, max, & size in pixels */
          ymin, ymax,
          char_wid,
          char_hgt,
          da_wid, da_hgt,
          num_xcut_items,
          inx,
          l, i;
   float  fdata,
          range_min, range_max;
   char buf[85];

   struct xcut_buf_t *xcut_buf;

   struct ds_t ds_var;
   struct ds_t *ds;
      
   // no data? 
	if( bp->N == 0 ) 
   {
      draw_no_data( da, dp, bp);
      return;
   }

   // some basic stuff for DA's
   gdk_drawable_get_size( da->window, &da_wid, &da_hgt );
   ds = init_ds( &ds_var, da, W.nor_gc, &W.fixed_font, W.fixed_font_wid, W.fixed_font_hgt);
   char_wid = W.fixed_font_wid;
   char_hgt = W.fixed_font_hgt;

   /*
   **  allocate array for data
   */
   if( NULL == ( xcut_buf = (struct xcut_buf_t *) malloc( sizeof(struct xcut_buf_t)*NUM_PIXEL)) )
   {
      printf("Memory allocation error\n");
      return;
   }
   /*
   **  Using a digital differential analyser (DDA) algorithm, cycle through the data
   **  and place points in buffer, the min and max.
   */
   num_xcut_items = xcut_line ( dp->xcut_xbeg, dp->xcut_ybeg,
                                     dp->xcut_xend, dp->xcut_yend,
                                     bp, xcut_buf, NUM_PIXEL-1 );

   /* Get Y scale  */

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
      dp->xcut_min = range_min;
      dp->xcut_max = range_max;
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
   xmin =  7*char_wid;
   xmax =  da_wid - 2*char_wid;
   ymin =  da_hgt - 3*char_hgt;
   ymax =  char_hgt*4;
   gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_BLACK]);    /* Canvas is black */
   gdk_draw_rectangle( da->window, W.nor_gc, TRUE, 0, 0, da_wid, da_hgt);
   /***
   gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_GRAY]);
   gdk_draw_rectangle( da->window, W.nor_gc, TRUE, xmin, ymax, xmax-xmin+1, ymin-ymax+1 );
   ****/
   gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_GREEN]);
   gdk_draw_rectangle( da->window, W.nor_gc, FALSE, xmin, ymax, xmax-xmin,  ymin-ymax);

   gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_WHITE]);
   sprintf( buf, "XCUT (%d,%d) to (%d, %d)", dp->xcut_xbeg, dp->xcut_ybeg,
                                             dp->xcut_xend, dp->xcut_yend);

   l = strlen(buf);
   x1 = (da_wid - l*char_wid)/2;
   draw_text( ds, U_PIXEL, x1, 0, buf);
   /*
   **  Label Y axis.
   */
   x1 = xmin - char_wid;        /* Scale markings */
   for( i=0; i <= 100; i+=25 )
   {
      y1 = rint( map( i, 0.0, 100.0, ymin, ymax));
      gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_GREEN]);
      gdk_draw_line( da->window, W.nor_gc, x1, y1, xmin, y1);
      /*
      **  Values of pixels for NAXIS1 (Red)
      */
      gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_WHITE]);
      fdata = map( i, 0.0, 100.0, range_min, range_max);
      double2str_len( buf, fdata, 6, 2); l = strlen(buf);
      x2 = xmin - ((l+1)*char_wid);
		draw_text( ds, U_PIXEL, x2, y1-char_hgt, buf );
   }
   /*
   **  Draw X axis.
   */
   y1 = ymin+char_hgt/2;       /* line markings */
   y2 = y1 + char_hgt;         /* Text          */
   for( i=0; i <= 100; i+=25 )
   {
      x1 = map( (float)i, (float)0.0, (float)100.0, (float)xmin, (float)xmax);
      gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_GREEN]);
      gdk_draw_line( da->window, W.nor_gc, x1, ymin, x1, y1);

      inx = rint( map( i, 0.0, 100.0, 0, num_xcut_items-1) );

      gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_CYAN]);
      sprintf( buf, "%d", xcut_buf[inx].x); l = strlen( buf );
      x2 = x1 - ((l+1)*char_wid)/2;
		draw_text( ds, U_PIXEL, x2, y1, buf );

      gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_WHITE]);
      sprintf( buf, "%d", xcut_buf[inx].y); l = strlen( buf );
      x2 = x1 - ((l+1)*char_wid)/2;
		draw_text( ds, U_PIXEL, x2, y2, buf );
   }
   /*
   **  Draw the graph
   */
   gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_RED]);

   x2 = xmin;
   y2 = ymin;
   for( i=0; i<num_xcut_items; i++)
   {
      x1 = map( i+1, 0, num_xcut_items, xmin, xmax);
		y1 = map( xcut_buf[i].value, range_min, range_max, ymin, ymax);

      gdk_draw_line( da->window, W.nor_gc, x2, y2, x2, y1);
      gdk_draw_line( da->window, W.nor_gc, x2, y1, x1, y1);

      x2 = x1;
      y2 = y1;
   }
   gdk_draw_line( da->window, W.nor_gc, x2, y2, xmax, ymin);

   /*---------------------------------------------
   ** option to output vfgfit.dat to gfit program.
   */
#if USE_GSLFIT
   if( Dpy[Lc.active].xcut_fit_data && (num_xcut_items > 2) )
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
         cc_printf( W.cmd.main_console, CM_BLUE, "fitgsl_lm() returned non-zero status\n" );

      /*--------------------------
      ** output text information
      */
      gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_CYAN]);

      x1 = 0;
      sprintf( buf, "Base=%5.2f ", R[FITGSL_B_INDEX]); l = strlen(buf);
		draw_text( ds, U_PIXEL, x1, char_hgt, buf );
      sprintf( buf, " Cen=%5.2f ", R[FITGSL_C_INDEX]); l = strlen(buf);
		draw_text( ds, U_PIXEL, x1, (2*char_hgt), buf );

      x1 = 13*char_wid;
      sprintf( buf, "Peak=%5.2f ", R[FITGSL_P_INDEX]); l = strlen(buf);
		draw_text( ds, U_PIXEL, x1, char_hgt, buf );
      sprintf( buf, " Wid=%5.2f ", R[FITGSL_W_INDEX]); l = strlen(buf);
		draw_text( ds, U_PIXEL, x1, 2*char_hgt, buf );

      x1 = 28*char_wid;
      sprintf( buf, "FWHM=%4.2f ", 2.35482*R[FITGSL_W_INDEX]); l = strlen(buf);
		draw_text( ds, U_PIXEL, x1, char_hgt, buf );
      sprintf( buf, "Seeing=%4.2f ", 2.35482*R[FITGSL_W_INDEX]*bp->arcsec_pixel); l = strlen(buf);
		draw_text( ds, U_PIXEL, x1, (2*char_hgt), buf );
      sprintf( buf, "Area=%4.2f ", 
         sqrt(2*M_PI) * R[FITGSL_P_INDEX] * R[FITGSL_W_INDEX]); l = strlen(buf);
		draw_text( ds, U_PIXEL, x1, (3*char_hgt), buf );

      if( use_x_axis )
			gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_CYAN]);
      else
			gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_WHITE]);
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

         gdk_draw_line( da->window, W.nor_gc, x1, y1, x2, y2);
      }
      Dpy[Lc.active].xcut_fit_data = FALSE;

      /* Free allocated data */
      fitgsl_free_data(dat);
   }
#endif // USE_GSLFIT

   /*---------------------------------------------
   ** option to output vfgfit.dat to gfit program.
   */
   if( Dpy[Lc.active].xcut_output_data )
   {
      FILE *fp;
      if( (fp = fopen( "/tmp/xgfit.dat", "w")) != NULL )
      {
         fchmod( fileno(fp), S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);

         fprintf( fp, " %d \n", num_xcut_items);
         for( i=0; i<num_xcut_items; i++)
            fprintf( fp, " %d %2.0f\n", i, xcut_buf[i].value);
         fclose( fp );
      }
      Dpy[Lc.active].xcut_output_data = FALSE;
   }

   free( (char *) xcut_buf);
}


/*----------------------------------------------------------------
** xcut_line - This copies the data from the buffer to destbuf.
**   It uses the digital differential analyzer (DDA) algorithm to
**   select a line of pixels from (x0, y0) to (x1, y1).
**   It returns count, the number of pixels found.
**----------------------------------------------------------------
*/
int xcut_line ( int x0, int y0, int x1, int y1,
                struct df_buf_t *bp,
                struct xcut_buf_t *destbuf,
                int num_destbuf )
{
   int x, y,
       reverse,
       count,
       i, is, ie, di;
   float dy, dx,
         dj, j;

   count = 0;
   dy = y1 - y0;
   dx = x1 - x0;

   if( fabs(dy) > fabs(dx) || dx==0 )
   {
      is = y0;
      ie = y1;
      di = ( dy > 0 ? 1 : -1);
      j  = x0;
      dj = dx/fabs(dy);
      reverse = 1;
   }
   else
   {
      is = x0;
      ie = x1;
      di = ( dx > 0 ? 1 : -1);
      j  = y0;
      dj = dy/fabs(dx);
      reverse = 0;
   }

   for( i=is; i!=ie+di; i += di)
   {
      x = i;
      y = j+0.5;
      if( reverse )
         { x = y; y = i; }

      if( INRANGE( 0, x, bp->naxis1-1) && INRANGE( 0, y, bp->naxis2-1))
      {
         destbuf->x = x;
         destbuf->y = y;
         destbuf->value = dfdataxy( bp, x, y);

         destbuf++;
         count++;
      }

      j += dj;
   }

   return count;
}


/*------------------------------------------------------
**  draw_pointer()
**------------------------------------------------------
*/

void draw_pointer( GtkWidget *da, struct dpy_t *dp, struct df_buf_t *bp )
{
   char * cptr;
   int  x, y, l,
        char_wid, char_hgt,
        da_wid, da_hgt;

   struct ds_t ds_var;
   struct ds_t *ds;
      
   // some basic stuff for DA's
   gdk_drawable_get_size( da->window, &da_wid, &da_hgt );
   ds = init_ds( &ds_var, da, W.nor_gc, &W.fixed_font, W.fixed_font_wid, W.fixed_font_hgt);
   char_wid = W.fixed_font_wid;
   char_hgt = W.fixed_font_hgt;

   gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_BLACK]);
   gdk_draw_rectangle( da->window, W.nor_gc, TRUE, 0, 0, da_wid, da_hgt);
   gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_GREEN]);

   y = da_hgt / 2;

   cptr = "Pointer Display";
   l = strlen(cptr);
   x = (da_wid - (char_wid*l))/2;
   draw_text( ds, U_PIXEL, x, y, cptr );

   cptr = "Move pointer to image to begin";
   l = strlen(cptr);
   x = (da_wid - (char_wid*l))/2;
   draw_text( ds, U_PIXEL, x, y+char_hgt, cptr );
}

/*------------------------------------------------------
**  draw_pointer_update()
**------------------------------------------------------
*/

void draw_pointer_update( GtkWidget *da, int dpinx, int ax, int ay, int image_dpinx )
{
   char buf[120];
   int  x, x_beg, x_end,
        y, y_beg, y_end,
        char_wid, char_hgt,
        da_wid, da_hgt,
        wid_pix,
        i_x, i_y, i_size;
   float data;

   struct dpy_t    *dp = &Dpy[dpinx];
   struct dpy_t    *imagedp = &Dpy[image_dpinx];
   struct df_buf_t *bp = &Buffer[imagedp->bufinx];

   /* draw only if (x,y) refers to a pixel */
   if( !INRANGE(0, ax, bp->naxis1-1)) return;
   if( !INRANGE(0, ay, bp->naxis2-1)) return;

   struct ds_t ds_var;
   struct ds_t *ds;
      
   // some basic stuff for DA's
   gdk_drawable_get_size( da->window, &da_wid, &da_hgt );
   ds = init_ds( &ds_var, da, W.nor_gc, &W.fixed_font, W.fixed_font_wid, W.fixed_font_hgt);
   char_wid = W.fixed_font_wid;
   char_hgt = W.fixed_font_hgt;

   gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_BLACK]);
   gdk_draw_rectangle( da->window, W.nor_gc, TRUE, 0, 0, da_wid, da_hgt);
   gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_YELLOW]);

   /*---------------------------------------
   ** Define location for image; and draw box/tick marks.
   */
	i_x = char_wid;  // image located at upper-left
	i_y = char_hgt;
	if( dp->pt_show_linecut )
		wid_pix = ((da_wid/2) - 2*char_wid)/dp->pt_image_size;  // size = 1/2 of da_wid
	else if( dp->pt_show_stats )
		wid_pix = (da_wid - (15*char_wid))/dp->pt_image_size;   // size = leave txt room for stats
	else
		wid_pix = (da_wid - (2*char_wid))/dp->pt_image_size;    // size = use entire window

	i_size = wid_pix * dp->pt_image_size;
	gdk_draw_rectangle( da->window, W.nor_gc, FALSE, i_x, i_y, i_size+1, i_size+1);

	/* Draw tick marks */
	x = i_x + i_size/2;
	gdk_draw_line( da->window, W.nor_gc, x, i_y, x, i_y-char_wid/2);

	y = i_y + i_size/2;
	x = i_x + i_size/2;

	/* display top of window */
	draw_text( ds, U_PIXEL, char_wid, 0, "XY=%d,%d ", ax, ay );

	data = dfdataxy( bp, ax, ay);      /* get value of center pixel */
	sprintf( buf, " %7.1f 0x%08lx ", data, (long)data);
	x = da_wid - char_wid*strlen(buf);
	draw_text( ds, U_PIXEL, x, 0, buf);

   /*---------------------------------------
   ** Put image in pixel box.
   */
   {
      int color,
          bitmap_pad,
          i;
      float block;
      GdkImage *image;
      int32_t       * tc_buf;
      unsigned char * pc_buf;

      /*---------------------
      ** create Image
      */
      i_size = wid_pix * dp->pt_image_size;
      if( (image = gdk_image_new( GDK_IMAGE_NORMAL, CM.visual, i_size, i_size )) == NULL )
         return;

      /* pseudoColor & TrueColor are supported by using 2 different pointers:   */
      tc_buf = (int32_t*) image->mem;       /* int32_t * tc_buf for TrueColor. */
      pc_buf = (unsigned char*) image->mem; /* uchar   * pc_buf for pseudoColor. */

      block = (imagedp->image_max - imagedp->image_min) / ((float)CM_NUM_RW_COLORS);

      /* calculate the amount of padding each line has in image->mem */
      i = (CM.visual->type == GDK_VISUAL_PSEUDO_COLOR ? sizeof(char) : sizeof(int32_t));
      i *= i_size;             /* size of ximage line in bytes */
      i = i % sizeof(int32_t); /* mod (wid,32 bit) */
      bitmap_pad = (i ? sizeof(int32_t)-i : 0 );

      /*---------------------------------------------------
      ** Loop through data & build display in Image buffer.
      ** Also get max, min, mean;
      */
      y_beg = ay - dp->pt_image_size/2;
      y_end = y_beg + dp->pt_image_size - 1;
      x_beg = ax - dp->pt_image_size/2;
      x_end = x_beg + dp->pt_image_size - 1;
      for( y = y_beg; y <= y_end; y++ )
      {
         for( x = x_beg; x <= x_end; x++ )
         {
            color = CM_BLACK;

            if( INRANGE( 0, x, bp->naxis1-1)  && INRANGE( 0, y, bp->naxis2-1) )
            {
               data = dfdataxy( bp, x, y );
               if( data >= imagedp->image_max )
                  color = CM_NUM_COLORS-1;  /* last color */
               else if (data <= imagedp->image_min )
                  color =  CM_NUM_STATIC_COLORS; /* first color */
               else
                  color = ( (data - imagedp->image_min) / block) + CM_NUM_STATIC_COLORS;
            }

            if( CM.visual->type == GDK_VISUAL_PSEUDO_COLOR )
            {
               memset( (char*)pc_buf, CM.colors[color].pixel, wid_pix);
               pc_buf += wid_pix;
            }
            else
            {
               i32_set( (uint32_t*)tc_buf, 
                        cm_swap_int32( CM.colors[color].pixel, CM.swap_rgb), 
                        wid_pix );
               tc_buf += wid_pix;
            }
         }
         if( bitmap_pad ) pc_buf += bitmap_pad;

         if( CM.visual->type == GDK_VISUAL_PSEUDO_COLOR )
         {
            int size = i_size + bitmap_pad;  /* add padding */
            for( i=1; i < wid_pix; i++ )
            {
               memcpy( pc_buf, (pc_buf - size), size);
               pc_buf += i_size;
               pc_buf += bitmap_pad;
            }
         }
         else
         {
            for( i=1; i < wid_pix; i++ )
            {
               memcpy( tc_buf, (tc_buf - i_size), i_size*sizeof(int32_t));
               tc_buf += i_size;
            }
         }
      }

      /*---------------------------------------------------
      ** Place image into drawing area, and free Image.
      */
      gdk_draw_image( da->window, W.nor_gc, image, 0, 0, i_x+1, i_y+1, i_size, i_size);
      //gdk_image_destroy( image );
		g_object_unref( image );


      // red box for for center pixel
		x = i_x + (ax-x_beg)*wid_pix;
		y = i_y + (ay-y_beg)*wid_pix;
      gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_RED]);
	   gdk_draw_rectangle( da->window, W.nor_gc, FALSE, 
		                    x-1, y-1, wid_pix+1, wid_pix+1); /* 1 x 1 */
   }

   /*-----------------------------------------------
   ** calculate and display statistics.
   */
	if( dp->pt_show_stats )
   {
      double N, min, max, sum, mean, std;

      cal_box_stats_subf( bp, 1, x_beg, y_beg, dp->pt_image_size,
         dp->pt_image_size, 0, 0, &N, &min, &max, &sum, &mean, &std);

      gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_GREEN]);
      x = i_size + 2*char_wid;
      y = 0*char_hgt;
      sprintf(buf, "   N %5.0f ", N); 
      draw_text( ds, U_PIXEL,   x, y+=char_hgt, buf );
      sprintf(buf, " Min %7.1f ", min); 
      draw_text( ds, U_PIXEL,   x, y+=char_hgt, buf );
      sprintf(buf, " Max %7.1f ", max); 
      draw_text( ds, U_PIXEL,   x, y+=char_hgt, buf );
      sprintf(buf, "Mean %7.1f ", mean); 
      draw_text( ds, U_PIXEL,   x, y+=char_hgt, buf );
      sprintf(buf, " STD %8.2f ", std); 
      draw_text( ds, U_PIXEL,   x, y+=char_hgt, buf );
   }

   /*-----------------------------------------------
   ** Do spex Signal to Noise Est
   */
	if( dp->pt_show_spex_sn )
	{
	   int    is_bufc, err,
		      sn_x, sn_y, sn_wid;
		double sn_ratio;
	   
	   // dbuf C is A-B beam image. This is where the signal data is.
		// dbuf B is a B beam image. This is where the background 

	   is_bufc = (imagedp->bufinx==2); // is the pointer in buffer C 
      err = cal_spex_sn(  
		   buf,                // Text to explain error 
			&sn_ratio,          // resulting SN, ratio (if no error)
			ax, ay,             // pointer x, y
			&sn_x, &sn_y,       // SN X, Y  box (upper left)
			&sn_wid,            // slit width in pixels
		   is_bufc,            // is the pointer in buffer C.
		   bp,                 // Buffer C.( A-B Image)
			&Buffer[1]);        // Buffer B (   B Image)

      if( 0 ) printf("cal_spex_sn(): %d [%20.20s] SN=%.3f sn_XY=%d,%d sn_W=%d \n", 
		   err, buf, sn_ratio, sn_x, sn_y, sn_wid);

      // Error?, display message 
      x = i_size + 2*char_wid;
      y = 6*char_hgt;
      gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_YELLOW]);
		if( err )
		{
		   draw_text( ds, U_PIXEL, x, y+=char_hgt, "S/N not calculated ");
		   draw_text( ds, U_PIXEL, x, y+=char_hgt, buf );
		}
		else
		{
		   draw_text( ds, U_PIXEL, x, y+=char_hgt, "Spex S/N: %.1f ", sn_ratio);

			// Draw a box representing the S/N box
			x = i_x + (sn_x-x_beg)*wid_pix;
			y = i_y + (sn_y-y_beg)*wid_pix;
			gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_YELLOW]);
			gdk_draw_rectangle( da->window, W.nor_gc, FALSE, 
									  x-1, y-1, (sn_wid*wid_pix)+1, (28*wid_pix)+1); /* SN box is hgt is 28 */

			y = i_y + (sn_y+7-y_beg)*wid_pix; //  7 pixel on top of SN Box
			gdk_draw_line( da->window, W.nor_gc, x-1, y-1, x+(sn_wid*wid_pix), y-1);

			y = i_y + (sn_y+21-y_beg)*wid_pix; // 10 pixel on botton of SN Box
			gdk_draw_line( da->window, W.nor_gc, x-1, y-1, x+(sn_wid*wid_pix), y-1);
		}
	}
	

   /*-----------------------------------------------------------
   **  XY line cut.
   */
	if( dp->pt_show_linecut )
   {
      int xmin, xmax, ymin, ymax,
          ox, oy,
          nx, ny;

      xmin = char_wid;
      xmax = xmin + i_size;
      ymax = da_hgt/2 + char_hgt;
      ymin = ymax + i_size;
      gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_YELLOW]);
      gdk_draw_rectangle( da->window, W.nor_gc, FALSE, xmin, ymax, i_size, i_size);

      /* X line cut */
      gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_RED]);
      ox = xmin;
      oy = ymin;
      x_beg = ax - dp->pt_image_size/2;
      x_end = x_beg + dp->pt_image_size - 1;
      for( x = x_beg; x <= x_end; x++ )
      {
         nx = ox + wid_pix;

         if( INRANGE( 0, x, bp->naxis1-1)  && INRANGE( 0, ay, bp->naxis2-1))
         {
            data = dfdataxy( bp, x, ay );

            ny = map( data, imagedp->image_min, imagedp->image_max, ymin, ymax);
            gdk_draw_line( da->window, W.nor_gc, ox, oy, ox, ny);
            gdk_draw_line( da->window, W.nor_gc, ox, ny, nx, ny);
            oy = ny;
         }

         ox = nx;
      }

      /* Y line cut */
      gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_GREEN]);
      ox = xmin;
      oy = ymax;
      y_beg = ay - dp->pt_image_size/2;
      y_end = y_beg + dp->pt_image_size - 1;
      for( y = y_beg; y <= y_end; y++ )
      {
         ny = oy + wid_pix;

         if( INRANGE( 0, ax, bp->naxis1-1)  && INRANGE( 0, y, bp->naxis2-1))
         {
            data = dfdataxy( bp, ax, y );

            nx = map( data, imagedp->image_min, imagedp->image_max, xmin, xmax);
            gdk_draw_line( da->window, W.nor_gc, ox, oy, nx, oy);
            gdk_draw_line( da->window, W.nor_gc, nx, oy, nx, ny);
            ox = nx;
         }

         oy = ny;
      }

      /* some text legends */
		// Line1: "Linecut X Y"
      x = (da_wid/2) + (1*char_wid);
      y = da_hgt - (3*char_hgt);

      strcpy(buf, "Line Cut" ); 
      gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_YELLOW]);
      draw_text( ds, U_PIXEL,  x, y, buf );

      gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_RED]);
      sprintf(buf, "%4d ", ax );
      draw_text( ds, U_PIXEL,  x+(9*char_wid), y, buf );

      sprintf(buf, "%4d ", ay); 
      gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_GREEN]);
      draw_text( ds, U_PIXEL,  x+(15*char_wid), y, buf );

		// Line2: "Range XXX XXX"
		y+=char_hgt;
      sprintf(buf, "Range %5.0f %5.0f", imagedp->image_min, imagedp->image_max); 
      gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_YELLOW]);
      draw_text( ds, U_PIXEL,  x, y, buf );
   }

}

/*----------------------------------------------------------------------------
**  cal_spex_sn() - Estimate SN values for Spex.
**     return ERR_NONE if sn_ration is OK, ERR_FAILED otherwise.
**----------------------------------------------------------------------------
*/
int cal_spex_sn(  
	char * reply,                  // Text to explain error 
	double *r_sn_ratio,            // resulting SN, ratio (if no error)
	int ax,  int ay,               // pointer x, y
	int *r_sn_x, int *r_sn_y,      // upper left of SN Box X and Y
	int *r_wid,                    // Slit Wid in pixels
	int    is_bufC,                // is the pointer in buffer C.
	struct df_buf_t *bufc,         // Buffer C.( A-B Image)
	struct df_buf_t *bufb          // Buffer B (   B Image)
)
{
   int rc, 
	    sn_x, sn_y,
	    i, 
		 wid ; 

	char buf[128];
	char *str_ptr;
	struct df_fheader_t * f_hdr;

   double N, min, max, sum, mean, std;
	double c1, c2, c3;
	double b1;
	double x, d, sn, gain;

   // defaults
   rc = 0;
   strcpy( reply, "OK" );
	*r_sn_ratio = 0;
	*r_wid = wid = 6;
	*r_sn_x = sn_x = ax - (wid/2);
	*r_sn_y = sn_y = ay - (28/2);      // SN box hgt is 28 (7+14+7)

   // should be in buffer C.
	if( !is_bufC )
	{
      strcpy( reply, "Not in the BufC." );
	   return ERR_FAILED;
	}

   // buffer B should be Sky frame: check for data, and it match Bufc in size.
	if( (bufb->status==DF_EMPTY) || 
	    (bufb->naxis1 != bufc->naxis1 ) || 
		 (bufb->naxis2 != bufc->naxis2 ) )
	{
      strcpy( reply, "BufB empty/diff." );
	   return ERR_FAILED;
	}

   // Get slit data from bufC 
	if( df_search_fheader( bufc->fheader, "SLIT", buf, sizeof(buf), &f_hdr, &i, FALSE) >=0 )
	{
	    //printf("   SLIT>> %s \n", buf);
		 if( ((i=parseSelection_r( buf, " /\n", &str_ptr, Gd_slit_selection)) >= 0) &&
	        (bufc->arcsec_pixel > 0.01))
		    *r_wid  = wid = Gd_slit_wid_selection[i]/bufc->arcsec_pixel;
        else
		    *r_wid  = wid = 6;
	}
	// update SN xy with new wid.
	*r_sn_x = sn_x = ax - wid/2;
	*r_sn_y = sn_y = ay - (28/2);  // SN box hgt is 28

   // stats on BoxC
	gain = 1.50;  // gain of bigdog 
   cal_box_stats_subf( bufc, 1, sn_x, sn_y, wid,  7, 0, 0, &N, &min, &max, &sum, &mean, &std);
	c3 = sum*gain;
   cal_box_stats_subf( bufc, 1, sn_x, sn_y+ 7, wid, 14, 0, 0, &N, &min, &max, &sum, &mean, &std);
	c1 = sum*gain;
   cal_box_stats_subf( bufc, 1, sn_x, sn_y+21, wid,  7, 0, 0, &N, &min, &max, &sum, &mean, &std);
	c2 = sum*gain;

   // stats on BoxB
   cal_box_stats_subf( bufb, 1, sn_x, sn_y+ 7, wid, 14, 0, 0, &N, &min, &max, &sum, &mean, &std);
	b1 = sum*gain;

   /*
	**  Bigdog Gain is 1.5 e/DN;
	**  c1, c2, c3, b1 are the sum_of_the_boxes * gain.
	**
	**         ((c1-(c2+c3))) 
	**  SN =  -----------------------------------
	**         (((c1-(c2+c3)) + b1)**0.5)
	**
	**  or SN = ((c1-(c2+c3))) / (((c1-(c2+c3)) + b1)**0.5)
	*/
	x = c1 -(c2+c3);
	d = sqrt( (x) + (b1) );
	sn = d==0? 0 : (x / d);
	if( !isnormal(sn) )
	   sn = 0;
	*r_sn_ratio = sn;
	//printf(" c1=%9.1f; c2=%9.1f; c3=%9.1f; b1=%9.1f;  SN=%.1f\n", c1, c2, c3, b1, sn);

	return ERR_NONE;;
	#undef NSLITS  
}

/*----------------------------------------------------------------------------
**  draw_stats() - Display the FITS Header information.
**----------------------------------------------------------------------------
*/
void draw_stats( GtkWidget *da, struct dpy_t *dp, struct df_buf_t *bp )
{
   char buf[85];
   int x, y, l,
       char_wid, char_hgt,
       da_wid, da_hgt;
   float mean;
   struct stats_t *sptr;

	struct ds_t ds_var;
	struct ds_t *ds;

   // no data? 
	if( bp->N == 0 ) 
   {
      draw_no_data( da, dp, bp);
      return;
   }

   /* Check that the drawing area has been initialized. */
   if ( da->window == NULL )
      return;

	// some basic stuff for DA's
	gdk_drawable_get_size( da->window, &da_wid, &da_hgt );
	ds = init_ds( &ds_var, da, W.nor_gc, &W.fixed_font, W.fixed_font_wid, W.fixed_font_hgt);

   /* clears window */
   gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_BLACK] );
   gdk_draw_rectangle( da->window, W.nor_gc, TRUE, 0, 0, da_wid, da_hgt);

   char_wid = W.fixed_font_wid;
   char_hgt = W.fixed_font_hgt;

   sptr = &Stats[dp->bufinx];   /* stats  pointer */

   /*----------------------
   ** Titles
   */
	da_set_foreground( ds, CM_BLACK );
	da_set_background( ds, CM_GREEN );

	draw_image_text(ds, U_PIXEL, 10*char_wid,  3*char_hgt, "    Object Box" );
	draw_image_text(ds, U_PIXEL, 27*char_wid,  3*char_hgt, "       Sky Box" );
	draw_image_text(ds, U_PIXEL, 10*char_wid, 11*char_hgt, "     Obj - Sky" );
	draw_image_text(ds, U_PIXEL, 27*char_wid, 11*char_hgt, "Photometry Est" );

	da_set_background( ds, CM_BLACK );

   /*----------------------
   ** Labels
   */
	da_set_foreground( ds, CM_GRAY );

   y = 0;
   x = char_wid;
	draw_text(ds, U_PIXEL, x, y,           "Buffer:" );
	draw_text(ds, U_PIXEL, x, y+=char_hgt, "   Min:" );
	draw_text(ds, U_PIXEL, x, y+=char_hgt, "   Max:" );

   y = 0;
   x = 20*char_wid;
	draw_text(ds, U_PIXEL, x, y,           "  Mean:" );
	draw_text(ds, U_PIXEL, x, y+=char_hgt, "   Var:" );
	draw_text(ds, U_PIXEL, x, y+=char_hgt, "StdDev:" );

   y = 3*char_hgt;
   x = char_wid;
	draw_text(ds, U_PIXEL, x, y+=char_hgt, "loc/size:" );
	draw_text(ds, U_PIXEL, x, y+=char_hgt, "     Sum:" );
	draw_text(ds, U_PIXEL, x, y+=char_hgt, "    Mean:" );
	draw_text(ds, U_PIXEL, x, y+=char_hgt, " Std Dev:" );
	draw_text(ds, U_PIXEL, x, y+=char_hgt, "NumPixel:" );
	draw_text(ds, U_PIXEL, x, y+=char_hgt, " Minimum:" );
	draw_text(ds, U_PIXEL, x, y+=char_hgt, " Maximum:" );

   y = 11*char_hgt;
   x = char_wid;
	draw_text(ds, U_PIXEL, x, y+=char_hgt, "     Sum:" );
	draw_text(ds, U_PIXEL, x, y+=char_hgt, "    Mean:" );
	draw_text(ds, U_PIXEL, x, y+=char_hgt, " Std Dev:" );
	draw_text(ds, U_PIXEL, x, y+=char_hgt, "NumPixel:" );
	draw_text(ds, U_PIXEL, x, y+=char_hgt, " Minimum:" );
	draw_text(ds, U_PIXEL, x, y+=char_hgt, " Maximum:" );

   y = 11*char_hgt;
   x = 28*char_wid;
	draw_text(ds, U_PIXEL, x, y+=char_hgt, " F=" );
   y += char_hgt;
	draw_text(ds, U_PIXEL, x, y+=char_hgt, "ZP=" );
	draw_text(ds, U_PIXEL, x, y+=char_hgt, " K=" );
	draw_text(ds, U_PIXEL, x, y+=char_hgt, " X=" );

   /*----------------------
   ** Values
   */
   y = 0;
   gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_YELLOW] );

	draw_text(ds, U_PIXEL, 18*char_wid, y,           buffer_selection[dp->bufinx] );

   double2str_len( buf, bp->min, 8, 1); l=strlen(buf);
	draw_text( ds, U_PIXEL, (20-l)*char_wid, y+=char_hgt, buf );

   double2str_len( buf, bp->max, 8, 1); l=strlen(buf);
	draw_text( ds, U_PIXEL, (20-l)*char_wid, y+=char_hgt, buf );

   /* buffer's mean, var, std */
   y = 0;
   double2str_len( buf, bp->mean, 8, 1); l=strlen(buf);
	draw_text( ds, U_PIXEL, (42-l)*char_wid, y,           buf );

   double2str_len( buf, bp->stddev*bp->stddev, 8, 2); l=strlen(buf);
	draw_text( ds, U_PIXEL, (42-l)*char_wid, y+=char_hgt, buf );

   double2str_len( buf, bp->stddev, 8, 2); l=strlen(buf);
	draw_text( ds, U_PIXEL, (42-l)*char_wid, y+=char_hgt, buf );

   /* Print OBJECT values */
   y = 3*char_hgt;
   sprintf(buf, "(%d,%d)%dx%d ", sptr->objx, sptr->objy, sptr->objwid, sptr->objhgt); l = strlen(buf);
	draw_text( ds, U_PIXEL, (25-l)*char_wid, y+=char_hgt, buf, l);

   double2str_len( buf, sptr->objsum, 8, 1); l=strlen(buf);
	draw_text( ds, U_PIXEL, (25-l)*char_wid, y+=char_hgt, buf, l);

   mean = (sptr->objN>0 ? sptr->objsum/sptr->objN : 0);
   double2str_len( buf, mean, 8, 1); l=strlen(buf);
	draw_text( ds, U_PIXEL, (25-l)*char_wid, y+=char_hgt, buf, l);
   double2str_len( buf, sptr->objstd, 8, 2); l=strlen(buf);
	draw_text( ds, U_PIXEL, (25-l)*char_wid, y+=char_hgt, buf, l);

   sprintf(buf, "%d ", sptr->objN); l=strlen(buf);
	draw_text( ds, U_PIXEL, (25-l)*char_wid, y+=char_hgt, buf, l);

   double2str_len( buf, sptr->objmin, 8, 1); l=strlen(buf);
	draw_text( ds, U_PIXEL, (25-l)*char_wid, y+=char_hgt, buf, l);

   double2str_len( buf, sptr->objmax, 8, 1); l=strlen(buf);
	draw_text( ds, U_PIXEL, (25-l)*char_wid, y+=char_hgt, buf, l);

   /* Print SKY values */
   y = 3*char_hgt;
   sprintf(buf, "(%d,%d)%dx%d ", sptr->skyx, sptr->skyy, sptr->skywid, sptr->skyhgt); l = strlen(buf);
   draw_text( ds, U_PIXEL, (42-l)*char_wid, y+=char_hgt, buf, l);

   double2str_len( buf, sptr->skysum, 8, 1); l=strlen(buf);
   draw_text( ds, U_PIXEL, (42-l)*char_wid, y+=char_hgt, buf, l);

   mean = (sptr->skyN>0 ? sptr->skysum/sptr->skyN : 0);
   double2str_len( buf, mean, 8, 1); l=strlen(buf);
   draw_text( ds, U_PIXEL, (42-l)*char_wid, y+=char_hgt, buf, l);

   double2str_len( buf, sptr->skystd, 8, 2); l=strlen(buf);
   draw_text( ds, U_PIXEL, (42-l)*char_wid, y+=char_hgt, buf, l);

   sprintf(buf, "%d ", sptr->skyN); l=strlen(buf);
   draw_text( ds, U_PIXEL, (42-l)*char_wid, y+=char_hgt, buf, l);

   double2str_len( buf, sptr->skymin, 8, 1); l=strlen(buf);
   draw_text( ds, U_PIXEL, (42-l)*char_wid, y+=char_hgt, buf, l);

   double2str_len( buf, sptr->skymax, 8, 1); l=strlen(buf);
   draw_text( ds, U_PIXEL, (42-l)*char_wid, y+=char_hgt, buf, l);

   /* show obj-sky stats */
   y = 11*char_hgt;
   double2str_len( buf, sptr->redsum, 8, 1); l=strlen(buf);
   draw_text( ds, U_PIXEL, (25-l)*char_wid, y+=char_hgt, buf, l);

   mean = (sptr->redN>0 ? sptr->redsum/sptr->redN : 0);
   double2str_len( buf, mean, 8, 1); l=strlen(buf);
   draw_text( ds, U_PIXEL, (25-l)*char_wid, y+=char_hgt, buf, l);

   double2str_len( buf, sptr->redstd, 8, 2); l=strlen(buf);
   draw_text( ds, U_PIXEL, (25-l)*char_wid, y+=char_hgt, buf, l);

   sprintf(buf, "%d ", sptr->redN); l=strlen(buf);
   draw_text( ds, U_PIXEL, (25-l)*char_wid, y+=char_hgt, buf, l);

   double2str_len( buf, sptr->redmin, 8, 1); l=strlen(buf);
   draw_text( ds, U_PIXEL, (25-l)*char_wid, y+=char_hgt, buf, l);

   double2str_len( buf, sptr->redmax, 8, 1); l=strlen(buf);
   draw_text( ds, U_PIXEL, (25-l)*char_wid, y+=char_hgt, buf, l);

   /* calculate the estimate for aperture photometry */
   {
      double d, f;

      x = 31*char_wid;
      y = 11*char_hgt;

      d = bp->itime * (Lc.divbydivisor ? 1 : bp->divisor);
      if( d <= 0.0 ) d = 1;
      f = (sptr->redsum / d );

      sprintf(buf, "%1.0f/%4.2f ", sptr->redsum, d ); l=strlen(buf);
      draw_text( ds, U_PIXEL, x, y+=char_hgt, buf, l );

      sprintf(buf, "%4.2f ", f ); l=strlen(buf);
      draw_text( ds, U_PIXEL, x, y+=char_hgt, buf, l );

      sprintf(buf, "%4.2f", bp->filter_zp); l=strlen(buf);
      draw_text( ds, U_PIXEL, x, y+=char_hgt, buf, l );

      sprintf(buf, "%4.2f", bp->ext_coff); l=strlen(buf);
      draw_text( ds, U_PIXEL, x, y+=char_hgt, buf, l );

      sprintf(buf, "%4.2f", bp->airmass); l=strlen(buf);
      draw_text( ds, U_PIXEL, x, y+=char_hgt, buf, l );

      x = 2*char_wid;
		y = 17*char_hgt;
		gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_GREEN] );
      if( f > 0.0 )
      {
         d = ( -2.5 * log10(f)) + bp->filter_zp - (bp->ext_coff*bp->airmass);
			sprintf( buf, "MagEst= -2.5log(F)+ZP-(K*X) = %5.3f ", d);
      }
      else
         strcpy( buf, "MagEst= Invalid Input. No Est.");
      l=strlen(buf);
      draw_text( ds, U_PIXEL, x, y+=char_hgt, buf, l );
   }
}

/*----------------------------------------------------------------------------
**  draw_aofig() - draw the ao figure 
**----------------------------------------------------------------------------
*/
void draw_aofig( GtkWidget *da, struct dpy_t *dp, struct df_buf_t *bp )
{

   /* Check that the drawing area has been initialized. */
   if ( da->window == NULL )
      return;

   // no data? 
	if( bp->N == 0 ) 
   {
      draw_no_data( da, dp, bp);
      return;
   }

   /*----------------------------------------
   ** check/set autoscaling 
   */
   autoscale_image_range( dp, bp );

   switch( dp->aofig_format )
   {
      case 0:
          draw_aofig_text_format( da, dp, bp );
          break;
      case 1:
      case 2:
          draw_aofig_DM_Sensor_format( da, dp, bp );
          break;
      default:
          draw_NA( da, dp, bp );
          break;
   }

}

void draw_aofig_text_format( GtkWidget *da, struct dpy_t *dp, struct df_buf_t *bp )
{
   char buf[80];
   int dpinx,
       i, 
       x, y,
       aofig_x, aofig_y,
       char_wid, char_hgt,
       da_wid, da_hgt;

   struct ds_t ds_var;
	struct ds_t *ds;

   /* Check that the drawing area has been initialized. */
   if ( da->window == NULL )
      return;

	// some basic stuff for DA's
	gdk_drawable_get_size( da->window, &da_wid, &da_hgt );
	ds = init_ds( &ds_var, da, W.nor_gc, &W.fixed_font, W.fixed_font_wid, W.fixed_font_hgt);
   char_wid = W.fixed_font_wid;
   char_hgt = W.fixed_font_hgt;

   dpinx = (intptr_t)  g_object_get_data( G_OBJECT(da), DPYINX_DATA_KEY);

   /* clears window */
   gdk_drawable_get_size( da->window, &da_wid, &da_hgt);
   gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_BLACK] );
   gdk_draw_rectangle( da->window, W.nor_gc, TRUE, 0, 0, da_wid, da_hgt);

   /*----------------------
   ** Titles
   */
	da_set_foreground( ds, CM_BLACK );
	da_set_background( ds, CM_GREEN );

	draw_image_text( ds, U_PIXEL,  1*char_wid,  1*char_hgt, "   Ring 0   " );
	draw_image_text( ds, U_PIXEL, 14*char_wid,  1*char_hgt, "   Ring 1   " );
	draw_image_text( ds, U_PIXEL, 29*char_wid,  1*char_hgt, "   Ring 2   " );

	da_set_background( ds, CM_BLACK );

   /*----------------
   ** Labels
   */
   gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_GRAY] );
   x =  1*char_wid;
   y =  1*char_hgt;
   for( i=0; i <= 5; i++ )
   {
      sprintf( buf, "%2d", i);
      draw_text( ds, U_PIXEL, x, y+=char_hgt, buf );
   }
   x = 14*char_wid;
   y =  1*char_hgt;
   for( i=6; i <= 17; i++ )
   {
      sprintf( buf, "%2d", i);
      draw_text( ds, U_PIXEL, x, y+=char_hgt, buf );
   }
   x = 29*char_wid;
   y =  1*char_hgt;
   for( i=18; i <= 35; i++ )
   {
      sprintf( buf, "%2d", i);
      draw_text( ds, U_PIXEL, x, y+=char_hgt, buf );
   }

   /*----------------
   ** Data
   */
   aofig_x = dp->aofig_x;
   aofig_y = dp->aofig_y;

   gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_YELLOW] );
   x =  4*char_wid;
   y =  1*char_hgt;
   for( i=0; i <= 5; i++ )
   {
		double2str_len( buf, dfdataxy( bp, aofig_x, aofig_y ), 9, 3); 
      draw_text( ds, U_PIXEL, x, y+=char_hgt, buf );
      if( dp->aofig_data )
         aofig_y++;
      else
         aofig_x++;
   }
   x = 17*char_wid;
   y =  1*char_hgt;
   for( i=6; i <= 17; i++ )
   {
		double2str_len( buf, dfdataxy( bp, aofig_x, aofig_y ), 9, 3 ); 
      draw_text( ds, U_PIXEL, x, y+=char_hgt, buf );
      if( dp->aofig_data )
         aofig_y++;
      else
         aofig_x++;
   }
   x = 32*char_wid;
   y =  1*char_hgt;
   for( i=18; i <= 35; i++ )
   {
		double2str_len( buf, dfdataxy( bp, aofig_x, aofig_y ), 9, 3); 
      draw_text( ds, U_PIXEL, x, y+=char_hgt, buf );
      if( dp->aofig_data )
         aofig_y++;
      else
         aofig_x++;
   }

}

void draw_aofig_DM_Sensor_format( GtkWidget *da, struct dpy_t *dp, struct df_buf_t *bp )
{
   int char_wid, char_hgt,
       da_wid, da_hgt;

	polygon_t *ptable;

   /* Check that the drawing area has been initialized. */
   if ( da->window == NULL )
      return;

   char_wid = W.fixed_font_wid;
   char_hgt = W.fixed_font_hgt;

   gdk_drawable_get_size( da->window, &da_wid, &da_hgt);

   /* clears window */
   //gdk_window_clear( da->window );
   gdk_drawable_get_size( da->window, &da_wid, &da_hgt);
   gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_BLACK] );
   gdk_draw_rectangle( da->window, W.nor_gc, TRUE, 0, 0, da_wid, da_hgt);

   if( dp->aofig_format == AOFIG_FORMAT_DM )
		ptable = AO_DM_Table;
   else
		ptable = AO_Sensor_Table;

   /*----------------
   ** Draw figure
   */
   {
      int x, y, w, h, 
			 aofig_x, aofig_y,
          c, e, p;
      float block, data;
	   GdkPoint pt[5];

      // put figure in this x,y,w,h box
		x = char_wid/2;
		y = char_hgt/2;
		w = da_wid - char_wid;
		h = da_hgt - char_hgt;
		w = MIN( w, h);   // make the figure square
		h = MIN( w, h);

		aofig_x = dp->aofig_x;
		aofig_y = dp->aofig_y;

		block = (dp->image_max - dp->image_min) / ((float)CM_NUM_RW_COLORS);

		for( e=0; e < AOFIG_NUM_ELEMENTS; e++ )
		{
			// Map polygon dimension to window size.
			for( p=0; p < ptable[e].n; p++)
			{
				pt[p].x = rint( map( ptable[e].pt[p].x, 0, 1000, x, x+w));
				pt[p].y = rint( map( ptable[e].pt[p].y, 0, 1000, y, y+h));
			}

			// Determine the data represent
         data = dfdataxy( bp, aofig_x, aofig_y );
			if( dp->aofig_data )
				aofig_y++;
			else
				aofig_x++;

			// translate data value to color.
         if( data >= dp->image_max )
            c = CM_NUM_COLORS-1;  /* last color */
			else if( data <= dp->image_min )
			   c = CM_NUM_STATIC_COLORS; 
			else 
			   c = ( (data - dp->image_min) / block) + CM_NUM_STATIC_COLORS;

			// filled polygon
			gdk_gc_set_foreground( W.nor_gc, &CM.colors[c]);
			gdk_draw_polygon( da->window, W.nor_gc, TRUE, pt, ptable[e].n);

			// outline polygon
			gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_GREEN] );
				gdk_draw_polygon( da->window, W.nor_gc, FALSE, pt, ptable[e].n);
		}

   }

}

/*----------------------------------------------------------------
**  draw_sa() - displays the spectograph A graph.
**----------------------------------------------------------------
*/
void draw_sa( GtkWidget *da, struct dpy_t *dp, struct df_buf_t *bp )
{
  int     da_wid, da_hgt,  /* size of paint window                */
          x1, y1, x2, y2,
          xmin, xmax,      /* Graph area min, max, & size in pixels */
          ymin, ymax,
          char_wid,
          char_hgt,
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

   struct ds_t ds_var;
   struct ds_t *ds;

   // no data? 
	if( bp->N == 0 ) 
   {
      draw_no_data( da, dp, bp);
      return;
   }

   // some basic stuff for DA's
   gdk_drawable_get_size( da->window, &da_wid, &da_hgt );
   ds = init_ds( &ds_var, da, W.nor_gc, &W.fixed_font, W.fixed_font_wid, W.fixed_font_hgt);
	char_wid = W.fixed_font_wid;
	char_hgt = W.fixed_font_hgt;

   /* Allocate memory */
   numgraph = (int) ceil( ((double)dp->sa_objbin_max-dp->sa_objbin_min+1) / dp->sa_rows_per_bin );

   sky    = (struct table_t *) calloc( 1, sizeof(struct table_t));
   object = (struct table_t *) calloc( numgraph, sizeof(struct table_t));

   if( object==NULL || sky==NULL )
   {
      if( object != NULL) free( object );
      if( sky    != NULL) free( sky    );
      perror("Memory allocation error");
      return;
   }

   /*
   **  Dimension graph area.
   */
   xmin =  2*char_wid;
   xmax =  da_wid - 7*char_wid;
   ymin =  da_hgt - 3*char_hgt;
   ymax =  char_hgt;

   /*
   **  Display header
   */
	gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_BLACK]);
	gdk_draw_rectangle( da->window, W.nor_gc, TRUE, 0, 0, da_wid, da_hgt);
	gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_GREEN]);

   strcpy( buf, "SPECTRA A"); l = strlen(buf);
   x1 = (da_wid - l*char_wid)/2;
   draw_text( ds, U_PIXEL, x1, 0, buf);
   /*
   **  Get the mean values for the sky
   */
   if( dp->sa_subtractsky )
   {
      sky->min = DF_MAX_SIGNED_INT32;
      sky->max = DF_MIN_SIGNED_INT32;
      sky->begrow = MAX(0, dp->sa_skybin_min);
      sky->endrow = MIN(dp->sa_skybin_max, bp->naxis2-1);

      for( x1 = MAX(0, dp->sa_xmin); x1 <= MIN(dp->sa_xmax, bp->naxis1-1); x1++)
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
      object[igraph].begrow = MAX(object[igraph].begrow, 0); 

      object[igraph].endrow = MIN(object[igraph].begrow+dp->sa_rows_per_bin-1,
                                  dp->sa_objbin_max);
      object[igraph].endrow = MIN(object[igraph].endrow, bp->naxis2-1);

/***
      printf("obj bin %d is loop x %d to %d, y %d to %d\n", igraph,
         MAX(0, Dpy[dpinx].sa_xmin), MIN(Dpy[dpinx].sa_xmax, Buffer[bufinx].naxis1-1),
         object[igraph].begrow, object[igraph].endrow);
***/

      for( x1 = MAX(0, dp->sa_xmin); x1 <= MIN(dp->sa_xmax, bp->naxis1-1); x1++)
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
      x1 = char_wid;
      y1 = 2 * char_hgt;
		gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_WHITE] );
      draw_text( ds, U_PIXEL, x1, y1, " Rows     N     Mean   STD   Ratio");
      y1 += char_hgt;

		gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_GREEN] );
      for( igraph=0; igraph < numgraph; igraph++)
      {
         /* Calculate stats */
         n = sum = sdd = 0;
         for( x1 = MAX(0, dp->sa_xmin); x1 <= MIN(dp->sa_xmax, bp->naxis1-1); x1++)
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
			draw_text( ds, U_PIXEL, char_wid, y1, "%3d-%3d  %3.0f  %6.0f %6.2f %6.2f",
			   object[igraph].begrow, object[igraph].endrow, n, mean, std, mean/std);
         y1 += char_hgt;
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
		gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_GREEN] );
		gdk_draw_line( da->window, W.nor_gc, xmin, ymin, xmax, ymin);
      y1 = ymin+char_hgt/2;        /* Scale make extend to y1 */
      for( i=0; i <= 100; i+=25 )
      {
         l = ((float)i/100) *  (dp->sa_xmax-dp->sa_xmin+1) + (dp->sa_xmin);
         sprintf( buf, "%d", l + dp->sa_shift); l = strlen(buf);

         x1 = map( i, 0, 100, xmin, xmax);
			gdk_draw_line( da->window, W.nor_gc, x1, ymin, x1, y1);
         x1 -= (l*char_wid)/2;
			draw_text( ds, U_PIXEL,  x1, y1, buf);
      }
      /*
      ** Graph each set of data
      */
      for( igraph=0; igraph < numgraph; igraph++)
      {
			gdk_gc_set_foreground( W.nor_gc, &CM.colors[(isodd(igraph)?CM_RED:CM_WHITE)] );
         /*
         **  Determine demension of graph area. Only Y's get adjusted.
         **  And draw Y axis.
         */
         ymin = da_hgt - 3*char_hgt;
         ymax = char_hgt;
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

         x1 = xmax + char_wid;
			gdk_draw_line( da->window, W.nor_gc, xmax, ymin, x1, ymin);
			gdk_draw_line( da->window, W.nor_gc, xmax, ymin, xmax, ymax);
			gdk_draw_line( da->window, W.nor_gc, xmax, ymax, x1, ymax);
         draw_text( ds, U_PIXEL,  x1, ymin-char_hgt, "%2.0f", yscale_min);
         draw_text( ds, U_PIXEL,  x1, ymax, "%2.0f", yscale_max);
         /*
         **  Graph the data
         **  Remember: XScale refers to naxis2, YScale refers to data values.
         */
         i = MAX( 0, dp->sa_xmin);             /* Get x,y of first data point */
         x1 = map( i, dp->sa_xmin, dp->sa_xmax+1, xmin, xmax);
         y1 = map( object[igraph].mean[i], yscale_min, yscale_max, ymin, ymax);
         /**
         printf("DrawLoop x %d to %d\n", i,
              MIN( Buffer[bufinx].naxis1-1, Dpy[dpinx].sa_xmax));
         **/

         for( ; i <= MIN( bp->naxis1-1, dp->sa_xmax); i++)
         {
            x2 = map( i+1, dp->sa_xmin, dp->sa_xmax+1, xmin, xmax);
            y2 = map( object[igraph].mean[i], yscale_min, yscale_max, ymin, ymax);

            /*printf("[%5.1f %d]", object[igraph].mean[i], object[igraph].cnt[i]);*/
				gdk_draw_line( da->window, W.nor_gc, x1, y1, x1, y2); 
				gdk_draw_line( da->window, W.nor_gc, x1, y2, x2, y2);

            x1 = x2;
            y1 = y2;
         }
      }
   }
   /*
   **  clean up
   */
   free( (char*) object );
   free( (char*) sky );
}


/*----------------------------------------------------------------
**  draw_sb() - displays the spectograph B graph.
**----------------------------------------------------------------
*/
void draw_sb( GtkWidget *da, struct dpy_t *dp, struct df_buf_t *bp )
{
  int    da_wid, da_hgt,  /* size of paint window                */
          x1, y1, x2, y2,
          xmin, xmax,      /* Graph area min, max, & size in pixels */
          ymin, ymax,
          imin, imax,
          char_wid,
          char_hgt,
          object,
          inx,
          l, i, cnt;

   float  *sky_mean,
          *obj_mean,
          num_sky_rows,
          num_obj_rows,
          diff_ymin, diff_ymax,
          data_ymin, data_ymax,
          fdata, fsum;

   char   buf[80];

   struct ds_t ds_var;
   struct ds_t *ds;

   // no data? 
	if( bp->N == 0 ) 
   {
      draw_no_data( da, dp, bp);
      return;
   }

   // some basic stuff for DA's
   gdk_drawable_get_size( da->window, &da_wid, &da_hgt );
   ds = init_ds( &ds_var, da, W.nor_gc, &W.fixed_font, W.fixed_font_wid, W.fixed_font_hgt);
   char_wid = W.fixed_font_wid;
   char_hgt = W.fixed_font_hgt;

   if( NULL == ( sky_mean = (float*) malloc( sizeof(float)*NUM_PIXEL) ) )
   {
      printf("Memory allocation error\n");
      return;
   }
   if( NULL == ( obj_mean = (float*) malloc( sizeof(float)*NUM_PIXEL) ) )
   {
      free( (char*) sky_mean );
      printf("Memory allocation error\n");
      return;
   }

   /*
   **  Dimension graph area.
   */
   xmin =  2*char_wid;
   xmax =  da_wid - 7*char_wid;
   ymin =  da_hgt - 3*char_hgt;
   ymax =  char_hgt;

   /*
   **  Display header & X axis labels
   */
   gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_BLACK]);    // canvas it black
	gdk_draw_rectangle( da->window, W.nor_gc, TRUE, 0, 0, da_wid, da_hgt);
	gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_GREEN]);    // graph is green

   strcpy( buf, "SPECTRA B"); l = strlen(buf);             // display header
	x1 = (da_wid - l*char_wid)/2;
	draw_text( ds, U_PIXEL, x1, 0, buf);

   gdk_draw_line( da->window, W.nor_gc, xmin, ymin, xmax, ymin);
   y1 = ymin+char_hgt/2;        /* Scale make extend to y1 */
   for( i=0; i <= 100; i+=25 )
   {
      l = map(  i, 0.0, 100.0, dp->sb_xmin, dp->sb_xmax+1);
      sprintf( buf, "%d", l); l = strlen(buf);

      x1 = map(i, 0.0, 100.0, xmin, xmax);

		gdk_draw_line( da->window, W.nor_gc, x1, ymin, x1, y1);
      x1 -= (l*char_wid)/2;
	   draw_text( ds, U_PIXEL, x1, y1, buf);
   }
   /*
   **  Find the mean for obj and sky bins.
   **  1st, sum the pixel by columns.
   */
   memset( (char*) sky_mean, 0, sizeof(float)*NUM_PIXEL);
   memset( (char*) obj_mean, 0, sizeof(float)*NUM_PIXEL);

   for( y1 = dp->sb_objbin_min;
        (y1 <= dp->sb_objbin_max) && (y1 < bp->naxis2);
        y1++)
      for( x1 = dp->sb_xmin;
           (x1 <= dp->sb_xmax) && (x1 < bp->naxis1);
           x1++)
      {
         inx = y1 * bp->naxis1 + x1;
			obj_mean[x1] += dfdatainx( bp, inx);
      }

   for( y1 = dp->sb_skybin_min;
        (y1 <= dp->sb_skybin_max) && (y1 < bp->naxis2);
        y1++)
      for( x1 = dp->sb_xmin;
           (x1 <= dp->sb_xmax) && (x1 < bp->naxis1);
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
      ymin = da_hgt - 3*char_hgt;
      ymax = char_hgt;
      if( dp->sb_showgraph & 0x06 )   // adjust if 'O' or 'S' is displayed
      {
         l = ((ymin-ymax+1)/2) - (char_hgt/2);
         ymin = ymax + l -1;
      }

      /*  Draw Y Axis Scale  */
      x1 = xmax+1;
		gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_RED] );
		gdk_draw_line( da->window, W.nor_gc, x1, ymin, x1, ymax); 
      x2 = x1 + char_wid;
      for( i=0; i <= 100; i+=25 )
      {
         l = map( i, 0.0, 100.0, diff_ymin, diff_ymax);
         sprintf( buf, "%d", l);

         y1 = map( i, 0.0, 100.0, ymin, ymax);
		   gdk_draw_line( da->window, W.nor_gc, x1, y1, x2, y1); 
			draw_text( ds, U_PIXEL, x2, y1-char_hgt, buf);
      }

      /*  Graph the Data  */
      fsum = 0;
      cnt = 0;
      /* Graph the points from imin to imax */
      imin = MAX( 0, dp->sb_xmin);
      imax = MIN( bp->naxis1-1, dp->sb_xmax);

      i = imin;                      /* get x, y of first data point */
      fdata = obj_mean[i] - sky_mean[i];

      x1 = map( i, dp->sb_xmin, dp->sb_xmax+1, xmin, xmax);
      y1 = map( fdata, diff_ymin, diff_ymax, ymin, ymax);

      for( i = imin; i <= imax; i++)
      {
         fdata = obj_mean[i] - sky_mean[i];
         fsum += fdata; cnt++;
         x2 = map( i+1, dp->sb_xmin, dp->sb_xmax+1, xmin, xmax);
         y2 = map(fdata, diff_ymin, diff_ymax, ymin, ymax);

			gdk_draw_line( da->window, W.nor_gc, x1, y1, x1, y2);
			gdk_draw_line( da->window, W.nor_gc, x1, y2, x2, y2);

         x1 = x2;
         y1 = y2;
      }

      /* display label */
      if( cnt )
      {
         y1 = map( fsum/cnt, diff_ymin, diff_ymax, ymin, ymax);
			draw_text( ds, U_PIXEL, char_wid/2, y1-char_hgt, "D");
      }
   }

   /*
   **  Draw the Obj & Sky Graph.
   */
   if( (dp->sb_showgraph & 0x06) )
   {
      /*  Dimension area for graph */
      ymin = da_hgt - 3*char_hgt;
      ymax = char_hgt;
      if( dp->sb_showgraph & 0x01 )   // adjust if 'D' is displayed
      {
         l = ((ymin-ymax+1)/2) - (char_hgt/2);
         ymax = ymin - l -1;
      }

      /*  Draw Y Axis Scale  */
      x1 = xmax + 1;
      x2 = x1 + char_wid;
		gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_WHITE] );
		gdk_draw_line( da->window, W.nor_gc, x1, ymin, x1, ymax);
      for( i=0; i <= 100; i+=25 )
      {
         l = map(  i, 0.0, 100.0, data_ymin, data_ymax);
         sprintf( buf, "%d", l);

         y1 = map( i, 0.0, 100.0, ymin, ymax);
			gdk_draw_line( da->window, W.nor_gc, x1, y1, x2, y1);

			draw_text( ds, U_PIXEL,  x2, y1-char_hgt, buf);
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

				gdk_gc_set_foreground( W.nor_gc, &CM.colors[(object?CM_WHITE:CM_YELLOW)] );
            i = MAX( 0, dp->sb_xmin);          /* get x, y of first data point */
            fdata = (object ? obj_mean[i] : sky_mean[i]);
            x1 = map(i, dp->sb_xmin, dp->sb_xmax+1, xmin, xmax);
            y1 = map(fdata, data_ymin, data_ymax, ymin, ymax);

            for( i = imin; i <= imax; i++)
            {
               fdata = (object ? obj_mean[i] : sky_mean[i]);
               fsum += fdata; cnt++;

               x2 = map(i+1, dp->sb_xmin, dp->sb_xmax+1, xmin, xmax);
               y2 = map(fdata, data_ymin, data_ymax, ymin, ymax);

               gdk_draw_line( da->window, W.nor_gc, x1, y1, x1, y2);
               gdk_draw_line( da->window, W.nor_gc, x1, y2, x2, y2);

               x1 = x2;
               y1 = y2;
            }
            /* display label */
            if( cnt )
            {
               y1 = map(fsum/cnt, data_ymin, data_ymax, ymin, ymax);
					draw_text( ds, U_PIXEL,  char_wid/2, y1-char_hgt, (object ? "O":"S"));
            }
         }
      }
   }
   /*
   **  clean up
   */
   free( (char*) obj_mean );
   free( (char*) sky_mean );
}


/*----------------------------------------------------------------------------
**  draw_NA() - Not Available - use this until draw_() is written.
**----------------------------------------------------------------------------
*/
void draw_NA( GtkWidget *da, struct dpy_t *dp, struct df_buf_t *bp )
{
   char buf[80];
   int dpinx,
       x, y,
       char_wid, char_hgt,
       da_wid, da_hgt;

   struct ds_t ds_var;
   struct ds_t *ds;

   /* Check that the drawing area has been initialized. */
   if ( da->window == NULL )
      return;

   // some basic stuff for DA's
   gdk_drawable_get_size( da->window, &da_wid, &da_hgt );
   ds = init_ds( &ds_var, da, W.nor_gc, &W.fixed_font, W.fixed_font_wid, W.fixed_font_hgt);
   char_wid = W.fixed_font_wid;
   char_hgt = W.fixed_font_hgt;

   dpinx = (intptr_t) g_object_get_data( G_OBJECT(da), DPYINX_DATA_KEY);

   /* clears window */
   gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_GRAY] );
   gdk_draw_rectangle( da->window, W.nor_gc, TRUE, 0, 0, da_wid, da_hgt);

   /* draw a frame in window  */
   gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_BLACK] );
   gdk_draw_rectangle( da->window, W.nor_gc, FALSE, 0, 0, da_wid-1, da_hgt-1);

   x = char_wid;
   y = char_hgt;

   sprintf( buf, "dpyinx = %d " , dpinx );
   draw_text( ds, U_PIXEL, x, y+=char_hgt, buf );

   sprintf( buf, "bufinx = %d " , dp->bufinx );
   draw_text( ds, U_PIXEL, x, y+=char_hgt, buf );

   sprintf( buf, "dpytype = %s " , dpytype_selection[dp->dpytype] );
   draw_text( ds, U_PIXEL, x, y+=char_hgt, buf );

   sprintf( buf, "buf->status = %s ", buffer_status_names[bp->status] );
   draw_text( ds, U_PIXEL, x, y+=char_hgt, buf );
}

/********************************************************************************/
/*                   Scroll bars related routines                               */
/********************************************************************************/

/*----------------------------------------------------------------------------
**  dpy_vadj_cb() - Callback for dpy's vertical scrollbar adjustments.
**----------------------------------------------------------------------------
*/
void dpy_vadj_cb( GtkAdjustment *adj, gpointer data )
{
   int dpinx            = (intptr_t) data;
   struct dpy_t *dp     = &Dpy[dpinx];         /* reference to dpy    */
   struct df_buf_t *bp  = &Buffer[dp->bufinx]; /* reference to buffer */

   if( bp->status == DF_EMPTY )
      return;

   if( dp->dpytype == DPYTYPE_HEADER )
   {
      dp->header_row = rint(adj->value);
      call_dpydata_redraw( dpinx );
   }
   else if( dp->dpytype == DPYTYPE_IMAGE )
   {
      dp->image_offy = rint(adj->value);
      call_dpydata_redraw( dpinx );
   }

}

/*----------------------------------------------------------------------------
**  dpy_hadj_cb() - Callback for dpy's horizontial scrollbar adjustments.
**----------------------------------------------------------------------------
*/
void dpy_hadj_cb( GtkAdjustment *adj, gpointer data )
{
   int dpinx            = (intptr_t) data;
   struct dpy_t *dp     = &Dpy[dpinx];         /* reference to dpy    */
   struct df_buf_t *bp  = &Buffer[dp->bufinx]; /* reference to buffer */

   if( bp->status == DF_EMPTY )
      return;

   if( dp->dpytype == DPYTYPE_HEADER )
   {
      dp->header_col = rint(adj->value);
      call_dpydata_redraw( dpinx );
   }
   else if( dp->dpytype == DPYTYPE_IMAGE )
   {
      dp->image_offx = rint(adj->value);
      call_dpydata_redraw( dpinx );
   }

}

/*----------------------------------------------------------------
**  update_scrollbars - set the scrollbars parameters according
**     to values in the Dpy strcuture.
**----------------------------------------------------------------
*/
void update_scrollbars( int dpinx )
{
   int da_wid, da_hgt;

   struct dpy_t    *dp = &Dpy[dpinx];            /* dpy pointer    */
   struct df_buf_t *bp = &Buffer[dp->bufinx];    /* buffer pointer */

   gfloat Hlower = 0,            /* minimum value */
          Hupper = 1,            /* maximum value */
          Hvalue = 0,            /* current value */
          Hstep_increment = 1,   /* step increment */
          Hpage_increment = 1,   /* page increment */
          Hpage_size = 1,        /* page size */
          Vlower = 0,
          Vupper = 1,
          Vvalue = 0,
          Vstep_increment = 1,
          Vpage_increment = 1,
          Vpage_size = 1;

   /* Check that the drawing area has been initialized. */
   if ( dp->data_drawingarea->window == NULL )
      return;

   /* get wid & hgt of drawing area */
   gdk_drawable_get_size( dp->data_drawingarea->window, &da_wid, &da_hgt);

   /*-------------------------------
   ** header
   */
   if( (bp->status != DF_EMPTY) && (dp->dpytype == DPYTYPE_HEADER) )
   {
      int char_wid = W.fixed_font_wid;
      int char_hgt = W.fixed_font_hgt;

      Hlower = 0;
      Hupper = 80;
      Hpage_size = MIN( da_wid / char_wid, Hupper);
      Hstep_increment = 1;
      Hpage_increment = Hpage_size-1;
      Hvalue = MIN(dp->header_col, Hupper-Hpage_size);
      Hvalue = MAX(Hvalue, 0 );

      Vlower = 0;
      Vupper = bp->Nheader-1;
      Vpage_size = MIN( da_hgt / char_hgt, Vupper);
      Vstep_increment = 1;
      Vpage_increment = Vpage_size-1;
      Vvalue = MIN(dp->header_row, Vupper-Vpage_size);
      Vvalue = MAX(Vvalue, 0 );
   }

   /*-------------------------------
   ** Image
   */
   if( (bp->status != DF_EMPTY) && (dp->dpytype == DPYTYPE_IMAGE) )
   {
      float pixel_wid = dp->image_zoom > 0 ? dp->image_zoom : 1.0/(1.0+abs(dp->image_zoom));

      Hlower = 0;
      Hupper = bp->naxis1;
      Hpage_size = MIN( da_wid / pixel_wid, bp->naxis1);
      Hstep_increment = 1;
      Hpage_increment = Hpage_size;
      Hvalue = dp->image_offx;

      Vlower = 0;
      Vupper = bp->naxis2;
      Vpage_size = MIN( da_hgt / pixel_wid, bp->naxis2);
      Vstep_increment = 1;
      Vpage_increment = Vpage_size;
      Vvalue = dp->image_offy;
   }

   /*--------------------------------------------------------
   ** write changes back to the scrollbar/adjustment widget
   */
#if DEBUG
   printf("update_scrollbars\n");
   printf("V: lower=%5.1f upper=%5.1f S_inc=%5.1f P_inc=%5.1f P_size=%5.1f value=%5.1f\n",
      Vlower, Vupper, Vstep_increment, Vpage_increment, Vpage_size, Vvalue);
   printf("H: lower=%5.1f upper=%5.1f S_inc=%5.1f P_inc=%5.1f P_size=%5.1f value=%5.1f\n",
      Hlower, Hupper, Hstep_increment, Hpage_increment, Hpage_size, Hvalue);
#endif

   dp->hadj->lower = Hlower;
   dp->hadj->upper = Hupper;
   dp->hadj->value = Hvalue;
   dp->hadj->step_increment = Hstep_increment;
   dp->hadj->page_increment = Hpage_increment;
   dp->hadj->page_size = Hpage_size;

   dp->vadj->lower = Vlower;
   dp->vadj->upper = Vupper;
   dp->vadj->value = Vvalue;
   dp->vadj->step_increment = Vstep_increment;
   dp->vadj->page_increment = Vpage_increment;
   dp->vadj->page_size = Vpage_size;

   gtk_adjustment_changed( dp->hadj );
   gtk_adjustment_changed( dp->vadj );
}

/********************************************************************************/
/* draw_XXX_on_dpy/buffer() pixel based drawing routines for imageing.                 */
/********************************************************************************/

/*----------------------------------------------------------------
**  draw_xor_box_on_dpy() - Draw the object box on the canvas
**----------------------------------------------------------------
*/
void draw_xor_box_on_dpy( dpinx, x, y, wid, hgt )
   int dpinx;            /* Identifies canvas to xor on     */
   int x, y, wid, hgt;   /* Draw a box around these pixels  */
{
   int      box_x, box_y, box_wid, box_hgt;
   float    pixel_wid;

   struct dpy_t *dp = &Dpy[dpinx];

   /* determine position of upper-left pixel */
   pixel_wid = dp->image_zoom > 0 ? dp->image_zoom : 1.0/(1.0+abs(dp->image_zoom));
   box_x = ((x - dp->image_offx) * pixel_wid) + 0.5;
   box_y = ((y - dp->image_offy) * pixel_wid) + 0.5;

   box_wid = wid * pixel_wid;
   box_hgt = hgt * pixel_wid;
   /* Adjust values so rectangle is draw around the pixels */
   box_x--; box_y--; box_wid++; box_hgt++;

   /* Draw it */
   gdk_gc_set_foreground( W.xor_gc, &CM.colors[CM_GREEN] );
   gdk_draw_rectangle( dp->data_drawingarea->window, W.xor_gc, FALSE, box_x, box_y, box_wid, box_hgt);

}

/*----------------------------------------------------------------
**  draw_xor_box_on_buffer() - Calls draw_xor_on_dpy for each
**     canvas displaying data from this buffer.
**----------------------------------------------------------------
*/
void draw_xor_box_on_buffer( bufinx, x, y, wid, hgt )
   int bufinx;            /* Update all images of data in Buffer[bufinx] */
   int x, y, wid, hgt;    /* Draw a box around these pixels              */
{
   int      dpinx;

   for( dpinx = 0; dpinx < Lc.num_dpy; dpinx++)
      if( Dpy[dpinx].bufinx == bufinx && Dpy[dpinx].dpytype == DPYTYPE_IMAGE )
      {
         draw_xor_box_on_dpy( dpinx, x, y, wid, hgt);
      }
}

/*----------------------------------------------------------------
**  draw_xor_line_on_dpy() - Draw the xor line on the canvas
**----------------------------------------------------------------
*/
void draw_xor_line_on_dpy( dpinx, xbeg, ybeg, xend, yend )
   int dpinx;                   /* Identifies canvas to xor on         */
   int xbeg, ybeg, xend, yend;  /* Draw a line from (x1,y1) to (x2,y2) */
{
   int      x1, y1, x2, y2;
   float    pixel_wid;

   struct dpy_t *dp = &Dpy[dpinx];

   /* determine position of upper-left pixel */
   pixel_wid = dp->image_zoom > 0 ? dp->image_zoom : 1.0/(1.0+abs(dp->image_zoom));
   x1 = ((xbeg - dp->image_offx) * pixel_wid) + (pixel_wid/2);
   y1 = ((ybeg - dp->image_offy) * pixel_wid) + (pixel_wid/2);

   x2 = ((xend - dp->image_offx) * pixel_wid) + (pixel_wid/2);
   y2 = ((yend - dp->image_offy) * pixel_wid) + (pixel_wid/2);

   /* Draw it */
   gdk_gc_set_foreground( W.xor_gc, &CM.colors[CM_WHITE] );
   gdk_draw_line( dp->data_drawingarea->window, W.xor_gc, x1, y1, x2, y2);
}

/*----------------------------------------------------------------
**  draw_xor_line_on_buffer() - Calls draw_xor_line_on_dpy for each
**     canvas displaying data from this buffer.
**----------------------------------------------------------------
*/
void draw_xor_line_on_buffer( bufinx, xbeg, ybeg, xend, yend )
   int bufinx;            /* Update all images of data in Buffer[bufinx] */
   int xbeg, ybeg, xend, yend;  /* Draw a line from (x,y)beg to (x,y)end */
{
   int      dpinx;

   for( dpinx = 0; dpinx < Lc.num_dpy; dpinx++)
      if( Dpy[dpinx].bufinx == bufinx && Dpy[dpinx].dpytype == DPYTYPE_IMAGE )
      {
         draw_xor_line_on_dpy( dpinx, xbeg, ybeg, xend, yend);
      }
}

/*----------------------------------------------------------------
**  draw_box_on_dpy() - Draw a box on the image canvas
**----------------------------------------------------------------
*/
void draw_box_on_dpy( 
   int dpinx,               /* Identifies canvas to draw on */
   float x,    float y,     /* Draw a box around these pixels  */
   float wid, float hgt, 
   int color                /* line foreground color */
)
{
   int      box_x, box_y, box_wid, box_hgt;
   float    pixel_wid;

   struct dpy_t *dp = &Dpy[dpinx];

   /* determine position of upper-left pixel */
   pixel_wid = dp->image_zoom > 0 ? dp->image_zoom : 1.0/(1.0+abs(dp->image_zoom));
   box_x = ((x - dp->image_offx) * pixel_wid) + 0.5;
   box_y = ((y - dp->image_offy) * pixel_wid) + 0.5;

   box_wid = wid * pixel_wid;
   box_hgt = hgt * pixel_wid;
   /* Adjust values so rectangle is draw around the pixels */
   box_x--; box_y--; box_wid++; box_hgt++;

   /* Draw it */
   gdk_gc_set_foreground( W.nor_gc, &CM.colors[color] );
   gdk_draw_rectangle( dp->data_drawingarea->window, W.nor_gc, FALSE, box_x, box_y, box_wid, box_hgt);
}

/*----------------------------------------------------------------
**  draw_line_on_dpy() - Draw a line on the canvas
**----------------------------------------------------------------
*/
void draw_line_on_dpy( 
   int dpinx,                 /* Identifies canvas to xor on         */
   float xbeg, float ybeg,    /* Draw a line from (x1,y1) to (x2,y2) */
   float xend, float yend, 
   int color                  /* line foreground color */
)
{
   int      x1, y1, x2, y2;
   float    pixel_wid;

   struct dpy_t *dp = &Dpy[dpinx];

   /* determine position of upper-left pixel */
   pixel_wid = dp->image_zoom > 0 ? dp->image_zoom : 1.0/(1.0+abs(dp->image_zoom));
   x1 = ((xbeg - dp->image_offx) * pixel_wid) + 0.5;
   y1 = ((ybeg - dp->image_offy) * pixel_wid) + 0.5;

   x2 = ((xend - dp->image_offx) * pixel_wid) + 0.5;
   y2 = ((yend - dp->image_offy) * pixel_wid) + 0.5;

   /* Draw it */
   gdk_gc_set_foreground( W.nor_gc, &CM.colors[color] );
   gdk_draw_line( dp->data_drawingarea->window, W.nor_gc, x1, y1, x2, y2);
}


/********************************************************************************/
/*  Text based drawing routines...                                               */
/********************************************************************************/

/*----------------------------------------------------------------------------
**   init_ds( ) - stores all info needed for our DS struct
**----------------------------------------------------------------------------
*/
struct ds_t * init_ds( 
   struct ds_t *ds,             // This ds_t struct gets initialized.
   GtkWidget *da,               // the drawing area widget.
   GdkGC  * gc,                 // the gc used.
#if USE_PANGO
   struct font_info_t *font,    // Pango font info   
#else
   GdkFont *font,               // non-pango font info
#endif
   int font_wid,                // font size
   int font_hgt                 // font size
)
{
   ds->window   = da->window;
   ds->gc       = gc;
   ds->font     = font;
	gdk_drawable_get_size( ds->window, &ds->da_wid, &ds->da_hgt );
   ds->char_wid = font_wid;
   ds->char_hgt = font_hgt;
   
   return ds;
}

/*----------------------------------------------------------------------------
**  draw_text( ) - draws text on a drawing area widget. 
**     x, y units in pixels (U_PIXEL) or Char_size (U_CHAR)
**----------------------------------------------------------------------------
*/
void draw_text( struct ds_t *ds, int units, float x, float y, char * fmt, ...)
{
   char buf[256];
   va_list argptr;

   va_start( argptr, fmt );
   vsprintf( buf, fmt, argptr );
   va_end( argptr );

#if USE_PANGO
   if( units == U_CHAR )
	{
      x = x*ds->char_wid; 
      y = y*ds->char_hgt; 
	}
   pango_layout_set_text ( ds->font->layout, buf, -1);
   gdk_draw_layout (ds->window, ds->gc, x, y, ds->font->layout);
#else
   if( units == U_CHAR )
	{
      x = x*ds->char_wid; 
      y = (y+1)*ds->char_hgt; 
	}
   gdk_draw_text( ds->window, ds->font, ds->gc, x, y, buf, strlen(buf));
#endif
}

/*----------------------------------------------------------------------------
**  draw_image_text( ) - draws text on a drawing area widget. 
**     x, y units in pixels (U_PIXEL) or Char_size (U_CHAR)
**----------------------------------------------------------------------------
*/
void draw_image_text( struct ds_t *ds, int units, float x, float y, char * fmt, ...)
{
   char buf[256];
   va_list argptr;

   va_start( argptr, fmt );
   vsprintf( buf, fmt, argptr );
   va_end( argptr );

#if USE_PANGO
   if( units == U_CHAR )
	{
      x = x*ds->char_wid; 
      y = y*ds->char_hgt; 
	}
   pango_layout_set_text ( ds->font->layout, buf, -1);
	gdk_draw_layout_with_colors (ds->window, ds->gc, x, y, ds->font->layout, ds->ifg, ds->ibg);
#else
   if( units == U_CHAR )
	{
      x = x*ds->char_wid; 
      y = (y+1)*ds->char_hgt; 
	}
   XDrawImageString( GDK_WINDOW_XDISPLAY(ds->window), GDK_WINDOW_XWINDOW(ds->window),
      GDK_GC_XGC(ds->gc), x, y, buf, strlen(buf));
#endif
}

/*----------------------------------------------------------------------------
**  drawD2len( ) - display a double adjusting it's resolution to fit inside len
**----------------------------------------------------------------------------
*/
void drawD2len( 
   struct ds_t *ds, 
   int units,          // U_CHAR or U_PIXEL
   float x,            // x and y to place text
   float y,
   double d,           // value to display 
   int len,            // max len of string: 3 to 15 
   int dec             // max dec for resolutions 
)
{
   char buf[40];

   double2str_len( buf, d, len, dec);
   draw_image_text( ds, units, x, y, buf );
}


/*----------------------------------------------------------------
**  double2str_len() converts double to a string show as much
**    percision as desired (dec), but limited to len.
**----------------------------------------------------------------
*/
void double2str_len( 
   char *buf,    /* put result here, must be at least buf[40] */
   double d,     /* value to display */
   int len,      /* max len of string */
   int dec )     /* max dec for resolutions */
{
   char format[40];
   double abs;
   int    s;

   abs = fabs(d);
   len = MAX(len, 3);
   len = MIN(len, 15);

   if( abs < 10.0 ) dec = len-3;
   else
   {
      s = log10(abs)+3;
      s = ceil(s);
      dec = len - s;
   }
   dec = MAX(0, dec);

   sprintf(format, "%%%d.%df ", len, dec);
   sprintf( buf, format, d);
}

/*----------------------------------------------------------------
** drawline() - Character coordinates based line drawer.
**----------------------------------------------------------------
*/
void draw_line( struct ds_t *ds, 
                int units,                          /* U_PIXEL or U_CHAR */
					 int x1, int y1, int x2, int y2 )   
{
   if( units == U_CHAR )
	{
		x1 = ds->char_wid * x1 + (ds->char_wid/2);
		x2 = ds->char_wid * x2 + (ds->char_wid/2);
		y1 = ds->char_hgt * y1 + (ds->char_hgt/2)+1;
		y2 = ds->char_hgt * y2 + (ds->char_hgt/2)+1;
	}

   gdk_draw_line( ds->window, ds->gc, x1, y1, x2, y2 );
}

/*----------------------------------------------------------------
** draw_box() - Character coordinates based box.
**----------------------------------------------------------------
*/
void draw_box( struct ds_t *ds, 
                int units,                          /* U_PIXEL or U_CHAR */
                int x1, int y1, int x2, int y2 )
{
   if( units == U_CHAR )
	{
		x1 = ds->char_wid * x1 + (ds->char_wid/2);
		x2 = ds->char_wid * x2 + (ds->char_wid/2);
		y1 = ds->char_hgt * y1 + (ds->char_hgt/2)+1;
		y2 = ds->char_hgt * y2 + (ds->char_hgt/2)+1;
	}

   gdk_draw_line( ds->window, ds->gc, x1, y1, x2, y1 );
   gdk_draw_line( ds->window, ds->gc, x1, y2, x2, y2 );
   gdk_draw_line( ds->window, ds->gc, x1, y1, x1, y2 );
   gdk_draw_line( ds->window, ds->gc, x2, y1, x2, y2 );
}

/*----------------------------------------------------------------
** clear_box() - clear a box
**----------------------------------------------------------------
*/
void clear_box( struct ds_t *ds, 
                int units,                          /* U_PIXEL or U_CHAR */
                int x1, int y1, int x2, int y2 )
{  
   if( units == U_CHAR )
	{
		x1 = ds->char_wid * x1;
		x2 = ds->char_wid * x2;
		y1 = ds->char_hgt * y1;
		y2 = ds->char_hgt * y2;
	}

   gdk_window_clear_area( ds->window, x1, y1,
                       x2-x1+ds->char_wid, y2-y1+ds->char_hgt);

}

/*----------------------------------------------------------------
** da_set_foreground/background() - sets the 
**    foreground/background color for gdk_draw_ & pango stuff.
**----------------------------------------------------------------
*/
void da_set_foreground( struct ds_t *ds, int fg )
{
   gdk_gc_set_foreground( ds->gc, &CM.colors[fg] );
	if( ds )
		ds->ifg = &CM.colors[fg];
}
void da_set_background( struct ds_t *ds, int bg )
{
   gdk_gc_set_background( ds->gc, &CM.colors[bg] );
	if( ds )
		ds->ibg = &CM.colors[bg];
}


/********************************************************************************/
