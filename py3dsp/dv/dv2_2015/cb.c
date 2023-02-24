/**************************************************************************
**  cb.c - general callbacks & other handlers for GUI widget and stuff
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
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <semaphore.h>
#include <mqueue.h>
#include <dirent.h>

#include <gdk/gdkx.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>


/*-----------------------------
**  Non-standard include files
**-----------------------------
*/

#include "dv.h"

/************************************************************************
**  base window/application related call backs.
*************************************************************************
*/

/*-------------------------------------------------------------------------------
**  base_delete_event() - base_window cb for "delete_event" signal from window manager.
**-------------------------------------------------------------------------------
*/
int base_delete_event( GtkWidget *widget, GdkEvent  *event, gpointer data )
{
   /* Return FALSE - gtk will call base_destroy_cb();
   **         TRUE - gtk will ignore delete event.
   */
   return(FALSE);
}

/*-------------------------------------------------------------------------------
**  base_destroy_cb() - base_window cb for "destroy" signal. Quits the applications.
**-------------------------------------------------------------------------------
*/
void base_destroy_cb( GtkWidget *widget, gpointer data )
{
   gtk_main_quit();
}


/************************************************************************
**  some standard call backs...
*************************************************************************
*/

/*----------------------------------------------------------------------------
**  widget_hide_cb() - dialog hide callback.
**----------------------------------------------------------------------------
*/
void widget_hide_cb( GtkWidget *widget, gpointer data )
{
   gtk_widget_hide( (GtkWidget*)data );
}

/*----------------------------------------------------------------------------
**  button_cb() - standard button cb. Call cmd_execute( data )
**----------------------------------------------------------------------------
*/
void button_cb( GtkWidget *widget, gpointer data )
{
   cmd_execute( W.cmd.main_console, (char*)data, FALSE);
}

/*-------------------------------------------------------------------------------
**  button2_cb() - another generic pushbutton callback, adds Lc.active as parameter
**-------------------------------------------------------------------------------
*/
void button2_cb( GtkWidget *widget, gpointer data )
{
   char buf[60];

   sprintf( buf, "%s %d ", (char*)data, Lc.active);
   cmd_execute( W.cmd.main_console, buf, TRUE);
}

/*-------------------------------------------------------------------------------
**  adj_cb() - standard adjustment cb.
**-------------------------------------------------------------------------------
*/
void adj_cb( GtkAdjustment *adj, gpointer data )
{
   char buf[50];
   sprintf( buf, "%s %f ", (char*)data, adj->value);
   cmd_execute( W.cmd.main_console, buf, TRUE);
}
/*-------------------------------------------------------------------------------
**  entry_cb() - standard entry cb. Call cmd_execute( data )
**  entry_fo_cb() - entry focus out event cb. Call the entry_cb.
**-------------------------------------------------------------------------------
*/
void entry_cb( GtkEntry *w, gpointer data )
{
   char buf[80];
   sprintf( buf, "%s %s ", (char*)data, (char*)gtk_entry_get_text(w) );
   cmd_execute( W.cmd.main_console, buf, TRUE);
}
gint entry_fo_cb( GtkEntry *widget, GdkEventFocus * event, gpointer data )
{
   entry_cb( widget, data );
	return FALSE;
}

/*-------------------------------------------------------------------------------
**  toggle_button_cb() - toggle button cb. Call cmd_execute( data ) when
**                       the widget is pressed.
**-------------------------------------------------------------------------------
*/
void toggle_button_cb( GtkToggleButton *widget, gpointer data )
{  
   if( widget->active )
		cmd_execute( W.cmd.main_console, data, TRUE);
}

/*---------------------------------------------------------------------------
**  checkbutton_offon_cb() - generic call back of check button (using offon)
**---------------------------------------------------------------------------
*/
void checkbutton_offon_cb( GtkToggleButton *widget, gpointer data )
{
   char buf[80];
                                                                                                           
   char *keyword = (char *) data;
   int  state = gtk_toggle_button_get_active( widget );
                                                                                                           
   sprintf( buf, "%s %s ", keyword, offon_selection[(int)state] );
   cmd_execute( W.cmd.main_console, buf, TRUE);
}
   
/*-------------------------------------------------------------------------------
**  dialog_delete_event() - doen't delete dialog box, just hides it.
**-------------------------------------------------------------------------------
*/
int dialog_delete_event( GtkWidget *widget, GdkEvent  *event, gpointer data )
{  
   /* Return FALSE - gtk will call base_destroy_cb();
   **         TRUE - gtk will ignore delete event.
   */
   gtk_widget_hide( GTK_WIDGET(widget) );

   return(TRUE);
}


/************************************************************************
**  main menu bar call backs...
*************************************************************************
*/

/*---------------------------------------------------------------------------
**  menubar_open_cb() - calls to file open dialog.
**---------------------------------------------------------------------------
*/
void menubar_open_cb( GtkWidget *widget, gpointer data )
{
	gtk_widget_show( W.file_open_dialog );
	gdk_window_raise( W.file_open_dialog->window );
}

/*---------------------------------------------------------------------------
**  menubar_save_cb() - calls to file save dialog.
**---------------------------------------------------------------------------
*/
void menubar_save_cb( GtkWidget *widget, gpointer data )
{
	gtk_widget_show( W.file_save_dialog );
	gdk_window_raise( W.file_save_dialog->window );
}

/*---------------------------------------------------------------------------
**  menubar_configure_cb() - calls the configure dialog.
**---------------------------------------------------------------------------
*/
void menubar_configure_cb( GtkWidget *widget, gpointer data )
{
   static int firsttime = TRUE;

	if( firsttime )
	{
	   gtk_window_set_position( GTK_WINDOW(W.configure_dialog_window), GTK_WIN_POS_MOUSE);
		firsttime = FALSE;
	}

	gtk_widget_show( W.configure_dialog_window );
	gdk_window_raise( W.configure_dialog_window->window );
}

/*---------------------------------------------------------------------------
**  menubar_quit_cb() - calls the macro dialog.
**---------------------------------------------------------------------------
*/
void menubar_quit_cb( GtkWidget *widget, gpointer data )
{
   cmd_execute(  W.cmd.main_console, "quit", TRUE );
}

/*---------------------------------------------------------------------------
**  colormap_cbox_cb() - combo box callback
**---------------------------------------------------------------------------
*/
void colormap_cbox_cb( GtkWidget *widget, gpointer data )
{
	char buf[80];
	int  state = gtk_combo_box_get_active( GTK_COMBO_BOX(widget) );

	sprintf( buf, "ColorMap %s ", colormap_selection[(int)state] );
	cmd_execute(  W.cmd.main_console, buf, TRUE );
}

/************************************************************************
**  Display Options callbacks
*************************************************************************
*/

/*-------------------------------------------------------------------------------
**  dpyactive_cb() - catches toggle button ->button pressed event.
**-------------------------------------------------------------------------------
*/
void dpyactive_cb( GtkToggleButton *widget, gpointer data )
{
   char buf[40];
   int  dpinx = (intptr_t)data;
   /* if( widget->active ) */
   {
      sprintf( buf, "Active %d ", dpinx);
      cmd_execute(  W.cmd.main_console, buf, TRUE );
   }

   if( dpinx == (Lc.num_dpy-1))
   {
      gtk_widget_show( W.dpy_dialog_window );
      gdk_window_raise( W.dpy_dialog_window->window );
   }

}
   
/*-------------------------------------------------------------------------------
**  dpytype_cb() - cbox for dpy selection 
**-------------------------------------------------------------------------------
*/
void dpytype_cb( GtkWidget * widget, gpointer data )
{
	char buf[80];
	int  state = gtk_combo_box_get_active( GTK_COMBO_BOX(widget) );

	sprintf( buf, "DisplayType %s %d ", dpytype_selection[(int)state], Lc.active );
	cmd_execute(  W.cmd.main_console, buf, TRUE );
}

/*-------------------------------------------------------------------------------
**  buffer_cb() - toggle button cb for buffer buttons.
**-------------------------------------------------------------------------------
*/
void buffer_cb( GtkToggleButton *widget, gpointer data )
{  
   char buf[40];
   int  bufid = (intptr_t)data;
   if( widget->active )
   {
      sprintf( buf, "Buffer %c %d ", bufid, Lc.active);
      cmd_execute(  W.cmd.main_console, buf, TRUE );
   }
}


/*********************** displaytype IMAGE CB callbacks *************************/

/*-------------------------------------------------------------------------------
**  image_autoscale_cb() - radio button cb
**-------------------------------------------------------------------------------
*/
void image_autoscale_cb( GtkWidget *widget, gpointer data )
{
   char buf[40];
	int inx = (intptr_t)data;

   sprintf( buf, "ImageAutoScale %s %d ", imageautoscale_selection[inx], Lc.active);
   cmd_execute(  W.cmd.main_console, buf, TRUE );
}

/*-------------------------------------------------------------------------------
**  image_zoom_cb() - standard adjustment cb.
**-------------------------------------------------------------------------------
*/
void image_zoom_cb( GtkAdjustment *adj, gpointer data )
{
   char buf[50];
   int zoom = rint(adj->value);
	char *s = data;
   sprintf( buf, "%s %d ", s, zoom );
   cmd_execute(  W.cmd.main_console, buf, TRUE );
}

/*-------------------------------------------------------------------------------
**  image_range_cb() - entry cb  for image range min & max
**-------------------------------------------------------------------------------
*/
void image_range_cb( GtkEntry *w, gpointer data )
{
   float min, max;
   char buf[60];
	char *str_ptr;

   strxcpy( buf, gtk_entry_get_text( GTK_ENTRY(W.image_range_min) ), sizeof(buf));
   if( parseFloat_r( &min, buf, " ", &str_ptr) != ERR_NONE )
      return;

   strxcpy( buf, gtk_entry_get_text( GTK_ENTRY(W.image_range_max) ), sizeof(buf));
   if( parseFloat_r( &max, buf, " ", &str_ptr) != ERR_NONE )
      return;

   if( min < max )
   {
      sprintf( buf, "ImageRange %4.2f %4.2f ", min, max );
      cmd_execute(  W.cmd.main_console, buf, TRUE );
   }
}
gint image_range_fo_cb( GtkEntry *widget, GdkEventFocus * event, gpointer data )
{
   image_range_cb( widget, data );
	return FALSE;
}

/*-------------------------------------------------------------------------------
**  inst_subarray_cb() - set subarray command to Instrument.
**-------------------------------------------------------------------------------
*/
void inst_subarray_cb( GtkWidget *widget, gpointer data )
{
   int bufinx,
       ainx,
       x, y, wid, hgt;
   char buf[80];

   bufinx = Dpy[Lc.active].bufinx;
   x   = Stats[bufinx].objx;
   y   = Stats[bufinx].objy;
   wid = Stats[bufinx].objwid;
   hgt = Stats[bufinx].objhgt;
   ainx = (intptr_t)data;

   if( INRANGE(0, ainx, 2))
   {
      if( (Lc.inst_flavor == INST_FLAVOR_SMOKEY) ) 
		{
			//if( Lc.inst_flavor == INST_FLAVOR_SPEX )
		   //   spex_fix_subarray_dim( &x, &y, &wid, &hgt);

			sprintf( buf, "Inst.Com Array %d %d %d %d %d ", ainx, x, y, wid, hgt);
			cmd_execute(  W.cmd.main_console, buf, TRUE );
		}
		else if( ( (Lc.inst_flavor==INST_FLAVOR_MORIS) && (ainx==0) ) ||
		         ( (Lc.inst_flavor==INST_FLAVOR_BIGDOG) && (ainx==0) ) ||
		         ( (Lc.inst_flavor==INST_FLAVOR_GUIDEDOG) && (ainx==0) ) ) 
		{
			sprintf( buf, "Inst.Com Array %d %d %d %d ", x, y, wid, hgt);
			cmd_execute(  W.cmd.main_console, buf, TRUE );
		}
   }
	else
	{
      /* Set the Guide Array  */
		if( Lc.inst_flavor == INST_FLAVOR_GUIDEDOG )
		{
			sprintf( buf, "Inst.Com GuideBox %c %d %d %d %d ", 
				(ainx==3?'A':'B'), x, y, wid, hgt);
			cmd_execute(  W.cmd.main_console, buf, TRUE );
		}
      else if( (Lc.inst_flavor == INST_FLAVOR_SMOKEY ) ||
               (Lc.inst_flavor == INST_FLAVOR_MORIS) )
		{
			sprintf( buf, "Inst.Com GuideBox %d %d %d %d ", x, y, wid, hgt);
			cmd_execute(  W.cmd.main_console, buf, TRUE );
		}
	}

}

/********************* displaytype HISTOGRAM CB callbacks ***********************/


/*-------------------------------------------------------------------------------
**  hist_area_cb() - radio button cb
**-------------------------------------------------------------------------------
*/
void hist_area_cb( GtkToggleButton *widget, gpointer data )
{
   char buf[40];
	int inx = (intptr_t)data;

   sprintf( buf, "HistArea %s %d ", allbox_selection[inx], Lc.active );
   cmd_execute(  W.cmd.main_console, buf, TRUE );
}

/*-------------------------------------------------------------------------------
**  hist_range_cb() - entry cb  for histogram range min & max
**-------------------------------------------------------------------------------
*/
void hist_range_cb( GtkEntry *w, gpointer data )
{
   int min, max;
   char buf[60];
	char *str_ptr;

   strxcpy( buf, gtk_entry_get_text( GTK_ENTRY(W.hist_range_min) ), sizeof(buf));
   if( parseInt_r( &min, buf, " ", &str_ptr) != ERR_NONE )
      return;

   strxcpy( buf, gtk_entry_get_text( GTK_ENTRY(W.hist_range_max) ), sizeof(buf));
   if( parseInt_r( &max, buf, " ", &str_ptr) != ERR_NONE )
      return;

   if( min < max )
   {
      sprintf( buf, "ImageRange %d %d ", min, max );
      cmd_execute(  W.cmd.main_console, buf, TRUE );
   }
}

gint hist_range_fo_cb( GtkEntry *widget, GdkEventFocus * event, gpointer data )
{
   hist_range_cb( widget, data );
	return FALSE;
}

/********************** displaytype LINECUT CB callbacks ************************/

/*-------------------------------------------------------------------------------
**  lcut_area_cb() - radio button cb
**-------------------------------------------------------------------------------
*/
void lcut_area_cb( GtkToggleButton *widget, gpointer data )
{
   char buf[40];
	int inx = (intptr_t)data;

   sprintf( buf, "LCutArea %s %d ", allbox_selection[inx], Lc.active );
   cmd_execute(  W.cmd.main_console, buf, TRUE );
}

/*-------------------------------------------------------------------------------
**  lcut_autoscale_cb() - radio button cb
**-------------------------------------------------------------------------------
*/
void lcut_autoscale_cb( GtkToggleButton *widget, gpointer data )
{
   char buf[40];
	int inx = (intptr_t)data;

   sprintf( buf, "LCutAutoScale %s %d ", imageautoscale_selection[inx], Lc.active);
   cmd_execute(  W.cmd.main_console, buf, TRUE );
}

/*-------------------------------------------------------------------------------
**  lcut_range_cb() - entry cb  for image range min & max
**  lcut_range_fo_cb() - entry focus out cb for image range min & max
**-------------------------------------------------------------------------------
*/
void lcut_range_cb( GtkEntry *w, gpointer data )
{
   int min, max;
   char buf[60];
	char *str_ptr;

   strxcpy( buf, gtk_entry_get_text( GTK_ENTRY(W.lcut_range_min) ), sizeof(buf));
   if( parseInt_r( &min, buf, " ", &str_ptr) != ERR_NONE )
      return;

   strxcpy( buf, gtk_entry_get_text( GTK_ENTRY(W.lcut_range_max) ), sizeof(buf));
   if( parseInt_r( &max, buf, " ", &str_ptr) != ERR_NONE )
      return;

   if( min < max )
   {
      sprintf( buf, "LCutRange %d %d ", min, max );
      cmd_execute(  W.cmd.main_console, buf, TRUE );
   }
}

gint lcut_range_fo_cb( GtkEntry *widget, GdkEventFocus * event, gpointer data )
{
   lcut_range_cb( widget, data );
	return FALSE;
}

/*-------------------------------------------------------------------------------
**   lcut_xy_cb()  - entry widget cb for LCutXY command.
**   lcut_xy_fo_cb()  - entry widget focus-out cb.
**-------------------------------------------------------------------------------
*/
void lcut_xy_cb( GtkEntry *w, gpointer data )
{
   int x, y;
   char buf[60];
	char * str_ptr;

   strxcpy( buf, gtk_entry_get_text( GTK_ENTRY(W.lcut_x) ), sizeof(buf));
   if( parseInt_r( &x, buf, " ", &str_ptr) != ERR_NONE )
      return;

   strxcpy( buf, gtk_entry_get_text( GTK_ENTRY(W.lcut_y) ), sizeof(buf));
   if( parseInt_r( &y, buf, " ", &str_ptr) != ERR_NONE )
      return;

   sprintf( buf, "LCutXY %.10d %.10d ", x, y );
   cmd_execute(  W.cmd.main_console, buf, TRUE );
}

gint lcut_xy_fo_cb( GtkEntry *widget, GdkEventFocus * event, gpointer data )
{
   lcut_xy_cb( widget, data );
	return FALSE;
}

/*-------------------------------------------------------------------------------
**   lcut_xarrow()  - Issues LCutXY command increment/decrement the x axis
**-------------------------------------------------------------------------------
*/
void lcut_xarrow( GtkWidget *w, gpointer data )
{
   char buf[60];
	int inc = (intptr_t)data;

   sprintf( buf, "LCutXY %d %d ", Dpy[Lc.active].lcut_x+inc, Dpy[Lc.active].lcut_y);
   cmd_execute(  W.cmd.main_console, buf, TRUE );
}

/*-------------------------------------------------------------------------------
**   lcut_yarrow()  - Issues LCutXY command increment/decrement the y axis
**-------------------------------------------------------------------------------
*/
void lcut_yarrow( GtkWidget *w, gpointer data )
{
   char buf[60];
	int inc = (intptr_t)data;

   sprintf( buf, "LCutXY %d %d ", Dpy[Lc.active].lcut_x, Dpy[Lc.active].lcut_y+inc);
   cmd_execute(  W.cmd.main_console, buf, TRUE );
}

/*********************** displaytype NOISE CB callbacks *************************/

/*-------------------------------------------------------------------------------
**   noise_g1range_cb()  - Issues NoiseG1Range command.
**-------------------------------------------------------------------------------
*/
void noise_g1range_cb( GtkEntry *w, gpointer data )
{
   int min, max;
   char buf[60];
	char * str_ptr;

   strxcpy( buf, gtk_entry_get_text( GTK_ENTRY(W.noise_g1_range_min) ), sizeof(buf));
   if( parseInt_r( &min, buf, " ", &str_ptr) != ERR_NONE )
      return;

   strxcpy( buf, gtk_entry_get_text( GTK_ENTRY(W.noise_g1_range_max) ), sizeof(buf));
   if( parseInt_r( &max, buf, " ", &str_ptr) != ERR_NONE )
      return;

   if( min < max )
   {
      sprintf( buf, "NoiseG1Range %d %d ", min, max );
      cmd_execute(  W.cmd.main_console, buf, TRUE );
   }
}

gint noise_g1range_fo_cb( GtkEntry *widget, GdkEventFocus * event, gpointer data )
{
   noise_g1range_cb( widget, data );
	return FALSE;
}

/*-------------------------------------------------------------------------------
**   noise_g2range_cb()  - Issues NoiseG2Range command.
**-------------------------------------------------------------------------------
*/
void noise_g2range_cb( GtkEntry *w, gpointer data )
{
   int min, max;
   char buf[60];
	char *str_ptr;

   strxcpy( buf, gtk_entry_get_text( GTK_ENTRY(W.noise_g2_range_min) ), sizeof(buf));
   if( parseInt_r( &min, buf, " ", &str_ptr) != ERR_NONE )
      return;

   strxcpy( buf, gtk_entry_get_text( GTK_ENTRY(W.noise_g2_range_max) ), sizeof(buf));
   if( parseInt_r( &max, buf, " ", &str_ptr) != ERR_NONE )
      return;

   if( min < max )
   {
      sprintf( buf, "NoiseG2Range %d %d ", min, max );
      cmd_execute(  W.cmd.main_console, buf, TRUE );
   }
}

gint noise_g2range_fo_cb( GtkEntry *widget, GdkEventFocus * event, gpointer data )
{
   noise_g2range_cb( widget, data );
	return FALSE;
}

/*-------------------------------------------------------------------------------
**   noise_autoscale_cb()  - Issues NoiseAutoScale command.
**-------------------------------------------------------------------------------
*/
void noise_autoscale_cb( GtkToggleButton *widget, gpointer data )
{
   char buf[40];
	int  inx = (intptr_t)data;

   sprintf( buf, "NoiseAutoScale %s %d ", imageautoscale_selection[inx], Lc.active);
   cmd_execute(  W.cmd.main_console, buf, TRUE );
}

/*-------------------------------------------------------------------------------
**   noise_graphtype_cb()  - Issues NoiseGraphType command.
**-------------------------------------------------------------------------------
*/
void noise_graphtype_cb( GtkToggleButton *widget, gpointer data )
{
   char buf[40];
	int  inx = (intptr_t)data;

   sprintf( buf, "NoiseGraphType %s %d ", noisegraphtype_selection[inx], Lc.active );
   cmd_execute(  W.cmd.main_console, buf, TRUE );
}

/*-------------------------------------------------------------------------------
**   noise_area_cb()  - Issues NoiseArea command.
**-------------------------------------------------------------------------------
*/
void noise_area_cb( GtkToggleButton *widget, gpointer data )
{
   char buf[40];
	int  inx = (intptr_t)data;

   sprintf( buf, "NoiseArea %s %d ", allbox_selection[inx], Lc.active );
   cmd_execute(  W.cmd.main_console, buf, TRUE );
}

/*-------------------------------------------------------------------------------
**   noise_mode_cb()  - Issues NoiseArea command.
**-------------------------------------------------------------------------------
*/
void noise_mode_cb( GtkToggleButton *widget, gpointer data )
{
   char buf[40];
	int  inx = (intptr_t)data;

   sprintf( buf, "NoiseMode %s %d ", noise_mode_selection[inx], Lc.active );
   cmd_execute(  W.cmd.main_console, buf, TRUE );
}

/********************** displaytype XLINECUT CB callbacks ***********************/

/*-------------------------------------------------------------------------------
**   xcut_autoscale_cb()  - Issues XcutAutoScale command.
**-------------------------------------------------------------------------------
*/
void xcut_autoscale_cb( GtkToggleButton *widget, gpointer data )
{
   char buf[40];
	int  inx = (intptr_t)data;

   sprintf( buf, "XCutAutoScale %s %d ", imageautoscale_selection[inx], Lc.active);
   cmd_execute(  W.cmd.main_console, buf, TRUE );
}

/*-------------------------------------------------------------------------------
**   xcut_range_cb()  - Issues XCutRange command.
**-------------------------------------------------------------------------------
*/
void xcut_range_cb( GtkEntry *w, gpointer data )
{
   int min, max;
   char buf[60];
	char *str_ptr;

   strxcpy( buf, gtk_entry_get_text( GTK_ENTRY(W.xcut_range_min) ), sizeof(buf));
   if( parseInt_r( &min, buf, " ", &str_ptr) != ERR_NONE )
      return;

   strxcpy( buf, gtk_entry_get_text( GTK_ENTRY(W.xcut_range_max) ), sizeof(buf));
   if( parseInt_r( &max, buf, " ", &str_ptr) != ERR_NONE )
      return;

   if( min < max )
   {
      sprintf( buf, "XCutRange %d %d ", min, max );
      cmd_execute(  W.cmd.main_console, buf, TRUE );
   }
}

gint xcut_range_fo_cb( GtkEntry *widget, GdkEventFocus * event, gpointer data )
{
   lcut_range_cb( widget, data );
	return FALSE;
}

/*-------------------------------------------------------------------------------
**   xcut_set_cb()  - Issues XCutSet command.
**   xcut_set_fo_cb() - focus out entry widget callback.
**-------------------------------------------------------------------------------
*/
void xcut_set_cb( GtkEntry *w, gpointer data )
{
   int xbeg, xend, ybeg, yend;
   char buf[60];
	char *str_ptr;

   strxcpy( buf, gtk_entry_get_text( GTK_ENTRY(W.xcut_beg) ), sizeof(buf));
   if( (parseInt_r( &xbeg, buf, " ", &str_ptr) != ERR_NONE) )
      return;
   if( (parseInt_r( &ybeg, NULL, " ", &str_ptr) != ERR_NONE) )
      return;

   strxcpy( buf, gtk_entry_get_text( GTK_ENTRY(W.xcut_end) ), sizeof(buf));
   if( (parseInt_r( &xend, buf, " ", &str_ptr) != ERR_NONE) )
      return;
   if( (parseInt_r( &yend, NULL, " ", &str_ptr) != ERR_NONE) )
      return;

   sprintf( buf, "XCutSet %d %d %d %d ", xbeg, ybeg, xend, yend );
   cmd_execute(  W.cmd.main_console, buf, TRUE );
}
gint xcut_set_fo_cb( GtkEntry *widget, GdkEventFocus * event, gpointer data )
{
   xcut_set_cb( widget, data );
	return FALSE;
}

/*-------------------------------------------------------------------------------
**   xcut_setline_cb()  - Issues XCutSet command.
**-------------------------------------------------------------------------------
*/
void xcut_setline_cb( GtkWidget *widget, gpointer data )
{
   char buf[60];
   int bufinx;

   bufinx = Dpy[Lc.active].bufinx;

   sprintf( buf, "XCutSet %d %d %d %d ", Stats[bufinx].ln_xbeg, Stats[bufinx].ln_ybeg,
                                         Stats[bufinx].ln_xend, Stats[bufinx].ln_yend);
   cmd_execute(  W.cmd.main_console, buf, TRUE );
}

/*-------------------------------------------------------------------------------
**   xcut_output_data_cb()  - Set flag and redraws xcut so the data is
**        written to an external file.
**-------------------------------------------------------------------------------
*/
void xcut_output_data_cb( GtkWidget *widget, gpointer data )
{
   if( Dpy[Lc.active].dpytype == DPYTYPE_XLINECUT )
   {
      Dpy[Lc.active].xcut_output_data = TRUE;
      call_dpydata_redraw( Lc.active );           /* redraw canvas */
   }
}

/*-------------------------------------------------------------------------------
**   xcut_fit_data_cb()  - Set flag and redraws xcut so the data is
**        fitted to a guassian equation.
**-------------------------------------------------------------------------------
*/
void xcut_fit_data_cb( GtkWidget *widget, gpointer data )
{
   if( Dpy[Lc.active].dpytype == DPYTYPE_XLINECUT )
   {
      Dpy[Lc.active].xcut_fit_data = TRUE;
      call_dpydata_redraw( Lc.active );           /* redraw canvas */
   }
}

/********************** displaytype STATS CB callbacks **************************/

/*-------------------------------------------------------------------------------
**  stats_setsky_cb - button callback.
**-------------------------------------------------------------------------------
*/
void stats_setsky_cb( GtkWidget *widget, gpointer data )
{
   char buf[40];
   int bufid;

   bufid = 'A'+Dpy[Lc.active].bufinx;

   sprintf( buf, "StatsSetSky %c ", bufid );
   cmd_execute(  W.cmd.main_console, buf, TRUE );
}

/*-------------------------------------------------------------------------------
**  stats_fixedwh_cb() - check_button_cb
**-------------------------------------------------------------------------------
*/
void stats_fixedwh_cb( GtkWidget *widget, gpointer data )
{
   int bufid, offon;
   char buf[60];

   bufid = 'A'+Dpy[Lc.active].bufinx;

   offon = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(widget) );
   sprintf( buf, "StatsFixedWH %c %s ", bufid, offon_selection[offon]);
   cmd_execute(  W.cmd.main_console, buf, TRUE );
}

/*-------------------------------------------------------------------------------
**  stats_wh_entry_cb() - entry cb for fixed WH entry.
**-------------------------------------------------------------------------------
*/
void stats_wh_entry_cb( GtkWidget *widget, gpointer data )
{
   int offon,
       bufid;
   char buf[60];

   bufid = 'A'+Dpy[Lc.active].bufinx;

   offon = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(W.stats_fixedwh) );

   sprintf( buf, "StatsFixedWH %c %s %s ", bufid, offon_selection[offon],
       gtk_entry_get_text(GTK_ENTRY(widget)) );
   cmd_execute(  W.cmd.main_console, buf, TRUE );
}
gint stats_wh_entry_fo_cb( GtkWidget *widget, GdkEventFocus * event, gpointer data )
{
	stats_wh_entry_cb( widget, data );
	return FALSE;
}

/********************** displaytype AOFIG CB callbacks **************************/

/*-------------------------------------------------------------------------------
**  aofig_format_cb() - call back for for menu entries.
**-------------------------------------------------------------------------------
*/
void aofig_format_cb ( GtkWidget *widget, gpointer data )
{
	char buf[50];
	int  inx = gtk_combo_box_get_active( GTK_COMBO_BOX(widget) );

	sprintf( buf, "AOFig.Format %s ", aofig_format_selection[inx] );
	cmd_execute(  W.cmd.main_console, buf, TRUE );
}

/*-------------------------------------------------------------------------------
**  aofig_data_cb() - call back for for menu entries.
**-------------------------------------------------------------------------------
*/
void aofig_data_cb ( GtkWidget *widget, gpointer data )
{
	char buf[50];
	int  inx = gtk_combo_box_get_active( GTK_COMBO_BOX(widget) );

	sprintf( buf, "AOFig.Data %s ", xy_selection[inx] );
	cmd_execute(  W.cmd.main_console, buf, TRUE );
}

/*----------------------------------------------------------------------------
**  aofig_XY_cb() - adjustment cb 
**----------------------------------------------------------------------------
*/
void aofig_XY_cb (GtkAdjustment *adj, gpointer data )
{
   int x, y;
   char buf[80];

   x = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(W.aofig_x));
   y = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(W.aofig_y));

   sprintf( buf, "aofig.XY %d %d ", x, y);
   cmd_execute(  W.cmd.main_console, buf, TRUE );
}

/********************* displaytype SpectraA CB callbacks ************************/

/*-------------------------------------------------------------------------------
**  sa_setobjbin_cb() - sets the columns for the objbin using the object Box.
**-------------------------------------------------------------------------------
*/
void sa_setobjbin_cb( GtkWidget *widget, gpointer data )
{
   int bufinx,
	    y, hgt;
   char buf[80];

   bufinx = Dpy[Lc.active].bufinx;
   y   = Stats[bufinx].objy;
   hgt = Stats[bufinx].objhgt;

   sprintf( buf, "SAObjBin %d %d ", y, y+hgt-1);
   cmd_execute(  W.cmd.main_console, buf, TRUE );
}

/*-------------------------------------------------------------------------------
**  sa_setskybin_cb() - sets the columns for the skybin using the Object Box.
**-------------------------------------------------------------------------------
*/
void sa_setskybin_cb( GtkWidget *widget, gpointer data )
{
   int bufinx,
	    y, hgt;
   char buf[80];

   bufinx = Dpy[Lc.active].bufinx;
   y   = Stats[bufinx].objy;
   hgt = Stats[bufinx].objhgt;

   sprintf( buf, "SASkyBin %d %d ", y, y+hgt-1);
   cmd_execute(  W.cmd.main_console, buf, TRUE );
}

/*-------------------------------------------------------------------------------
**  sa_yautoscale_cb() - call back for for menu entries.
**-------------------------------------------------------------------------------
*/
void sa_yautoscale_cb ( GtkMenuItem * menuitem, gpointer data )
{
   char buf[50];
	int  inx = (intptr_t)data;

   sprintf( buf, "SAYAutoScale %s ", sa_yautoscale_selection[inx] );
   cmd_execute(  W.cmd.main_console, buf, TRUE );
}

/*-------------------------------------------------------------------------------
**  sa_setxscale_cb() - sets the columns for the objbin using the object Box.
**-------------------------------------------------------------------------------
*/
void sa_setxscale_cb( GtkWidget *widget, gpointer data )
{
   int bufinx,
	    x, wid;
   char buf[80];

   bufinx = Dpy[Lc.active].bufinx;
   x   = Stats[bufinx].objx;
   wid = Stats[bufinx].objwid;

   sprintf( buf, "SAXScale %d %d ", x, x+wid-1);
   cmd_execute(  W.cmd.main_console, buf, TRUE );
}

/********************* displaytype SpectraB CB callbacks ************************/

/*-------------------------------------------------------------------------------
**  sb_setobjbin_cb() - sets the columns for the objbin using the object Box.
**-------------------------------------------------------------------------------
*/
void sb_setobjbin_cb( GtkWidget *widget, gpointer data )
{
   int bufinx,
	    y, hgt;
   char buf[80];

   bufinx = Dpy[Lc.active].bufinx;
   y   = Stats[bufinx].objy;
   hgt = Stats[bufinx].objhgt;

   sprintf( buf, "SBObjBin %d %d ", y, y+hgt-1);
   cmd_execute(  W.cmd.main_console, buf, TRUE );
}

/*-------------------------------------------------------------------------------
**  sb_setskybin_cb() - sets the columns for the skybin using the Object Box.
**-------------------------------------------------------------------------------
*/
void sb_setskybin_cb( GtkWidget *widget, gpointer data )
{
   int bufinx,
	    y, hgt;
   char buf[80];

   bufinx = Dpy[Lc.active].bufinx;
   y   = Stats[bufinx].objy;
   hgt = Stats[bufinx].objhgt;

   sprintf( buf, "SBSkyBin %d %d ", y, y+hgt-1);
   cmd_execute(  W.cmd.main_console, buf, TRUE );
}

/*---------------------------------------------------------------------------
**  sb_show_cb() - check button callback 
**---------------------------------------------------------------------------
*/
void sb_show_cb( GtkToggleButton *widget, gpointer data )
{
   char buf[50];
   char mask_str[20];

   strcpy( mask_str, "");
   if( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(W.sb_show_diff) ) )
	   strcat( mask_str, "D");
   if( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(W.sb_show_obj) ) )
	   strcat( mask_str, "O");
   if( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(W.sb_show_sky) ) )
	   strcat( mask_str, "S");

   sprintf( buf, "SBShow %s ", mask_str );
   cmd_execute(  W.cmd.main_console, buf, TRUE );
}

/*-------------------------------------------------------------------------------
**  sb_setxscale_cb() - sets the columns for the objbin using the object Box.
**-------------------------------------------------------------------------------
*/
void sb_setxscale_cb( GtkWidget *widget, gpointer data )
{
   int bufinx,
	    x, wid;
   char buf[80];

   bufinx = Dpy[Lc.active].bufinx;
   x   = Stats[bufinx].objx;
   wid = Stats[bufinx].objwid;

   sprintf( buf, "SBXScale %d %d ", x, x+wid-1);
   cmd_execute(  W.cmd.main_console, buf, TRUE );
}
/********************************************************************************/
/*  Macro Dialog window callbacks                                               */
/********************************************************************************/

/*-------------------------------------------------------------------------------
**   m_execute_cb() - Call back for the execute button.
**        run file in buffer.
**-------------------------------------------------------------------------------
*/
void m_execute_cb( GtkWidget *widget, gpointer data )
{
   /* Get file name from list */
   GtkTreeSelection  *selection;
   GtkTreeIter       iter;
   GtkTreeModel      *model = GTK_TREE_MODEL(Md.file_store);
   char              cmd[256];
   char              filename[256];
    
   /* Get the filename from the list widget if there is a selection */
   selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(Md.file_list));
   if(gtk_tree_selection_get_selected( selection, &model, &iter))
   {
      gchar *str_data;
      gtk_tree_model_get( model, &iter, 0, &str_data, -1 );

      /* Create whole path of the file */
      cat_pathname( filename, Md.path, str_data, sizeof(filename) );

      /* Create the "M.Execute" command and execute it on the file */
      sprintf(cmd, "M.Execute %s", filename);
		cmd_execute(  W.cmd.main_console, cmd, TRUE );

      g_free( str_data );
   }
}

/*-------------------------------------------------------------------------------
**   m_filelist_cb() - when the user selects a filename, lets load the text.
**-------------------------------------------------------------------------------
*/
void m_filelist_cb( GtkWidget *widget, gint row, gint col, gpointer data )
{
   /* Get file name from list */
   GtkTreeSelection  *selection;
   GtkTreeIter       iter;
   GtkTreeModel      *model = GTK_TREE_MODEL(Md.file_store);
   char              cmd[256];
   char              filename[256];
    
   /* Get the filename from the list widget if there is a selection */
   selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(Md.file_list));
   if(gtk_tree_selection_get_selected( selection, &model, &iter))
   {
      gchar *str_data;
      gtk_tree_model_get( model, &iter, 0, &str_data, -1 );

      /* Create whole path of the file */
      cat_pathname( filename, Md.path, str_data, sizeof(filename) );
      g_free( str_data );

      /* Create the "M.Execute" command and execute it on the file */
      sprintf(cmd, "M.Load %s", filename);
		cmd_execute(  W.cmd.main_console, cmd, TRUE );
   }
}

/*-------------------------------------------------------------------------------
**   m_filemask_cb() - applies the mask to the file list display.
**   m_filemask_fo_cb() - focus out call backup.
**-------------------------------------------------------------------------------
*/
void m_filemask_cb( GtkWidget *widget, gpointer data )
{
   char buf[40];

   sprintf( buf, "m.filemask %-.25s ", gtk_entry_get_text( GTK_ENTRY(Md.file_mask_w) ) );

   cmd_execute(  W.cmd.main_console, buf, TRUE );
}
gint m_filemask_fo_cb( GtkWidget *widget, GdkEventFocus * event, gpointer data )
{
	m_filemask_cb( widget, data );
	return FALSE;
}

/*-------------------------------------------------------------------------------
**   m_set_path_cb() - hitting the 'set path' button brins up browseForPath dialog.
**-------------------------------------------------------------------------------
*/
void m_set_path_cb( GtkWidget *widget, gpointer data )
{
   bfp_show_dialog( (struct browseForPath_t *)data );
}

/*-------------------------------------------------------------------------------
**   m_path_cb() - applies the path to the file list display.
**   m_path_fo_cb() - focus out call backup.
**-------------------------------------------------------------------------------
*/
void m_path_cb( GtkWidget *widget, gpointer data )
{
   char buf[80];

   sprintf( buf, "m.path %-.65s ", gtk_entry_get_text( GTK_ENTRY(Md.path_w) ) );

   cmd_execute(  W.cmd.main_console, buf, TRUE );
}
gint m_path_fo_cb( GtkWidget *widget, GdkEventFocus * event, gpointer data )
{
	m_path_cb( widget, data );
	return FALSE;
}

/*-------------------------------------------------------------------------------
**   m_refresh_cb() - refreshes the file list display, clears the selection, and
**                 clears the file display text area.
**-------------------------------------------------------------------------------
*/
void m_refresh_cb( GtkWidget *widget, gpointer data )
{
   cmd_execute(  W.cmd.main_console, "M.Refresh", TRUE );
}

/*-------------------------------------------------------------------------------
**   m_stop_cb() - stop an executing macro.
**-------------------------------------------------------------------------------
*/
void m_stop_cb( GtkWidget *widget, gpointer data )
{
   cmd_execute(  W.cmd.main_console, "M.Stop", TRUE );
}

/*-------------------------------------------------------------------------------
**   m_clear_file() - Clear the file display.
**-------------------------------------------------------------------------------
*/
void m_clear_file( void )
{
	gtk_text_buffer_set_text( GTK_TEXT_BUFFER(Md.text_buffer_w), "", -1);
   Md.filename = NULL;      /* De-select the file */
}

/*-------------------------------------------------------------------------------
**   m_exe_timer_func() - Reads/executes the next command.
**-------------------------------------------------------------------------------
*/
gboolean m_exe_timer_func( gpointer data )
{
   int rc;
   static int  read_next = TRUE;
   static char command_buffer[80];

   if( Md.fp == NULL )
      return 1;
   /*
   ** Read Next command.
   */
   if( read_next )
   {
      if( NULL == fgets( command_buffer, sizeof(command_buffer)-1, Md.fp) )
      {
         /* EOF */
         fclose( Md.fp );
         Md.fp = NULL;
         cc_printf( W.cmd.main_console, CM_BLUE, "Macro execution Finished!\n" );
         return 0;
      }
      unpad( command_buffer, '\n' );
   }

   /*
   ** execute command in buffer
   */
   rc = cmd_execute(  W.cmd.main_console, command_buffer, TRUE );
	if( rc == ERR_NONE )
   {
      /* install another timer */
      unsigned long time_interval = ( read_next ? 50 : 200 );
      Md.exe_timerid = g_timeout_add( time_interval, m_exe_timer_func, NULL );
   }
   else
   {
      cc_printf( W.cmd.main_console, CM_RED, "Aborting Macro execute due to error!\n");
      cmd_execute(  W.cmd.main_console, "M.Stop", TRUE );
   }

   read_next = TRUE;   /* read next instruction */
   return FALSE;      /* This is FALSE because we need to adjust the time intervals */

}

/********************************************************************************/
/*  Math widgets callbacks.                                                     */
/********************************************************************************/

/*-------------------------------------------------------------------------------
**  math_equ_cb - issue equation command.
**-------------------------------------------------------------------------------
*/
void math_equ_cb( GtkWidget *widget, gpointer data )
{
   char buf[40];
   GtkWidget * entry = (GtkWidget *)data;

   strxcpy( buf, gtk_entry_get_text( GTK_ENTRY(entry) ), sizeof(buf));
   cmd_execute(  W.cmd.main_console, buf, TRUE );
}

/*-------------------------------------------------------------------------------
**  math_copy_button_cb - issues copy command.
**-------------------------------------------------------------------------------
*/
void math_copy_button_cb( GtkWidget *widget, gpointer data )
{
   char buf[60];

	int from_inx = gtk_combo_box_get_active(GTK_COMBO_BOX(W.math_copy_from_bufinx));
	int to_inx   = gtk_combo_box_get_active(GTK_COMBO_BOX(W.math_copy_to_bufinx));

   sprintf( buf, "copy %s to %s ", buffer_selection[from_inx], buffer_selection[to_inx]);
   cmd_execute(  W.cmd.main_console, buf, TRUE );
}

/*-------------------------------------------------------------------------------
** math_clear_button_cb()
**-------------------------------------------------------------------------------
*/
void math_clear_button_cb( GtkWidget *widget, gpointer data )
{
   char buf[60];

	int bufinx = gtk_combo_box_get_active(GTK_COMBO_BOX(W.math_clear_bufinx));

   sprintf( buf, "Clear %s  ", buffer_selection[bufinx]);
   cmd_execute(  W.cmd.main_console, buf, TRUE );
}

/*-------------------------------------------------------------------------------
**  math_rotate_button_cb() - 
**-------------------------------------------------------------------------------
*/
void math_rotate_button_cb( GtkWidget *widget, gpointer data )
{
   char buf[60];
	int bufinx = gtk_combo_box_get_active(GTK_COMBO_BOX(W.math_rotate_bufinx));
	int ops = gtk_combo_box_get_active(GTK_COMBO_BOX(W.math_rotate_ops));

   sprintf( buf, "Rotate %s %s ", buffer_selection[bufinx], rotate_selection[ops]);
   cmd_execute(  W.cmd.main_console, buf, TRUE );
}

/********************************************************************************/
/*  Offset widgets callbacks.                                                    */
/********************************************************************************/

/*-------------------------------------------------------------------------------
**  mga_show_button_cb() - call back for "MORIS GuideBox Adj" button.
**-------------------------------------------------------------------------------
*/
void mga_show_button_cb ( GtkWidget *widget, gpointer data )
{
   mga_show_dialog( &W.mga );
}

/********************************************************************************/
/*  Setup widgets callbacks.                                                    */
/********************************************************************************/

/*-------------------------------------------------------------------------------
**  printer_type_cb() - call back for for menu entries.
**-------------------------------------------------------------------------------
*/
void printer_type_cb ( GtkWidget *widget, gpointer data )
{
   char buf[50];
	int inx = gtk_combo_box_get_active( GTK_COMBO_BOX(widget) );

   sprintf( buf, "PrinterType %s ", printer_selection[inx] );
	cmd_execute( W.cmd.main_console, (char*)buf, TRUE);
}

/*-------------------------------------------------------------------------------
**  inst_flavor_cb() - call back for for menu entries.
**-------------------------------------------------------------------------------
*/
void inst_flavor_cb ( GtkWidget *widget, gpointer data )
{
   char buf[50];
	int inx = gtk_combo_box_get_active( GTK_COMBO_BOX(widget) );

   sprintf( buf, "Inst.Flavor %s ", inst_flavor_selection[inx] );
   cmd_execute( W.cmd.main_console, (char*)buf, TRUE);
}

/*-------------------------------------------------------------------------------
**   browse_data_path_cb() - hitting the 'Browse Path...' button brins up browseForPath dialog.
**-------------------------------------------------------------------------------
*/
void browse_data_path_cb( GtkWidget *widget, gpointer data )
{
   bfp_show_dialog( (struct browseForPath_t *)data );
}

/********************************************************************************/
/*  TCS Offset widgets callbacks.                                               */
/********************************************************************************/

/*-------------------------------------------------------------------------------
**  offset_defaults_cb() - set the angle and plate scale to the selected default.
**-------------------------------------------------------------------------------
*/
void offset_defaults_cb( GtkWidget *widget, gpointer data )
{
	int  inx = (intptr_t)data;
   switch ( inx )
   {
      case 1:
         /*set defaults for cshell */
         Lc.offset_angle      = COORD_CSHELL_ANGLE;
         Lc.offset_platescale = COORD_CSHELL_PSCALE;
         Lc.usefitsanglescale = FALSE;
         break;

      case 2:
         /*set defaults for nsfcam */
         Lc.offset_angle      = COORD_NSFCAM_ANGLE;
         Lc.offset_platescale = COORD_NSFCAM_PSCALE;
         Lc.usefitsanglescale = FALSE;
         break;

      case 3:
      default:
         /*set defaults for spex */
         Lc.offset_angle      = COORD_SPEX_ANGLE;
         Lc.offset_platescale = COORD_SPEX_PSCALE;
         Lc.usefitsanglescale = TRUE;
         break;
   }

   cal_offset_radec( );
   update_offset_widgets( );
}


/********************************************************************************/
/*         Update Functions                                                     */
/********************************************************************************/

/*-------------------------------------------------------------------------------
**  update_display_widgets () - update all user visible application widgets
**-------------------------------------------------------------------------------
*/
void update_display_widgets( void )
{
   /* Update Cmap_option_w */
   /* gtk_option_menu_set_history( Cmap_option_w, Lc.view );  */

   /* we need to figure out which page is displayed on the notebook */
   /* and call the approipriate function. For now we just call: */

   update_displayOptions_widgets( );
   update_setup_widgets( );
   update_offset_widgets( );
}

/*-------------------------------------------------------------------------------
**  update_displayOptions_widgets () - update all widgets related to
**     display configurations.
**-------------------------------------------------------------------------------
*/
void update_displayOptions_widgets( void )
{
   int  i;
   struct dpy_t * dp;

   dp = &Dpy[Lc.active];

   /* Dpyactive_w[] */
   for( i=0; i<Lc.num_dpy; i++ )
   {
     if (Lc.active == i)
        gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(W.dpyactive_w[i]), TRUE);
     else
        gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(W.dpyactive_w[i]), FALSE);
   }

   /* W.dpytype_w */
	updateComboboxWidget( W.dpytype_w, W.dpytype_hid, dp->dpytype);

   /* Dpybuf_w[] */
   for( i=0; i<NUM_BUFFER; i++ )
     gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(W.dpybuf_w[i]),
        (i==dp->bufinx?TRUE:FALSE));

   /* Set display parameters notebook */
	//updateNotebookWidget( W.dpy_notebook, W.dpy_notebook_hid, Dpy[Lc.active].dpytype );
	gtk_notebook_set_current_page( GTK_NOTEBOOK(W.dpy_notebook), Dpy[Lc.active].dpytype );

   /* update the display parameters widget for the current notebook (or dpytype) */
   switch( Dpy[Lc.active].dpytype )
   {
      case DPYTYPE_IMAGE:
         update_dpytype_image_widgets( &Dpy[Lc.active] );
         break;
      case DPYTYPE_HISTOGRAM:
         update_dpytype_histogram_widgets( &Dpy[Lc.active] );
         break;
      case DPYTYPE_LINECUT:
         update_dpytype_linecut_widgets( &Dpy[Lc.active] );
         break;
      case DPYTYPE_NOISE:
         update_dpytype_noise_widgets( &Dpy[Lc.active] );
         break;
      case DPYTYPE_XLINECUT:
         update_dpytype_xlinecut_widgets( &Dpy[Lc.active] );
         break;
      case DPYTYPE_POINTER:
         update_dpytype_pointer_widgets( &Dpy[Lc.active] );
         break;
      case DPYTYPE_STATS:
         update_dpytype_stats_widgets( &Dpy[Lc.active] );
         break;
      case DPYTYPE_AOFIG:
         update_dpytype_aofig_widgets( &Dpy[Lc.active] );
         break;
      case DPYTYPE_SA:
         update_dpytype_spectra_a_widgets( &Dpy[Lc.active] );
         break;
      case DPYTYPE_SB:
         update_dpytype_spectra_b_widgets( &Dpy[Lc.active] );
         break;
   }
}

/*-------------------------------------------------------------------------------
**  update_dpytype_image_widgets () - update all widgets related to
**     DisplayType DPYTYPE_IMAGE
**-------------------------------------------------------------------------------
*/
void update_dpytype_image_widgets( struct dpy_t *dp )
{
   char buf[60];

   /* Image AutoScale */
   gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(W.image_autoscale[dp->image_autoscale]), TRUE);

   /* Image Zoom */
   updateAdjustmentWidget( W.image_zoom, W.image_zoom_hid, dp->image_zoom );

   /* Image Range: Min & Max  */
   if( (dp->image_max-dp->image_min) < 10 )
   {
		sprintf( buf, "%4.2f", dp->image_min );
		gtk_entry_set_text( GTK_ENTRY(W.image_range_min), buf );

		sprintf( buf, "%4.2f", dp->image_max );
		gtk_entry_set_text( GTK_ENTRY(W.image_range_max), buf );
   }
   else
   {
		sprintf( buf, "%2.0f", dp->image_min );
		gtk_entry_set_text( GTK_ENTRY(W.image_range_min), buf );

		sprintf( buf, "%2.0f", dp->image_max );
		gtk_entry_set_text( GTK_ENTRY(W.image_range_max), buf );
   }
}

/*-------------------------------------------------------------------------------
**  update_dpytype_histogram_widgets () - update all widgets related to
**     DisplayType DPYTYPE_HISTOGRAM
**-------------------------------------------------------------------------------
*/
void update_dpytype_histogram_widgets( struct dpy_t *dp )
{
   char buf[60];

   /* Histogram Range: Min & Max  */
   sprintf( buf, "%2.0f", dp->image_min );
   gtk_entry_set_text( GTK_ENTRY(W.hist_range_min), buf );

   sprintf( buf, "%2.0f", dp->image_max );
   gtk_entry_set_text( GTK_ENTRY(W.hist_range_max), buf );

   /* Histogram Bins  */
   sprintf( buf, "%2.0d", dp->hist_bin );
   gtk_entry_set_text( GTK_ENTRY(W.hist_bins), buf );

   /* Histogram Area */
   gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(W.hist_area[dp->hist_area]), TRUE);

   /* Histogram auto range  */
   gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(W.hist_auto_range), dp->hist_auto_range);
}

/*-------------------------------------------------------------------------------
**  update_dpytype_linecut_widgets () - update all widgets related to
**     DisplayType DPYTYPE_LINECUT
**-------------------------------------------------------------------------------
*/
void update_dpytype_linecut_widgets( struct dpy_t *dp )
{
   char buf[60];

   /* Linecut AutoScale */
   gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(W.lcut_autoscale[dp->lcut_autoscale]), TRUE);

   /* Linecut Range: Min & Max  */
   sprintf( buf, "%2.0f", dp->lcut_min );
   gtk_entry_set_text( GTK_ENTRY(W.lcut_range_min), buf );

   sprintf( buf, "%2.0f", dp->lcut_max );
   gtk_entry_set_text( GTK_ENTRY(W.lcut_range_max), buf );

   /* Linecut Axes: X & Y  */
   sprintf( buf, "%d", dp->lcut_x );
   gtk_entry_set_text( GTK_ENTRY(W.lcut_x), buf );

   sprintf( buf, "%d", dp->lcut_y );
   gtk_entry_set_text( GTK_ENTRY(W.lcut_y), buf );

   /* Linecut Area */
   gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(W.lcut_area[dp->lcut_area]), TRUE);
}

/*-------------------------------------------------------------------------------
**  update_dpytype_noise_widgets () - update all widgets related to
**     DisplayType DPYTYPE_NOISE
**-------------------------------------------------------------------------------
*/
void update_dpytype_noise_widgets( struct dpy_t *dp )
{
   char buf[60];

    /* Noise Mod  */
   sprintf( buf, "%d", dp->noise_mod );
   gtk_entry_set_text( GTK_ENTRY(W.noise_mod), buf );

    /* Noise Ch Size  */
   sprintf( buf, "%d", dp->noise_size );
   gtk_entry_set_text( GTK_ENTRY(W.noise_size), buf );

   /* Noise GraphType */
   gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(W.noise_graph_type[dp->noise_graph_type]), TRUE);

   /* Noise AutoScale */
   gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(W.noise_autoscale[dp->noise_autoscale]), TRUE);

   /* Noise Area */
   gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(W.noise_area[dp->noise_area]), TRUE);

   /* Noise mode */
   gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(W.noise_mode[dp->noise_mode]), TRUE);

   /* Noise Graph 1 Range: Min & Max  */
   sprintf( buf, "%2.0f", dp->noise_g1_min );
   gtk_entry_set_text( GTK_ENTRY(W.noise_g1_range_min), buf );

   sprintf( buf, "%2.0f", dp->noise_g1_max );
   gtk_entry_set_text( GTK_ENTRY(W.noise_g1_range_max), buf );

    /* Noise Graph 2 Range: Min & Max  */
   sprintf( buf, "%2.0f", dp->noise_g2_min );
   gtk_entry_set_text( GTK_ENTRY(W.noise_g2_range_min), buf );

   sprintf( buf, "%2.0f", dp->noise_g2_max );
   gtk_entry_set_text( GTK_ENTRY(W.noise_g2_range_max), buf );
}

/*-------------------------------------------------------------------------------
**  update_dpytype_pointer_widgets () - update all widgets related to
**     DisplayType DPYTYPE_POINTER
**-------------------------------------------------------------------------------
*/
void update_dpytype_pointer_widgets( struct dpy_t *dp )
{
   char buf[60];

   /* Pointer Image Size  */
   sprintf( buf, "%d", dp->pt_image_size );
   gtk_entry_set_text( GTK_ENTRY(W.pt_image_size), buf );

   /* PtShowStats */
   gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(W.pt_show_stats), dp->pt_show_stats);

   /* PtShowSpexSN */
   gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(W.pt_show_spex_sn), dp->pt_show_spex_sn);

   /* PtShowLinecut */
   gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(W.pt_show_linecut), dp->pt_show_linecut);
}

/*-------------------------------------------------------------------------------
**  update_dpytype_xlinecut_widgets () - update all widgets related to
**     DisplayType DPYTYPE_XLINECUT
**-------------------------------------------------------------------------------
*/
void update_dpytype_xlinecut_widgets( struct dpy_t *dp )
{
   char buf[60];

   /* XLinecut AutoScale */
   gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(W.xcut_autoscale[dp->xcut_autoscale]), TRUE);

   /* XLinecut Range: Min & Max  */
   sprintf( buf, "%2.0f", dp->xcut_min );
   gtk_entry_set_text( GTK_ENTRY(W.xcut_range_min), buf );

   sprintf( buf, "%2.0f", dp->xcut_max );
   gtk_entry_set_text( GTK_ENTRY(W.xcut_range_max), buf );

   /* XLinecut Line beginning and end points  */
   sprintf( buf, "%d %d", dp->xcut_xbeg, dp->xcut_ybeg );
   gtk_entry_set_text( GTK_ENTRY(W.xcut_beg), buf );

   sprintf( buf, "%d %d", dp->xcut_xend, dp->xcut_yend );
   gtk_entry_set_text( GTK_ENTRY(W.xcut_end), buf );
}

/*----------------------------------------------------------------
**  update_dpytype_stats_widgets () - update all widgets related to
**     DisplayType DPYTYPE_STATS
**----------------------------------------------------------------
*/
void update_dpytype_stats_widgets( struct dpy_t *dp )
{
   char buf[60];

   /* FixedWH toggle */
   gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(W.stats_fixedwh), Stats[dp->bufinx].fixedWH );

   /* FixedWH entry */
   sprintf( buf, "%d %d", Stats[dp->bufinx].objwid, Stats[dp->bufinx].objhgt);
   gtk_entry_set_text( GTK_ENTRY(W.stats_wh_entry), buf );
}

/*----------------------------------------------------------------
**  update_dpytype_aofig_widgets () - update all widgets related to
**     DisplayType DPYTYPE_AOFIG
**----------------------------------------------------------------
*/
void update_dpytype_aofig_widgets( struct dpy_t *dp )
{
   /* Aofig_format_w */
	updateComboboxWidget( W.aofig_format, W.aofig_format_hid, dp->aofig_format);

   /* Aofig_data_w */
	updateComboboxWidget( W.aofig_data, W.aofig_data_hid, dp->aofig_data);

    /* XY adjustment */
    gtk_spin_button_set_value( GTK_SPIN_BUTTON(W.aofig_x), dp->aofig_x );
    gtk_spin_button_set_value( GTK_SPIN_BUTTON(W.aofig_y), dp->aofig_y );
}

/*----------------------------------------------------------------
**  update_dpytype_spectra_a_widgets () - update all widgets 
**     related to DisplayType DPYTYPE_SA
**----------------------------------------------------------------
*/
void update_dpytype_spectra_a_widgets( struct dpy_t *dp )
{
   char buf[80];

   // objbin min & max 
   sprintf( buf, "%d %d", dp->sa_objbin_min, dp->sa_objbin_max);
   gtk_entry_set_text( GTK_ENTRY(W.sa_objbin), buf );

   // objbin_label
   sprintf( buf, "(%d)", dp->sa_objbin_max-dp->sa_objbin_min+1);
   gtk_label_set_text( GTK_LABEL(W.sa_objbin_label), buf );

   // skybin min & max 
   sprintf( buf, "%d %d", dp->sa_skybin_min, dp->sa_skybin_max);
   gtk_entry_set_text( GTK_ENTRY(W.sa_skybin), buf );

   // skybin_label
   sprintf( buf, "(%d)", dp->sa_skybin_max-dp->sa_skybin_min+1);
   gtk_label_set_text( GTK_LABEL(W.sa_skybin_label), buf );

   // rows per bin 
   sprintf( buf, "%d ", dp->sa_rows_per_bin );
   gtk_entry_set_text( GTK_ENTRY(W.sa_rows_per_bin), buf );

   // shift 
   sprintf( buf, "%d ", dp->sa_shift );
   gtk_entry_set_text( GTK_ENTRY(W.sa_shift), buf );

   /* SubtractSky */
   gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(W.sa_subtractsky), dp->sa_subtractsky);

   /* Stats */
   gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(W.sa_stats), dp->sa_stats);

   /* YautoScale */
	updateComboboxWidget( W.sa_yautoscale, W.sa_yautoscale_hid, dp->sa_yautoscale);

   /* Y Scale */
   sprintf( buf, "%d %d ", dp->sa_ymin, dp->sa_ymax);
   gtk_entry_set_text( GTK_ENTRY(W.sa_yscale), buf );

   /* X Scale */
   sprintf( buf, "%d %d ", dp->sa_xmin, dp->sa_xmax);
   gtk_entry_set_text( GTK_ENTRY(W.sa_xscale), buf );

}

/*----------------------------------------------------------------
**  update_dpytype_spectra_b_widgets () - update all widgets 
**     related to DisplayType DPYTYPE_SB
**----------------------------------------------------------------
*/
void update_dpytype_spectra_b_widgets( struct dpy_t *dp )
{
   char buf[80];

   // objbin min & max 
   sprintf( buf, "%d %d", dp->sb_objbin_min, dp->sb_objbin_max);
   gtk_entry_set_text( GTK_ENTRY(W.sb_objbin), buf );

   // objbin_label
   sprintf( buf, "(%d)", dp->sb_objbin_max-dp->sb_objbin_min+1);
   gtk_label_set_text( GTK_LABEL(W.sb_objbin_label), buf );

   // skybin min & max 
   sprintf( buf, "%d %d", dp->sb_skybin_min, dp->sb_skybin_max);
   gtk_entry_set_text( GTK_ENTRY(W.sb_skybin), buf );

   // skybin_label
   sprintf( buf, "(%d)", dp->sb_skybin_max-dp->sb_skybin_min+1);
   gtk_label_set_text( GTK_LABEL(W.sb_skybin_label), buf );

   /* Show */
   gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(W.sb_show_diff), (dp->sb_showgraph&0x01)==0x01);
   gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(W.sb_show_obj),  (dp->sb_showgraph&0x02)==0x02);
   gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(W.sb_show_sky),  (dp->sb_showgraph&0x04)==0x04);

   /* YAutoScale */
   gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(W.sb_yautoscale), dp->sb_yautoscale);

   // Diff Y Range 
   sprintf( buf, "%d %d", dp->sb_diff_ymin, dp->sb_diff_ymax);
   gtk_entry_set_text( GTK_ENTRY(W.sb_diffyrange), buf );

   // Data Y Range 
   sprintf( buf, "%d %d", dp->sb_data_ymin, dp->sb_data_ymax);
   gtk_entry_set_text( GTK_ENTRY(W.sb_datayrange), buf );

   // Set X Scale  
   sprintf( buf, "%d %d", dp->sb_xmin, dp->sb_xmax);
   gtk_entry_set_text( GTK_ENTRY(W.sb_xscale), buf );
}

/*-------------------------------------------------------------------------------
**  update_setup_widgets () - update all widgets related to setup notebook.
**-------------------------------------------------------------------------------
*/
void update_setup_widgets ( void )
{
   char buf[80];

   if( Lc.dvport )
		sprintf( buf, "Dv socket port is %d ", Lc.dvport);
	else
		strcpy( buf, "Dv socket was not initialized.");
   gtk_label_set_text( GTK_LABEL(W.dvport_w), buf );

   /* DivByDivisor */
   gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(W.div_by_divisor), Lc.divbydivisor);

   /* TSC Hostname */
   gtk_entry_set_text( GTK_ENTRY(W.tcs_hostname), Lc.tcshostname );

   /* Printer */
   gtk_entry_set_text( GTK_ENTRY(W.printer_name), Lc.printer );

   /* PrinterType */
	updateComboboxWidget( W.printer_type, W.printer_type_hid, Lc.printertype);

   /* PrintToFile */
   gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(W.print_to_file), Lc.printtofile);

   /* Path */
   gtk_entry_set_text( GTK_ENTRY(W.path), Lc.path );

   /* CMSwapLongs */
   gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(W.cm_swap_rgb), CM.swap_rgb);

   /* Inst_flavor */
	updateComboboxWidget( W.inst_flavor, W.inst_flavor_hid, Lc.inst_flavor);

   /* ImageCompass_w */
   gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(W.image_compass), Lc.image_compass_show);

   /* ImageCompass_FlipNS_w */
   gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(W.image_compass_flip_NS), Lc.image_compass_flipNS);

   /* ImageShowGBox_w */
   gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(W.image_show_gbox), Lc.image_show_gbox);

   /* GBox_Cmd_Right_Click_w */
   gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(W.gbox_cmd_right_click), Lc.gbox_cmd_right_click );

}

/*-------------------------------------------------------------------------------
**  void update_offset_widgets() - refresh tcs offset widgets
**-------------------------------------------------------------------------------
*/
void update_offset_widgets( void )
{
   char  buf[40];

   /* Use_FITS_Angle&Scale */
   gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(W.use_fits_angle_scale), Lc.usefitsanglescale);

   /* Position Angle */
   sprintf( buf, " %6.2f ", Lc.offset_angle);
   gtk_entry_set_text( GTK_ENTRY(W.offset_angle), buf );

   /* Plate Scale  */
   sprintf( buf, " %7.4f ", Lc.offset_platescale);
   gtk_entry_set_text( GTK_ENTRY(W.offset_platscale), buf );

   /* Update Beg X,Y */
   sprintf( buf, " %2.0f %2.0f ", Lc.offset_beg_x, Lc.offset_beg_y);
   gtk_entry_set_text( GTK_ENTRY(W.offset_beg_xy), buf );

   /* Update End X,Y */
   sprintf( buf, " %2.0f %2.0f ", Lc.offset_end_x, Lc.offset_end_y);
   gtk_entry_set_text( GTK_ENTRY(W.offset_end_xy), buf );

   /* Update RA & DEC coordinates */
   sprintf( buf, " %2.2f, %2.2f ", Lc.offset_ra, Lc.offset_dec);
   gtk_label_set_text( GTK_LABEL(W.offset_radec), buf );
}

/****************************************************************************/
/*  Dialog Windows Callbacks                                                */
/****************************************************************************/

/*----------------------------------------------------------------------------
**  file_open_activate_cb() - call backup from OPEN gtk_file_chooser "file-activate"
**----------------------------------------------------------------------------
*/
void file_open_activate_cb( GtkWidget *widget, gpointer data )
{
   char * f_name;
	int  bufinx;
   char cmd[512];

   // get f_name 
   f_name = gtk_file_chooser_get_filename( GTK_FILE_CHOOSER(widget) );

   // get buffer 
   bufinx = gtk_combo_box_get_active( GTK_COMBO_BOX(W.file_open_dialog_buffer) );

   // formt dv command
	sprintf(cmd, "Read %s %s ", f_name, buffer_selection[bufinx] );
	g_free( f_name );

   // execute command
	//printf("file_open_activate_cb() %s \n", cmd);
	cmd_execute( W.cmd.main_console, cmd, TRUE );
}

/*----------------------------------------------------------------------------
**  file_open_response_cb() - when user presses 'open' /
**----------------------------------------------------------------------------
*/
void file_open_response_cb( GtkWidget *widget, gint response_id, gpointer data )
{
   // OK button
   if( response_id == GTK_RESPONSE_ACCEPT )
	{
		char * f_name;
		char cmd[512];
		int bufinx;

		f_name = gtk_file_chooser_get_filename( GTK_FILE_CHOOSER(widget) );

		// get buffer 
		bufinx = gtk_combo_box_get_active( GTK_COMBO_BOX(W.file_open_dialog_buffer) );

		// formt dv command
		sprintf(cmd, "Read %s %s ", f_name,  buffer_selection[bufinx] );
		g_free( f_name );

		// execute command
		//printf("file_open_response_cb() %s \n", cmd);
		cmd_execute( W.cmd.main_console, cmd, TRUE );
	}
   // Cancel button
   if( response_id == GTK_RESPONSE_CANCEL )
	{
	   gtk_widget_hide( widget );
	}
}

/*----------------------------------------------------------------------------
**  file_open_read_xtension() - cb for 'read.xtension' button on File Open dialog
**----------------------------------------------------------------------------
*/
void file_open_read_xtension( GtkWidget *widget, gpointer data )
{
   int xtension,
       bufinx;
	char * f_name;
	char cmd[512];

   // spin button provides the Xtension number
   xtension = (int) gtk_spin_button_get_value( GTK_SPIN_BUTTON(W.file_open_xtension));

   f_name = gtk_file_chooser_get_filename( GTK_FILE_CHOOSER(W.file_open_dialog) );

   // get buffer 
   bufinx = gtk_combo_box_get_active( GTK_COMBO_BOX(W.file_open_dialog_buffer) );

   // formt dv command
   sprintf(cmd, "ReadXtension %s %s %d ", f_name,  buffer_selection[bufinx], xtension );
   g_free( f_name );

   // execute command
   cmd_execute( W.cmd.main_console, cmd, TRUE );
}

/*----------------------------------------------------------------------------
**  file_open_movieshow() - cb for 'ShowMovie' button on File Open dialog
**----------------------------------------------------------------------------
*/
void file_open_movieshow( GtkWidget *widget, gpointer data )
{
   int bufinx;
	char * f_name;
	char cmd[512];


   f_name = gtk_file_chooser_get_filename( GTK_FILE_CHOOSER(W.file_open_dialog) );

   // get buffer 
   bufinx = gtk_combo_box_get_active( GTK_COMBO_BOX(W.file_open_dialog_buffer) );

   // formt dv command
   sprintf(cmd, "MovieShow %s %s ", f_name,  buffer_selection[bufinx] );
   g_free( f_name );

   // execute command
   cmd_execute( W.cmd.main_console, cmd, TRUE );
}

/*----------------------------------------------------------------------------
**  file_save_response_cb() - when user presses 'open' /
**----------------------------------------------------------------------------
*/
void file_save_response_cb( GtkWidget *widget, gint response_id, gpointer data )
{
   if( response_id == GTK_RESPONSE_ACCEPT )
	{
		// user pressed the 'save' button

      char * f_name;
		char path[256];
		char buf[256];
		int bufinx;

		// get filename 
		f_name = gtk_file_chooser_get_filename( GTK_FILE_CHOOSER(widget) );
		strxcpy( path, f_name, sizeof(path));
		g_free( f_name );

		// get buffer 
		bufinx = gtk_combo_box_get_active( GTK_COMBO_BOX(W.file_save_dialog_buffer) );

      // issue save command.
		sprintf( buf, "Save  %s %s ", path, buffer_selection[bufinx] );
		cmd_execute( W.cmd.main_console, buf, TRUE );

	}
	else if( response_id == GTK_RESPONSE_CANCEL )
	{
		// user pressed the 'cancel' button
	   gtk_widget_hide( widget );
	}
}

/*----------------------------------------------------------------------------
**  file_save_bufinx_cb() - when user pick a new buffer on the save dialog
**    dv needs to update the folder & filename.
**----------------------------------------------------------------------------
*/

void file_save_bufinx_cb( GtkWidget * widget, gpointer data )
{
	int  bufinx = gtk_combo_box_get_active( GTK_COMBO_BOX(widget) );

	update_file_save_dialog( bufinx );
}

/*-------------------------------------------------------------------------------
**  update_file_save_dialog() - updates the bufinx & path information in the save dialog box.
**-------------------------------------------------------------------------------
*/
void update_file_save_dialog( int bufinx )
{
   char buf[512];

	static char path[DF_FITS_LEN_PATH];

	int  save_bufinx = gtk_combo_box_get_active( GTK_COMBO_BOX(W.file_save_dialog_buffer) );
   if ( bufinx != save_bufinx )
      return;

   if ( Buffer[bufinx].status == DF_EMPTY )
   {
		gtk_file_chooser_set_current_name( GTK_FILE_CHOOSER( W.file_save_dialog ), "EMPTY");
	}
	else
	{
		gtk_file_chooser_set_current_name( GTK_FILE_CHOOSER( W.file_save_dialog ), 
			Buffer[bufinx].filename );

      /*
		** use the static path[] variable to limited calling gtk_file_chooser_set_current_folder()
		** only if the path changes. gtk_file_chooser is a bad widget with memory leaks,
		** calling set_current_folder() seem to be a leaky function!!!!
		** Maybe consider not calling gtk_file_chooser_set_current_folder() at all!
		*/
      if( strncmp( path, Buffer[bufinx].directory, DF_FITS_LEN_PATH-1 ) != 0 )
		{
			strxcpy( path, Buffer[bufinx].directory, sizeof(path));
		   printf("update_file_save_dialog() %s\n", path);

			sprintf( buf, "%s/", Buffer[bufinx].directory );
			gtk_file_chooser_set_current_folder( GTK_FILE_CHOOSER( W.file_save_dialog ), buf );
		}
	}

   return;
}

/*-----------------------------------------------------------------------
**  bfp_macro_ok_cb() - callback for (data) path's OK button from bfp dialog
**-----------------------------------------------------------------------
*/
void bfp_macro_ok_cb( GtkWidget *widget, gpointer data )
{
   char buf[256];
   struct browseForPath_t *bfp = (struct browseForPath_t *)data;

   strcpy( buf, "m.path   ");
   strxcpy( &buf[7], bfp->dirlist.path, sizeof(buf)-7);
   if( cmd_execute( W.cmd.main_console, buf, TRUE ) == ERR_NONE )
      bfp_hide_dialog( bfp );
}
   
/*-----------------------------------------------------------------------
**  bfp_data_path_ok_cb() - callback for (data) path's OK button from bfp dialog
**-----------------------------------------------------------------------
*/
void bfp_data_path_ok_cb( GtkWidget *widget, gpointer data )
{
   char buf[256];
   struct browseForPath_t *bfp = (struct browseForPath_t *)data;

   strcpy( buf, "Path    ");
   strxcpy( &buf[7], bfp->dirlist.path, sizeof(buf)-7);
   if( cmd_execute( W.cmd.main_console, buf, TRUE ) == ERR_NONE )
      bfp_hide_dialog( bfp );
}

/************************************************************************
**  Application Timer
*************************************************************************
*/

/*------------------------------------------------------------------------
**  timer_function() -
**------------------------------------------------------------------------
*/
gboolean timer_function( gpointer data )
{
   /*-----------------------------------
   ** refresh status display
   */

	Lc.app_timerid = g_timeout_add( Lc.timer_ms, timer_function, NULL);
   return 0; /* false = Do not renew timer */
}

/*-------------------------------------------------------------------------------
**  socket_accept_connection() - This routine services request
**     for connections on a socket. It accept the connection,
**     reads and executes commands from the connection. 
**-------------------------------------------------------------------------------
*/ 
#if USE_GIO
gboolean socket_accept_connection( GIOChannel * source, GIOCondition condition, gpointer data)
#else
void socket_accept_connection( gpointer data, int fd, GdkInputCondition ic )
#endif
{
	int  one;
   int  i, a;
   char buf[SOCKET_MSG_LEN];

	struct sockaddr saddr;
	socklen_t       saddr_len = sizeof(saddr);

   /*
   **  accept connection
   */
   if( -1 == (Lc.connect_fd = accept( Lc.sock_fd, &saddr, &saddr_len)) )
      return TRUE;

   one = 1;
   setsockopt( Lc.connect_fd , SOL_SOCKET, SO_REUSEADDR, &one, sizeof( one ));
   /*
   **  Read and process data.
   */
   while( (a=sock_read_data( Lc.connect_fd, buf, (long)SOCKET_MSG_LEN, FALSE, 1000)) > 0 )
   {
      cmd_execute( W.cmd.main_console, buf, TRUE );
   }
   /*
   **  Close connection.
   */
	shutdown( Lc.connect_fd, 2);
   close( Lc.connect_fd );
   
   /*
   ** execute command on cmd_stack (in FIRST-IN-FIRST-OUT).
   */

   for( i=0; i<Lc.stack_inx; i++ )
   {
      char * cptr = Lc.cmd_stack[i];
      cmd_execute( W.cmd.main_console, cptr, TRUE );
      free( cptr );
   }
   Lc.stack_inx = 0;

	return TRUE;
}

/********************************************************************************/
/*                   helper function for updating widgets                       */
/********************************************************************************/

/*----------------------------------------------------------------
** updateNotebookWidget() - sets the value of the notebook widget
**----------------------------------------------------------------
*/
void updateNotebookWidget( GtkWidget * widget, int widget_hid, int index )
{
   g_signal_handler_block( GTK_OBJECT(widget), widget_hid );
   gtk_notebook_set_current_page( GTK_NOTEBOOK(widget), index );
   g_signal_handler_unblock( GTK_OBJECT(widget), widget_hid );
}

/*----------------------------------------------------------------
** updateComboboxWidget() - sets the value of the combo box
**----------------------------------------------------------------
*/ 
void updateComboboxWidget( GtkWidget * widget, int widget_hid, int index )
{
   g_signal_handler_block( GTK_OBJECT(widget), widget_hid );
   gtk_combo_box_set_active( GTK_COMBO_BOX(widget), index );
   g_signal_handler_unblock( GTK_OBJECT(widget), widget_hid );
}


/*----------------------------------------------------------------
** updateAdjustmentWidget() - sets the value of the Adjustment object 
**----------------------------------------------------------------
*/
void updateAdjustmentWidget( GtkObject * widget, int widget_hid, gdouble value )
{
   g_signal_handler_block( GTK_OBJECT(widget), widget_hid );
   gtk_adjustment_set_value(GTK_ADJUSTMENT(widget), value );
   g_signal_handler_unblock( GTK_OBJECT(widget), widget_hid );
}

/*-------------------------------------------------------------------------------
**  update_file_list( ) - Updates the file list.
**-------------------------------------------------------------------------------
*/

int update_file_list( 
   int isDir,                 // T or F; Ture filter only directories
	char *cdir,                // current directory 
	char *fmask,               // string filter for list
	GtkListStore *list_store, 
	GtkTreeView *list_view
)
{
#define MAX_FILE_LIST 2000

   int l, rc, nfile;

   DIR            *dirp;
   struct dirent  *dp;
   struct stat    sbuf;
   char           **avlist;

   char workingdir[256];
   char listdir[256];
   char pathname[300];

   avlist   = (char **) malloc( MAX_FILE_LIST * sizeof (char *));
   nfile    = 0;
   rc       = ERR_NONE;

   /*  read current directory and save in workingdir[] */
   getcwd( workingdir, sizeof(workingdir) );

   /* change to cdir */
   if( strlen( cdir ) >= sizeof(listdir)-1 )
      { rc = ERR_INV_PATH; goto done; }
   strxcpy( listdir, cdir, sizeof(listdir));
   unpad( listdir, ' ');
   if( chdir( listdir ) < 0 )
      { rc = ERR_INV_PATH; goto done; }

   getcwd( listdir, sizeof(listdir));
   /*
   **  Read directory and update list
   */
   if( NULL != (dirp = opendir(listdir)) )
   {
      for( dp = readdir(dirp); dp != NULL; dp = readdir(dirp))
      {
         if( matchname( fmask, dp->d_name))
          {
            cat_pathname( pathname, listdir, dp->d_name, sizeof(pathname));
            if( stat(pathname, &sbuf) >= 0 )
            {
               if( ( isDir ? S_ISDIR(sbuf.st_mode) :  S_ISREG(sbuf.st_mode))
                   && (nfile < MAX_FILE_LIST)
                   && (dp->d_name[0] != '.'))
               {
                  l = strlen(dp->d_name);
                  avlist[nfile] = malloc( (unsigned) l+1);
                  memcpy( avlist[nfile], dp->d_name, l+1);
                  nfile++;
               }
             }
         }
      }
      closedir( dirp );
   }

   /* Sort the list */
   if( nfile > 0 )
   {
      qsort( (char *) avlist, nfile, sizeof(char*), qsort_comp_string );
   }

   struct sclist_view_item_t temp;

   /* Delete current contents of list */
   sclist_view_clear(list_view, list_store);

   /* Add All items to list */
   {
      for( l=0; l<nfile; l++ )
      {
         sprintf(temp.text, "%s", avlist[l]);
         sclist_view_append(list_store, &temp);
      }
   }

done:
   for( l=0; l<nfile; l++)
      free( (char*) avlist[l]);
   free( (char*) avlist);

   /* change back to original working directory */
   chdir( workingdir );
   return rc;
}

/****************************************************************************/
/****************************************************************************/

