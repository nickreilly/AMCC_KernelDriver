/***************************************************************************
**  init.c - Function for initializing application & creating GUI elements
**
** Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php
** Copyright (C) 2012 <NASA IRTF. http://irtfweb.ifa.hawaii.edu>
***************************************************************************
*/

#define EXTERN extern
#define DEBUG 0

/*--------------------------
**  Standard include files
**--------------------------
*/
#include <sys/time.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <semaphore.h>
#include <mqueue.h>

#include <gtk/gtk.h>

/*-----------------------------
**  Non-standard include files
**-----------------------------
*/
#include "dv.h"

/*-----------------------------------------------------------------------
**  create_base() - create/initialize all GUI objects
**-----------------------------------------------------------------------
*/
int create_base(  char * fixed_font_name  )
{
   GtkWidget * base_window;
   GtkWidget * base_vbox;
   GtkWidget * widget;
   GtkWidget * window;
   char buf[80];

   /*----------------------------------
   ** create application's base window
   */
   base_window = gtk_window_new( GTK_WINDOW_TOPLEVEL );
   sprintf( buf, "%s %s (%s)", APP_NAME, APP_VERSION, __DATE__);
   gtk_window_set_title (GTK_WINDOW (base_window), buf );
   g_signal_connect (G_OBJECT(base_window), "delete_event", G_CALLBACK(base_delete_event), 
      (gpointer) " quit ");
   g_signal_connect (G_OBJECT(base_window), "destroy",      G_CALLBACK(base_destroy_cb), NULL);
   gtk_widget_realize( base_window );

#if USE_PANGO
   // fixed fonts
	get_font( &W.fixed_font, base_window, fixed_font_name);
   W.fixed_font_wid = W.fixed_font.wid;
   W.fixed_font_hgt = W.fixed_font.hgt;
#else
   // fixed fonts
   if( (W.fixed_font = gdk_font_load ( fixed_font_name )) == NULL )
      printf("Error unable to font: Fixed_font\n");
   W.fixed_font_wid = gdk_char_width( W.fixed_font, 'W' );
   W.fixed_font_hgt =  W.fixed_font->ascent+W.fixed_font->descent;
#endif

   /*--------------------------------------
   ** Create/set application icon.
   */
   {
      GtkStyle *style;
      GdkBitmap *mask;

      style = gtk_widget_get_style( base_window );
      W.icon_pixmap = gdk_pixmap_create_from_xpm_d( base_window->window, &mask,
       &style->bg[GTK_STATE_NORMAL], (gchar **)Icon_xpm_data );

      gdk_window_set_icon( base_window->window, NULL, W.icon_pixmap, NULL);
      gdk_window_set_icon_name( base_window->window, APP_NAME);
   }

   /*---------------------------------------------------------------
   ** Create some styles for different background/foregound combo.
	*/
	W.style_Default = gtk_widget_get_default_style();
	W.style_BlackBG  = gtk_style_copy( W.style_Default );
	MyStyleSetItemColor( CM.colors[CM_BLACK],  'b', W.style_BlackBG );

	W.style_YellowGray  = gtk_style_copy( W.style_Default );
	MyStyleSetItemColor( CM.colors[CM_GRAY],    's', W.style_YellowGray );
	MyStyleSetItemColor( CM.colors[CM_YELLOW],  't', W.style_YellowGray );

	W.style_GreenGray  = gtk_style_copy( W.style_Default );
	MyStyleSetItemColor( CM.colors[CM_GRAY],   's', W.style_GreenGray );
	MyStyleSetItemColor( CM.colors[CM_GREEN],  't', W.style_GreenGray );

	W.style_BlueGray  = gtk_style_copy( W.style_Default );
	MyStyleSetItemColor( CM.colors[CM_GRAY],  's', W.style_BlueGray );
	MyStyleSetItemColor( CM.colors[CM_BLUE],  't', W.style_BlueGray );

	W.style_Red  = gtk_style_copy( W.style_Default );
	MyStyleSetItemColor( CM.colors[CM_RED],  't', W.style_Red );

	W.style_Green  = gtk_style_copy( W.style_Default );
	MyStyleSetItemColor( CM.colors[CM_GREEN],  't', W.style_Green );

   /*-------------------------------------
   ** Create base_vbox on the base_window - this is the main contain on the base window.
   */
   base_vbox = gtk_vbox_new( FALSE, 0);
   gtk_container_add(GTK_CONTAINER(base_window), base_vbox);

   /*----------------------------------
   ** Create main (or application) buttons
   */
   create_mainbar_widgets( &widget );
   gtk_box_pack_start(GTK_BOX(base_vbox), widget, FALSE, TRUE, 0);
   gtk_widget_show( widget );
   
   /*----------------------------------
   ** Create all data display widgets 
   */
	create_main_dpydata( &widget );
	gtk_box_pack_start(GTK_BOX(base_vbox), widget, FALSE, TRUE, 0);
	gtk_widget_show( widget );

	create_dialog_dpydata( &W.dpy_dialog_window );

   /*-----------------------------------------------------------------------------------
   ** Configuration Widgets can be on the BASE or in a Dialog Box 
   */
	if( Lc.use_config_window )
	{
		// create configuration widget inside a dialog window. 
		window = gtk_window_new( GTK_WINDOW_TOPLEVEL );
		gtk_window_set_title( GTK_WINDOW (window), " DV Controls " );
		g_signal_connect( G_OBJECT(window), "delete_event", G_CALLBACK(dialog_delete_event), NULL );
		g_signal_connect( G_OBJECT(window), "destroy", G_CALLBACK(dialog_delete_event), NULL );
	   W.configure_dialog_window = window;

		create_configure_widgets( &widget, base_window );
      gtk_container_add( GTK_CONTAINER(window), widget );
		gtk_widget_show( widget );
	}
	else
	{
	   W.configure_dialog_window = NULL;
		create_configure_widgets( &widget, base_window );
		gtk_box_pack_start(GTK_BOX(base_vbox), widget, FALSE, TRUE, 0);
		gtk_widget_show( widget );
	}

   /*----------------------------------
   ** A command feedback label on the base_window
   */
	widget = gtk_label_new("command feedback");
	gtk_misc_set_alignment( GTK_MISC(widget), 0, 1);
	gtk_box_pack_start(GTK_BOX(base_vbox), widget, FALSE, TRUE, 0);
	gtk_widget_show( widget );
	W.command_feedback =  widget;

   /* show vbox, and base_window */
   gtk_widget_show( base_vbox );
   gtk_widget_show( base_window );

   /*------------------------------------------------------------
   ** Get GC need for DrawingArea
	*/
   if( (W.nor_gc = gdk_gc_new(base_window->window)) == NULL )
      printf("Error unable to obtain new GC: Nor_gc\n");


   if( (W.xor_gc = gdk_gc_new(base_window->window)) == NULL )
		printf("Error unable to obtain new GC: W.xor_gc\n");
	gdk_gc_set_function( W.xor_gc, GDK_XOR);
	gdk_gc_set_line_attributes( W.xor_gc, 3, GDK_LINE_SOLID, GDK_CAP_BUTT, GDK_JOIN_MITER);

   /*------------------------------------------------
   ** Create timer Function for data/screen updates
   */
   Lc.app_timerid = g_timeout_add( Lc.timer_ms, timer_function, NULL);

   /*------------------------------------------------
   ** create open dialog box
	*/
	{
	   GtkWidget * dialog;
	   GtkWidget * label;
	   GtkWidget * button;
	   GtkWidget * cbox;
	   GtkWidget * entry;
	   GtkWidget * table;

		GtkAdjustment *adj;

		dialog = gtk_file_chooser_dialog_new("Open File", GTK_WINDOW(base_window),
			GTK_FILE_CHOOSER_ACTION_OPEN,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OPEN,   GTK_RESPONSE_ACCEPT, 
			NULL);

      // so dialog widget don't get destroyed when users click on [X]
      g_signal_connect( G_OBJECT(dialog), "delete_event", G_CALLBACK(dialog_delete_event), NULL );
      g_signal_connect( G_OBJECT(dialog), "destroy", G_CALLBACK(dialog_delete_event), NULL );

		// Table for custom buttons
		table  = gtk_table_new( 1, 5, TRUE);
		gtk_widget_set_size_request( GTK_WIDGET(table), 400, 80 );

      // widgets for reading Xtenssion
		button = gtk_button_new_with_label( "Read.Xtension" );
		g_signal_connect( G_OBJECT(button), "released", G_CALLBACK(file_open_read_xtension), (gpointer)NULL );
		gtk_table_attach_defaults( GTK_TABLE(table), button, 0, 1, 0, 1 );
		gtk_widget_show( button );

		adj =  (GtkAdjustment*) gtk_adjustment_new( 0, 0, 20, 1, 1, 1);
	   button = gtk_spin_button_new( adj, 1.0, 0);
		gtk_table_attach_defaults(GTK_TABLE(table), button, 1, 2, 0, 1);
		gtk_widget_show(button);
		W.file_open_xtension = button;

      // widget for selecting data buffer
		label = gtk_label_new( " Buffer:" );
		gtk_table_attach_defaults( GTK_TABLE(table), label, 3, 4, 0, 1 );
		gtk_widget_show( label );

		cbox = MyComboBoxCreate( buffer_selection );
		gtk_table_attach_defaults( GTK_TABLE(table), cbox, 4, 5, 0, 1 );
		gtk_widget_show( cbox );
		W.file_open_dialog_buffer = cbox;

      // widgets for reading "movieshow"
		button = gtk_button_new_with_label( "Movie Show" );
		g_signal_connect( G_OBJECT(button), "released", G_CALLBACK(file_open_movieshow), (gpointer)NULL );
		gtk_table_attach_defaults( GTK_TABLE(table), button, 0, 1, 1, 2 );
		gtk_widget_show( button );

      // movieshowdelay 
	   entry = gtk_entry_new( );
      g_signal_connect( G_OBJECT(entry), "activate",
		   G_CALLBACK(entry_cb), (gpointer) "MovieShowDelay" ); 
		g_signal_connect (G_OBJECT (entry), "focus-out-event", 
		   G_CALLBACK (entry_fo_cb), (gpointer) "MovieShowDelay" );
		gtk_table_attach_defaults( GTK_TABLE(table), entry, 1, 2, 1, 2 );
		sprintf( buf, "%0.3f", Lc.movie_show_delay );
      gtk_entry_set_text( GTK_ENTRY(entry), (char*) buf );
		gtk_widget_show( entry );
		W.file_open_movie_show_delay = entry;

      // add table to file dialog.
		gtk_widget_show( table );
		gtk_file_chooser_set_extra_widget( GTK_FILE_CHOOSER(dialog), table );

      // callback for file open 
		g_signal_connect( G_OBJECT(dialog), "file-activated", G_CALLBACK(file_open_activate_cb), NULL);
		gtk_dialog_set_default_response( GTK_DIALOG(dialog), GTK_RESPONSE_OK );
		g_signal_connect( G_OBJECT(dialog), "response", G_CALLBACK(file_open_response_cb), NULL);

		W.file_open_dialog = dialog;  // save dialog reference.
	}

   /*------------------------------------------------
   ** create save dialog box
	*/
	{
	   GtkWidget * dialog;
	   GtkWidget * label;
	   GtkWidget * cbox;
	   GtkWidget * table;

		dialog = gtk_file_chooser_dialog_new("Save File", GTK_WINDOW(base_window),
			GTK_FILE_CHOOSER_ACTION_SAVE,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_SAVE,   GTK_RESPONSE_ACCEPT, 
			NULL);
		#if GTK_CHECK_VERSION( 2, 8, 0 )
       gtk_file_chooser_set_do_overwrite_confirmation( GTK_FILE_CHOOSER( dialog ), TRUE);
		#endif

      // so dialog widget don't get destroyed when users click on [X]
      g_signal_connect( G_OBJECT(dialog), "delete_event", G_CALLBACK(dialog_delete_event), NULL );
      g_signal_connect( G_OBJECT(dialog), "destroy", G_CALLBACK(dialog_delete_event), NULL );

		// Table for custom buttons
		table  = gtk_table_new( 1, 5, TRUE);

      // widget for selecting buffer
		label = gtk_label_new( " Buffer:" );
		gtk_table_attach_defaults( GTK_TABLE(table), label, 3, 4, 0, 1 );
		gtk_widget_show( label );

		cbox = MyComboBoxCreate( buffer_selection );
		gtk_table_attach_defaults( GTK_TABLE(table), cbox, 4, 5, 0, 1 );
	   W.file_save_dialog_buffer_hid = 
		   g_signal_connect( G_OBJECT(cbox), "changed", G_CALLBACK(file_save_bufinx_cb), NULL );
		gtk_widget_show( cbox );
		W.file_save_dialog_buffer = cbox;

      // add table to file dialog.
		gtk_widget_show( table );
		gtk_file_chooser_set_extra_widget( GTK_FILE_CHOOSER(dialog), table );

      // call back for s
		g_signal_connect( G_OBJECT(dialog), "response", G_CALLBACK(file_save_response_cb), NULL);

		W.file_save_dialog = dialog;  // save dialog reference.
   }


   /*------------------------------------------------
   ** create Moris Guide Adjustment Dialog
	*/
	{
	   mga_create_dialog( &W.mga );
	}

   return 0;
}

/*-----------------------------------------------------------------------
**  create_mainbar_widgets() - Create an hbox with:
**    |--hbox
**        |- button menu     - application's button menu
**        |- frame
**        |   |- Colormap_w  - colormap drawing area.
**        |- cm_file_w       - widget to select colormap
**-----------------------------------------------------------------------
*/
void create_mainbar_widgets( GtkWidget ** c )
{
   GtkWidget * container;
   GtkWidget * hbox;
   GtkWidget * widget; 
   GtkWidget * da_w; 
   GtkWidget * frame;
   GtkWidget * cbox;

   hbox = gtk_hbox_new( FALSE, 0 );
   gtk_widget_show( hbox );
   
   container = gtk_frame_new( NULL );
   gtk_frame_set_shadow_type( GTK_FRAME(container), GTK_SHADOW_ETCHED_OUT );
   gtk_container_add( GTK_CONTAINER(container), hbox );
   
   /* main buttons */
   create_mainbar_button_menu( &widget );
   gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
   gtk_widget_show( widget );

   /* a frame */
   frame = gtk_frame_new( NULL );
   gtk_frame_set_shadow_type( GTK_FRAME(frame), GTK_SHADOW_ETCHED_OUT);
   gtk_box_pack_start(GTK_BOX(hbox), frame, FALSE, TRUE, 0);
   
   /* Put colormap_w in frame */
   da_w = gtk_drawing_area_new();
   gtk_widget_set_size_request( GTK_WIDGET(da_w), 200, 25);
   gtk_container_add( GTK_CONTAINER(frame), da_w);
   
   g_signal_connect( G_OBJECT(da_w), "expose_event",
      G_CALLBACK(colormap_expose_event), NULL);
   g_signal_connect( G_OBJECT(da_w), "configure_event",
      G_CALLBACK(colormap_configure_event), NULL);
   gtk_widget_show ( da_w );
   W.colormap = da_w;
   gtk_widget_show ( frame );

   /*-----------------------------------------------
   ** Option menu for selecting colormap definitions
   */
   cbox = MyComboBoxCreate( colormap_selection );
   gtk_box_pack_start(GTK_BOX(hbox), cbox, FALSE, TRUE, 0);
   W.colormap_cbox_hid = g_signal_connect( G_OBJECT(cbox), "changed", G_CALLBACK(colormap_cbox_cb), NULL
);
   gtk_widget_show(cbox);
   W.colormap_cbox = cbox;

   /* return the box reference to the user */
   *c = container;
}
 
/*-----------------------------------------------------------------------
**  create_mainbar_button_menu() - Creates a menu of buttons.
**-----------------------------------------------------------------------
*/
void create_mainbar_button_menu( GtkWidget ** c )
{
   GtkWidget * hbox;
   GtkWidget * button;

   hbox = gtk_hbox_new( FALSE, 0);

   /* File Open Button */
   button = gtk_button_new_with_label( "Open" );
   g_signal_connect( G_OBJECT(button), "clicked", G_CALLBACK(menubar_open_cb), NULL );
   gtk_box_pack_start( GTK_BOX(hbox), button, TRUE, TRUE, 0 );
   gtk_widget_show( button );

   /* File Save As Button */
   button = gtk_button_new_with_label( "Save" );
   g_signal_connect( G_OBJECT(button), "clicked", G_CALLBACK(menubar_save_cb), NULL );
   gtk_box_pack_start( GTK_BOX(hbox), button, TRUE, TRUE, 0 );
   gtk_widget_show( button );

   /* Macro Configure Box Button */
	if( Lc.use_config_window )
	{
		button = gtk_button_new_with_label( "Configure" );
		g_signal_connect( G_OBJECT(button), "clicked", G_CALLBACK(menubar_configure_cb), NULL );
		gtk_box_pack_start( GTK_BOX(hbox), button, TRUE, TRUE, 0 );
		gtk_widget_show( button );
	}

   /* Quit Button */
   button = gtk_button_new_with_label( "Quit" );
   g_signal_connect( G_OBJECT(button), "clicked", G_CALLBACK(menubar_quit_cb), NULL );
   gtk_box_pack_start( GTK_BOX(hbox), button, TRUE, TRUE, 0 );
   gtk_widget_show( button );

   /* return the box reference to the user */
   *c = hbox;
}

/********************************************************************************/
/*  Function to create Displays (DrawingArea) for data Imaging/Graphing.        */
/********************************************************************************/

/*-----------------------------------------------------------------------
**  create_main_dpydata() - create container and fills the contains up
**         with dpydata used for images display.
**    table
**      |--- (insert all display into table).
**-----------------------------------------------------------------------
*/
void create_main_dpydata( GtkWidget ** c )
{
   int i;

   GtkWidget *table;
   GtkWidget *dpy;

   table = gtk_table_new( 1, 1, TRUE );

   /* create (Lc.num_dpy-1), last DPY is assumed to be in a dialog window */
   for( i=0; i < (Lc.num_dpy-1); i++ )
   {
      int row =  i % Lc.dpy_r;
      int col =  i / Lc.dpy_r;

      create_one_dpydata( &dpy, Lc.dpy_default_wid, Lc.dpy_default_hgt, i, FALSE );
      gtk_table_attach(GTK_TABLE(table), dpy, col, col+1, row, row+1, 0, 0, 0, 0);
      gtk_widget_show (dpy);
   }

   /* return the box reference to the user */
   *c = table;
}

/*-----------------------------------------------------------------------
**  create_dialog_dpydata() - create a dpydata used for images display
**    in its own dialog window.
**-----------------------------------------------------------------------
*/
void create_dialog_dpydata ( GtkWidget ** c )
{
   GtkWidget * window;
   GtkWidget * dpy;
   char buf[25];

   window = gtk_window_new( GTK_WINDOW_TOPLEVEL );
   sprintf( buf, "DV Dpy %d", Lc.num_dpy-1);
   gtk_window_set_title( GTK_WINDOW (window), buf);
	gtk_window_set_resizable( GTK_WINDOW(window), TRUE );
   g_signal_connect( G_OBJECT(window), "delete_event", G_CALLBACK(dialog_delete_event), NULL );
   g_signal_connect( G_OBJECT(window), "destroy", G_CALLBACK(dialog_delete_event), NULL );

   create_one_dpydata( &dpy, Lc.dpy_default_wid, Lc.dpy_default_hgt, Lc.num_dpy-1, TRUE);
   gtk_container_add( GTK_CONTAINER (window), dpy );
   gtk_widget_show( dpy );

   /* return window handle to caller */
   *c = window;
}

/*-----------------------------------------------------------------------
**  create_one_dpydata() - creates one display area using:
**     |-frame
**         |- table
**              |- drawingArea
**              |- drawingArea - vbar
**              |- hbar
**-----------------------------------------------------------------------
*/
void create_one_dpydata( GtkWidget ** c, int wid, int hgt, int dpinx, int resize_it )
{
   int options;

   GtkWidget *frame;
   GtkWidget *table;
   GtkWidget *title;
   GtkWidget *data;
   GtkObject *vadj, *hadj;
   GtkWidget *vsb, *hsb;

   //printf("create_one_dpydata( %dx%d inx=%d)\n", wid, hgt, dpinx ); 

   /* create frame to surround all widgets */
   frame = gtk_frame_new( NULL );
   gtk_frame_set_shadow_type( GTK_FRAME(frame), GTK_SHADOW_ETCHED_OUT);

   /* create table & attach to frame */
   table = gtk_table_new( 3, 2, FALSE );
   gtk_container_add( GTK_CONTAINER(frame), table);

   /*----- title for canvas -----*/
   title = gtk_drawing_area_new();
	gtk_widget_set_size_request( GTK_WIDGET(title), wid, 15);
   gtk_table_attach(GTK_TABLE(table), title, 0, 3, 0, 1, GTK_SHRINK|GTK_FILL, 0, 0, 0);
   g_object_set_data( G_OBJECT(title), DPYINX_DATA_KEY, (gpointer) (intptr_t)dpinx );
   gtk_widget_show (title);

   g_signal_connect( G_OBJECT(title), "expose_event",
      G_CALLBACK(dpytitle_expose_event), NULL);
   g_signal_connect( G_OBJECT(title), "configure_event",
      G_CALLBACK(dpytitle_configure_event), NULL);

   g_signal_connect( G_OBJECT(title), "button_press_event",
      G_CALLBACK(dpytitle_button_press_event), (gpointer)(intptr_t)dpinx );

   gtk_widget_set_events ( title, GDK_BUTTON_PRESS_MASK );

   /*------------------------------------------
   ** data area for canvas.
   ** drawingArea don't normally accept focus/keypress event, so we
   ** need to add support for focus callback to support key_press events.
   */
   options = ( resize_it ?  GTK_EXPAND | GTK_SHRINK | GTK_FILL : 0 );
   data = gtk_drawing_area_new();
	gtk_widget_set_size_request( GTK_WIDGET(data), wid, hgt);
   gtk_table_attach(GTK_TABLE(table), data, 0, 1, 1, 2, options, options, 0, 0 );
   g_object_set_data( G_OBJECT(data), DPYINX_DATA_KEY, (gpointer)(intptr_t) dpinx );
   gtk_widget_show(data);

   g_signal_connect( G_OBJECT(data), "expose_event",
      G_CALLBACK(dpydata_expose_event), NULL);
   g_signal_connect( G_OBJECT(data), "configure_event",
      G_CALLBACK(dpydata_configure_event), NULL);

   /* IO events to watch for */
   g_signal_connect( G_OBJECT(data), "button_press_event",
      G_CALLBACK(dpydata_button_press_event), (gpointer)(intptr_t)dpinx );
   g_signal_connect( G_OBJECT(data), "motion_notify_event",
      G_CALLBACK(dpydata_motion_notify_event), (gpointer)(intptr_t)dpinx );

   g_signal_connect( G_OBJECT(data), "key_press_event",
      G_CALLBACK(dpydata_key_press_event), (gpointer)(intptr_t)dpinx );

   g_signal_connect( G_OBJECT(data), "focus_in_event",
      G_CALLBACK(dpydata_focus_in_event), (gpointer)(intptr_t)dpinx );
   g_signal_connect( G_OBJECT(data), "focus_out_event",
      G_CALLBACK(dpydata_focus_out_event), (gpointer)(intptr_t)dpinx );

   gtk_widget_set_events ( data, GDK_EXPOSURE_MASK
      | GDK_LEAVE_NOTIFY_MASK
      | GDK_BUTTON_PRESS_MASK
      | GDK_KEY_PRESS_MASK
      | GDK_FOCUS_CHANGE_MASK
      | GDK_POINTER_MOTION_MASK
      | GDK_POINTER_MOTION_HINT_MASK);

   GTK_WIDGET_SET_FLAGS( data, GTK_CAN_FOCUS );

   /*----- Vertical scroll bar -----*/
   vadj = gtk_adjustment_new( 0, 0, Lc.dpy_default_hgt, 1, Lc.dpy_default_hgt, Lc.dpy_default_hgt );
   vsb  = gtk_vscrollbar_new( GTK_ADJUSTMENT(vadj) );
   gtk_range_set_update_policy( GTK_RANGE(vsb), GTK_UPDATE_DISCONTINUOUS);
   g_signal_connect(G_OBJECT(vadj), "value_changed", G_CALLBACK(dpy_vadj_cb), (gpointer)(intptr_t)dpinx);
   gtk_table_attach(GTK_TABLE(table), vsb, 1, 2, 1, 2, 0, GTK_SHRINK|GTK_FILL, 0, 0);
   gtk_widget_show ( vsb );

   /*----- horizontial  scroll bar -----*/
   hadj = gtk_adjustment_new( 0, 0, Lc.dpy_default_wid, 1, Lc.dpy_default_wid, Lc.dpy_default_wid );
   hsb  = gtk_hscrollbar_new( GTK_ADJUSTMENT(hadj) );
   gtk_range_set_update_policy( GTK_RANGE(hsb), GTK_UPDATE_DISCONTINUOUS);
   g_signal_connect(G_OBJECT(hadj), "value_changed", G_CALLBACK(dpy_hadj_cb), (gpointer)(intptr_t)dpinx);
   gtk_table_attach(GTK_TABLE(table), hsb, 0, 1, 2, 3, GTK_SHRINK|GTK_FILL, 0, 0, 0);
   gtk_widget_show ( hsb );

   /* show table, save reference to title, data, and adjustments. Return frame reference */
   gtk_widget_show (table);
   Dpy[dpinx].title_drawingarea = title;
   Dpy[dpinx].data_drawingarea = data;
   Dpy[dpinx].vadj = GTK_ADJUSTMENT(vadj);
   Dpy[dpinx].hadj = GTK_ADJUSTMENT(hadj);
   *c = frame;
}
/********************************************************************************************/
/*  Configure Dialog: Display Options, Math, Setup, Offset, CommandIO, etc.                 */
/********************************************************************************************/

/*-----------------------------------------------------------------------
**  create_configure_widgets() - create configure widgets 
**-----------------------------------------------------------------------
*/
void create_configure_widgets( GtkWidget ** c, GtkWidget *base_window )
{
   GtkWidget *notebook,
	          *label,
				 *widget;

   // notebook 
	notebook = gtk_notebook_new();

   // Display Options
   create_display_widgets( &widget );
   label = gtk_label_new( "Display Options" );
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), widget, label);
	gtk_widget_show(widget);

   // Math
   create_math_widgets( &widget );
   label = gtk_label_new( "Math" );
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), widget, label);
	gtk_widget_show(widget);

   // Offset
   create_offset_widgets( &widget );
   label = gtk_label_new( "Offset" );
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), widget, label);
	gtk_widget_show(widget);

   // Macros
	create_macro_widgets( &widget );
   label = gtk_label_new( "Macros" );
   gtk_notebook_append_page(GTK_NOTEBOOK(notebook), widget, label);
   gtk_widget_show( widget );

   // Setup
   create_setup_widgets( &widget );
   label = gtk_label_new( "Setup" );
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), widget, label);
	gtk_widget_show(widget);

   // CommandIO
   create_cmdio_widgets(&widget, base_window);
   label = gtk_label_new( "CommandIO" );
   gtk_notebook_append_page(GTK_NOTEBOOK(notebook), widget, label);
   gtk_widget_show( widget );

   // About DV
	create_about_widgets( &widget );
   label = gtk_label_new( "About DV" );
   gtk_notebook_append_page(GTK_NOTEBOOK(notebook), widget, label);
   gtk_widget_show( widget );

   *c = notebook;
}

/*-----------------------------------------------------------------------
**  create_cmdio_widgets() - create a main command line interface.
**-----------------------------------------------------------------------
*/
void create_cmdio_widgets(GtkWidget ** c, GtkWidget *base_window)
{
   GtkWidget *vbox;

   vbox = gtk_vbox_new( FALSE, 0);
   gtk_widget_set_size_request( GTK_WIDGET(vbox), 550, 200);

   W.cmd.main_console = cc_create_console(GTK_WINDOW(base_window));
   cc_set_entry_callback(W.cmd.main_console, (cc_cmd_callback)cmd_execute, TRUE);
   gtk_box_pack_start(GTK_BOX(vbox), W.cmd.main_console->container, TRUE, TRUE, 0);
   gtk_widget_show_all(W.cmd.main_console->container);
   gtk_widget_show( W.cmd.main_console->container);

   *c = vbox;
}

/*-----------------------------------------------------------------------
**  create_display_widgets() - create widgets for main parameter container.
**  vbox
**   |--hbox  (W.dpyactive_w, W.dpytype_w, W.dpybuf_w).
**   |--hbox
**-----------------------------------------------------------------------
*/
void create_display_widgets( GtkWidget ** c )
{
   GtkWidget * vbox;
   GtkWidget * hbox;
   GtkWidget * widget;
   GtkWidget * table;
   GtkWidget * cbox;

   int i;
   char buf[10];

   vbox = gtk_vbox_new( FALSE, 0);

   /*--------------------------------------------
   ** hbox - for W.dpyactive_w, W.dpytype_w, W.dpybuf_w
   */
   hbox = gtk_hbox_new( FALSE, 0);
   gtk_box_pack_start( GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

   /*------------------------------------------------
   ** create dpyactive_w as a set of button in table
   */
   widget = gtk_label_new( "ActiveDpy:" );
   gtk_box_pack_start( GTK_BOX(hbox), widget, FALSE, FALSE, 0 );
   gtk_widget_show( widget );

   table = gtk_table_new( 2, 2, TRUE );
   gtk_box_pack_start( GTK_BOX(hbox), table, FALSE, FALSE, 0 );
   for( i = 0; i < Lc.num_dpy; i++ )
   {
      int row = i % Lc.dpy_r;
      int col = i / Lc.dpy_r; 
      sprintf(buf, " %d ", i);
      widget = gtk_toggle_button_new_with_label( buf );
      gtk_table_attach_defaults( GTK_TABLE(table), widget, col, col+1, row, row+1 );
      g_signal_connect( G_OBJECT(widget), "released",
         G_CALLBACK(dpyactive_cb), (gpointer) ((intptr_t)i) );
      gtk_widget_show( widget );
      W.dpyactive_w[i] = widget;
   }
   gtk_widget_show( table );

   /*------------------------------------------------
   ** create Dyptype_w as an option menu
   */
   widget = gtk_label_new( " DisplayType:");
   gtk_box_pack_start( GTK_BOX(hbox), widget, FALSE, FALSE, 0);
   gtk_widget_show( widget );

	cbox = MyComboBoxCreate( dpytype_selection );
   gtk_box_pack_start(GTK_BOX(hbox), cbox, FALSE, FALSE, 0);
	W.dpytype_hid = g_signal_connect( G_OBJECT(cbox), "changed", G_CALLBACK(dpytype_cb), NULL );
	gtk_widget_show(cbox);
	W.dpytype_w = cbox;

   gtk_widget_show( hbox );

   /*------------------------------------------------
   ** create Buffer_w as a set of button in table
   */
   widget = gtk_label_new( " Buffer:");
   gtk_box_pack_start( GTK_BOX(hbox), widget, FALSE, FALSE, 0 );
   gtk_widget_show( widget );

   table = gtk_table_new( 2, 2, TRUE );
   gtk_box_pack_start( GTK_BOX(hbox), table, FALSE, FALSE, 0 );
   for( i = 0; i < NUM_BUFFER; i++ )
   {
      int row = i % Lc.dpy_r; 
      int col = i / Lc.dpy_r;
      sprintf(buf, " %c ", 'A'+i);
      widget = gtk_toggle_button_new_with_label( buf );
      gtk_table_attach_defaults( GTK_TABLE(table), widget, col, col+1, row, row+1 );
      g_signal_connect( G_OBJECT(widget), "released",
         G_CALLBACK(buffer_cb), (gpointer) 'A'+i );
      gtk_widget_show( widget );
      W.dpybuf_w[i] = widget;
   }
   gtk_widget_show( table );

   /*------------------------------------------------
   ** create a notebook for the display option parameters
   */
   create_display_parameters_notebook( &widget );
   gtk_box_pack_start( GTK_BOX(vbox), widget, TRUE, TRUE, 0 );
   gtk_widget_show( widget );

   /* return the box reference to the user */
   *c = vbox;
}

/*-----------------------------------------------------------------------
**  create_math_widgets() - create math operations/equations widgets on main notebook.
**-----------------------------------------------------------------------
*/
void create_math_widgets( GtkWidget ** c )
{
   GtkWidget * table;
   GtkWidget * entry;
   GtkWidget * label;
   GtkWidget * cbox;
   GtkWidget * button;

   int i, row;
   char * equation[] =
   {
      "c = a - b",
      "c = c - d",
      "a = a * 10",
      NULL,
   };

   table = gtk_table_new( 7, 20, TRUE );
   gtk_widget_set_size_request( GTK_WIDGET(table), 500, 125 );

   /*----------------------------------
   ** 3 execute button/entry widgets
   */
   row = 1;
   for ( i = 0; i < NUM_MATH_EQU; i++ )
   {
      row = (i*2)+1;

      /* entry widget */
      entry = gtk_entry_new( );
      gtk_entry_set_text( GTK_ENTRY(entry), equation[i] );
      gtk_table_attach_defaults( GTK_TABLE(table), entry, 4, 8, row, row+1 );
      gtk_widget_show(entry);
      W.math_equ_w[i] = entry;

      /* button to execute equation in entry widget */
      button = gtk_button_new_with_label( "Execute"  );
      gtk_table_attach_defaults( GTK_TABLE(table), button, 1, 4, row, row+1 );
      gtk_widget_show( button );
      g_signal_connect( G_OBJECT(button), "released", G_CALLBACK(math_equ_cb), (gpointer)entry);
   }

   /*------------------------
   ** copy
   */
   button = gtk_button_new_with_label( "Copy" );
   gtk_table_attach_defaults( GTK_TABLE(table), button, 10, 12, 1, 2 );
   gtk_widget_show( button );
   g_signal_connect( G_OBJECT(button), "released", G_CALLBACK(math_copy_button_cb), (gpointer)NULL );

   cbox = MyComboBoxCreate( buffer_selection );
   gtk_table_attach_defaults( GTK_TABLE(table), cbox, 12, 14, 1, 2 );
	gtk_widget_show(cbox);
	W.math_copy_from_bufinx = cbox;

   label = gtk_label_new( "to" );
   gtk_table_attach_defaults( GTK_TABLE(table), label, 14, 15, 1, 2 );
   gtk_widget_show( label );

   cbox = MyComboBoxCreate( buffer_selection );
   gtk_table_attach_defaults( GTK_TABLE(table), cbox, 15, 17, 1, 2 );
	gtk_widget_show(cbox);
	W.math_copy_to_bufinx = cbox;

   /*------------------------
   ** clear
   */
   button = gtk_button_new_with_label( "Clear" );
   gtk_table_attach_defaults( GTK_TABLE(table), button, 10, 12, 3, 4 );
   gtk_widget_show( button );
   g_signal_connect( G_OBJECT(button), "released", G_CALLBACK(math_clear_button_cb), (gpointer)NULL );

   cbox = MyComboBoxCreate( buffer_selection );
   gtk_table_attach_defaults( GTK_TABLE(table), cbox, 12, 14, 3, 4 );
	gtk_widget_show(cbox);
	W.math_clear_bufinx = cbox;

   /*------------------------
   ** rotate
   */
   button = gtk_button_new_with_label( "Rotate" );
   gtk_table_attach_defaults( GTK_TABLE(table), button, 10, 12, 5, 6 );
   gtk_widget_show( button );
   g_signal_connect( G_OBJECT(button), "released", G_CALLBACK(math_rotate_button_cb), (gpointer)NULL );

   cbox = MyComboBoxCreate( buffer_selection );
   gtk_table_attach_defaults( GTK_TABLE(table), cbox, 12, 14, 5, 6 );
	gtk_widget_show(cbox);
	W.math_rotate_bufinx = cbox;

   cbox = MyComboBoxCreate( rotate_selection );
   gtk_table_attach_defaults( GTK_TABLE(table), cbox, 15, 17, 5, 6 );
	gtk_widget_show(cbox);
	W.math_rotate_ops = cbox;

   /* return the table reference to the user */
   *c = table;
}

/*-----------------------------------------------------------------------
**  create_setup_widgets() - create the setup options main notebook page.
**-----------------------------------------------------------------------
*/
void create_setup_widgets( GtkWidget ** c )
{
   GtkWidget * table;
   GtkWidget * entry;
   GtkWidget * label;
   GtkWidget * checkbutton;
   GtkWidget * cbox;
   GtkWidget * button;

   table = gtk_table_new( 7, 20, TRUE );
   gtk_widget_set_size_request( GTK_WIDGET(table), 500, 125 );

   /*-------------------------------
   **  Show DV Port 
   */
   label = gtk_label_new( "Dv Port Label" );
   gtk_misc_set_alignment( GTK_MISC(label), 0, 0.5 );
   gtk_table_attach_defaults( GTK_TABLE(table), label, 2, 9, 0, 1 );
   gtk_widget_show( label );
	W.dvport_w = label;

   /*-------------------------------
   **  div by Divisor
   */
   checkbutton = gtk_check_button_new_with_label("Div By Divisor");
   g_signal_connect( G_OBJECT(checkbutton), "toggled",
      G_CALLBACK (checkbutton_offon_cb), (gpointer) "DivByDivisor" );
   gtk_table_attach_defaults (GTK_TABLE(table), checkbutton, 4, 9, 1, 2 );
   gtk_widget_show(checkbutton);
   W.div_by_divisor = checkbutton;

   /*-------------------------------
   **  tcs system & hostname
   */
   label = gtk_label_new( "TCS Hostname: " );
   gtk_misc_set_alignment( GTK_MISC(label), 1, 0.5 );
   gtk_table_attach_defaults( GTK_TABLE(table), label, 0, 4, 2, 3 );
   gtk_widget_show( label );

   entry = gtk_entry_new( );
   g_signal_connect( G_OBJECT(entry), "activate", 
			 G_CALLBACK(entry_cb), (gpointer) "TCSHostname" );
	g_signal_connect (G_OBJECT (entry), "focus-out-event",
			 G_CALLBACK (entry_fo_cb), (gpointer) "TCSHostname" );
   gtk_table_attach_defaults( GTK_TABLE(table), entry, 4, 9, 2, 3 );
   gtk_widget_show(entry);
   W.tcs_hostname = entry;

   /*-------------------------------
   **  printer name
   */
   label = gtk_label_new( "Printer: " );
   gtk_misc_set_alignment( GTK_MISC(label), 1, 0.5 );
   gtk_table_attach_defaults( GTK_TABLE(table), label, 0, 4, 3, 4 );
   gtk_widget_show( label );

   entry = gtk_entry_new( );
   g_signal_connect( G_OBJECT(entry), "activate", 
			 G_CALLBACK(entry_cb), (gpointer) "Printer" );
	g_signal_connect (G_OBJECT (entry), "focus-out-event",
			 G_CALLBACK (entry_fo_cb), (gpointer) "Printer" );
   gtk_table_attach_defaults( GTK_TABLE(table), entry, 4, 9, 3, 4 );
   gtk_widget_show(entry);
   W.printer_name = entry;

   /*-------------------------------
   **  PrinterType
   */
   label = gtk_label_new( "Printer Type: " );
   gtk_misc_set_alignment( GTK_MISC(label), 1, 0.5 );
   gtk_table_attach_defaults( GTK_TABLE(table), label, 0, 4, 4, 5 );
   gtk_widget_show( label );

	cbox = MyComboBoxCreate( printer_selection );
   gtk_table_attach_defaults( GTK_TABLE(table), cbox, 4, 9, 4, 5 );
	W.printer_type_hid = g_signal_connect( G_OBJECT(cbox), "changed", 
	                                        G_CALLBACK(printer_type_cb), NULL);
	gtk_widget_show(cbox);
	W.printer_type = cbox;

   /*-------------------------------
   **  Print To File
   */
   checkbutton = gtk_check_button_new_with_label("Print to File");
   g_signal_connect( G_OBJECT(checkbutton), "toggled",
      G_CALLBACK (checkbutton_offon_cb), (gpointer) "PrintToFile" );
   gtk_table_attach_defaults (GTK_TABLE(table), checkbutton, 4, 9, 5, 6 );
   gtk_widget_show(checkbutton);
   W.print_to_file = checkbutton;

   /*-------------------------------
   **  Default Path
   */
   label = gtk_label_new( "(default) Path: " );
   gtk_misc_set_alignment( GTK_MISC(label), 1, 0.5 );
   gtk_table_attach_defaults( GTK_TABLE(table), label, 0, 4, 6, 7 );
   gtk_widget_show( label );

   entry = gtk_entry_new( );
   g_signal_connect( G_OBJECT(entry), "activate", 
		G_CALLBACK(entry_cb), (gpointer) "Path" );
	g_signal_connect (G_OBJECT (entry), "focus-out-event",
		G_CALLBACK (entry_fo_cb), (gpointer) "Path" );
   gtk_table_attach_defaults( GTK_TABLE(table), entry, 4, 15, 6, 7 );
   gtk_widget_show(entry);
   W.path = entry;

   /*---------------------------------------------------
   **  Button to bring up browse for (data) path dialog
   */
   button = gtk_button_new_with_label("Browse Path...");
   g_signal_connect( G_OBJECT(button), "clicked", G_CALLBACK (browse_data_path_cb), 
	   (gpointer) &W.bfp_data_path );
   gtk_table_attach_defaults( GTK_TABLE(table), button, 15, 19, 6, 7);
   gtk_widget_show(button);

   // creates the macro Browse for Path
   bfp_create_dialog( &W.bfp_data_path, "Browse for Data Path");
	bfp_set_ok_cb( &W.bfp_data_path, G_CALLBACK(bfp_data_path_ok_cb) );    /* assign a cb to the OK button */

   /*-------------------------------
   **  cm.reverse. button
   */
   button = gtk_button_new_with_label("ColorMap.Inverse");
   g_signal_connect( G_OBJECT(button), "clicked", G_CALLBACK(button_cb),
      (gpointer) "Colormap.Inverse" );
   gtk_table_attach_defaults( GTK_TABLE(table), button, 12, 18, 0, 1);
   gtk_widget_show( button );

   /*-------------------------------
   **  cm.Swap.Longs
   */
   checkbutton = gtk_check_button_new_with_label("CM.Swap.RGB");
   g_signal_connect( G_OBJECT(checkbutton), "toggled",
      G_CALLBACK (checkbutton_offon_cb), (gpointer) "CM.Swap.RGB" );
   gtk_table_attach_defaults (GTK_TABLE(table), checkbutton, 12, 18, 1, 2);
   gtk_widget_show(checkbutton);
   W.cm_swap_rgb = checkbutton;

   /*-------------------------------
   **  Inst Flavor
   */
   label = gtk_label_new( "Inst.Flavor: " );
   gtk_misc_set_alignment( GTK_MISC(label), 1, 0.5 );
   gtk_table_attach_defaults( GTK_TABLE(table), label, 10, 14, 2, 3 );
   gtk_widget_show( label );

	cbox = MyComboBoxCreate( inst_flavor_selection );
   gtk_table_attach_defaults( GTK_TABLE(table), cbox, 14, 18, 2, 3 );
	W.inst_flavor_hid = g_signal_connect( G_OBJECT(cbox), "changed", G_CALLBACK(inst_flavor_cb), NULL);
	gtk_widget_show(cbox);
	W.inst_flavor = cbox;

   /*-------------------------------
   **  Image Compass
   */
   label = gtk_label_new( "ImageCompass: " );
   gtk_misc_set_alignment( GTK_MISC(label), 1, 0.5 );
   gtk_table_attach_defaults( GTK_TABLE(table), label, 10, 14, 3, 4 );
   gtk_widget_show( label );

   checkbutton = gtk_check_button_new_with_label("Show");
   g_signal_connect( G_OBJECT(checkbutton), "toggled",
      G_CALLBACK (checkbutton_offon_cb), (gpointer) "ImageCompass" );
   gtk_table_attach_defaults (GTK_TABLE(table), checkbutton, 14, 17, 3, 4 );
   gtk_widget_show(checkbutton);
   W.image_compass = checkbutton;

   /*-------------------------------
   **  Image Compass FlipNS
   */
   checkbutton = gtk_check_button_new_with_label("FlipNS");
   g_signal_connect( G_OBJECT(checkbutton), "toggled",
      G_CALLBACK (checkbutton_offon_cb), (gpointer) "ImageCompass.FlipNS" );
   gtk_table_attach_defaults (GTK_TABLE(table), checkbutton, 17, 20, 3, 4 );
   gtk_widget_show(checkbutton);
   W.image_compass_flip_NS = checkbutton;

   /*-------------------------------
   **  Image Show Guide Box
   */
   checkbutton = gtk_check_button_new_with_label("Image Show GBox");
   g_signal_connect( G_OBJECT(checkbutton), "toggled",
      G_CALLBACK (checkbutton_offon_cb), (gpointer) "ImageShowGBox" );
   gtk_table_attach_defaults (GTK_TABLE(table), checkbutton, 12, 19, 4, 5 );
   gtk_widget_show(checkbutton);
   W.image_show_gbox = checkbutton;

   /*-------------------------------
   **  GBox.Cmd.Right.Click
   */
   checkbutton = gtk_check_button_new_with_label("GBox.Cmd.Right.Click");
   g_signal_connect( G_OBJECT(checkbutton), "toggled",
      G_CALLBACK (checkbutton_offon_cb), (gpointer) "GBox.Cmd.Right.Click" );
   gtk_table_attach_defaults (GTK_TABLE(table), checkbutton, 12, 19, 5, 6 );
   gtk_widget_show(checkbutton);
   W.gbox_cmd_right_click = checkbutton;

   /* return the table reference to the user */
   *c = table;
}

/*-----------------------------------------------------------------------
**  create_offset_widgets() - create the TCS offset notebook page.
**-----------------------------------------------------------------------
*/
void create_offset_widgets( GtkWidget ** c )
{
   GtkWidget * table;
   GtkWidget * entry;
   GtkWidget * label;
   GtkWidget * button;
   GtkWidget * checkbutton;

   table = gtk_table_new( 7, 20, TRUE );
   gtk_widget_set_size_request( GTK_WIDGET(table), 500, 125 );

   /*-------------------------------
   **  Set defaults buttons
   */
	// removed these (all data should be in FITS header for IRTF instruments)

   /*-------------------------------
   **  use_fits_angle_scale
   */
   checkbutton = gtk_check_button_new_with_label("Use FITS Angle&Scale");
   g_signal_connect( G_OBJECT(checkbutton), "toggled",
      G_CALLBACK (checkbutton_offon_cb), (gpointer) "UseFITSAngle&Scale" );
   gtk_table_attach_defaults (GTK_TABLE(table), checkbutton, 4, 11, 0, 1);
   gtk_widget_show(checkbutton);
   W.use_fits_angle_scale = checkbutton;
	gtk_widget_set_tooltip_text( checkbutton,
	     "When checked, the FITS header is used to update the Angle and PlateScale. "
		  "Uncheck this box, to manually enter these values.");

   /*-------------------------------
   **  angle text entry
   */
   label = gtk_label_new( "Angle: " );
   gtk_misc_set_alignment( GTK_MISC(label), 1, 0.5 );
   gtk_table_attach_defaults( GTK_TABLE(table), label, 1, 4, 2, 3 );
   gtk_widget_show( label );

   entry = gtk_entry_new( );
   gtk_entry_set_text( GTK_ENTRY(entry), "270" );
   g_signal_connect( G_OBJECT(entry), "activate", 
							 G_CALLBACK(entry_cb), (gpointer) "Offset.angle" );
	g_signal_connect (G_OBJECT (entry), "focus-out-event", 
							 G_CALLBACK (entry_fo_cb), (gpointer) "Offset.angle" );
   gtk_table_attach_defaults( GTK_TABLE(table), entry, 4, 9, 2, 3 );
   gtk_widget_show(entry);
   W.offset_angle = entry;

   /*-------------------------------
   **  plate scale text entry
   */
   label = gtk_label_new( "Plate Scale: " );
   gtk_misc_set_alignment( GTK_MISC(label), 1, 0.5 );
   gtk_table_attach_defaults( GTK_TABLE(table), label, 1, 4, 3, 4 );
   gtk_widget_show( label );

   entry = gtk_entry_new( );
   gtk_entry_set_text( GTK_ENTRY(entry), "0.20" );
   g_signal_connect( G_OBJECT(entry), "activate", 
							  G_CALLBACK(entry_cb), (gpointer) "Offset.platescale" );
	g_signal_connect (G_OBJECT (entry), "focus-out-event", 
							 G_CALLBACK (entry_fo_cb), (gpointer) "Offset.platescale" );
   gtk_table_attach_defaults( GTK_TABLE(table), entry, 4, 9, 3, 4 );
   gtk_widget_show(entry);
   W.offset_platscale = entry;

   /*-------------------------------
   **  beginning coordinates text entry
   */
   label = gtk_label_new( "From (x,y): " );
   gtk_misc_set_alignment( GTK_MISC(label), 1, 0.5 );
   gtk_table_attach_defaults( GTK_TABLE(table), label, 9, 14, 2, 3);
   gtk_widget_show( label );

   entry = gtk_entry_new( );
   gtk_entry_set_text( GTK_ENTRY(entry), "50 125" );
   g_signal_connect( G_OBJECT(entry), "activate", 
							  G_CALLBACK(entry_cb), (gpointer) "Offset.BegXY" );
	g_signal_connect (G_OBJECT (entry), "focus-out-event", 
							 G_CALLBACK (entry_fo_cb), (gpointer) "Offset.BegXY" );
   gtk_table_attach_defaults( GTK_TABLE(table), entry, 14, 18, 2, 3);
   gtk_widget_show(entry);
   W.offset_beg_xy = entry;

   /*-------------------------------
   **  end coordinates text entry
   */
   label = gtk_label_new( "to (x,y): " );
   gtk_misc_set_alignment( GTK_MISC(label), 1, 0.5 );
   gtk_table_attach_defaults( GTK_TABLE(table), label, 9, 14, 3, 4 );
   gtk_widget_show( label );

   entry = gtk_entry_new( );
   gtk_entry_set_text( GTK_ENTRY(entry), "50 125" );
   g_signal_connect( G_OBJECT(entry), "activate", 
							  G_CALLBACK(entry_cb), (gpointer) "Offset.EndXY" );
	g_signal_connect( G_OBJECT (entry), "focus-out-event", 
							  G_CALLBACK (entry_fo_cb), (gpointer) "Offset.EndXY" );
   gtk_table_attach_defaults( GTK_TABLE(table), entry, 14, 18, 3, 4 );
   gtk_widget_show(entry);
   W.offset_end_xy = entry;

   /*-------------------------------
   **  send offset to TCS button
   */
   button = gtk_button_new_with_label( "Offset Telescope" );
   g_signal_connect( G_OBJECT(button), "clicked", G_CALLBACK(button_cb),
      (gpointer) "Offset.TCS" );
   gtk_table_attach_defaults( GTK_TABLE(table), button, 0, 6, 4, 5 );
	gtk_widget_set_tooltip_text( button,
	      "Increment the TCS User offset values ");
   gtk_widget_show( button );

   /*-------------------------------
   **  send AB.offset to TCS 
	*/
   button = gtk_button_new_with_label( "Send AB Beam Offsets " );
   g_signal_connect( G_OBJECT(button), "clicked", G_CALLBACK(button_cb),
      (gpointer) "OffsetAB.TCS" );
   gtk_table_attach_defaults( GTK_TABLE(table), button, 14, 20, 4, 5 );
	gtk_widget_set_tooltip_text( button,
	      "Set the TCS beamswitch offset values ");
   gtk_widget_show( button );


   /*-------------------------------
   **  Offset in RA and DEC
   */
   label = gtk_label_new( "Offsets are:" );
   gtk_misc_set_alignment( GTK_MISC(label), 1, 0.5 );
   gtk_table_attach_defaults( GTK_TABLE(table), label, 6, 9, 4, 5 );
   gtk_widget_show( label );

   label = gtk_label_new( "0 0" );
   gtk_table_attach_defaults( GTK_TABLE(table), label, 9, 14, 4, 5 );
   gtk_widget_show( label );
   W.offset_radec   = label;

   /*-------------------------------
   **  show Moris Guide Adj Dialog
	*/
   button = gtk_button_new_with_label( "MORIS GuideBox Adj" );
   g_signal_connect( G_OBJECT(button), "clicked", G_CALLBACK(mga_show_button_cb), (gpointer) NULL);
   gtk_table_attach_defaults( GTK_TABLE(table), button, 14, 20, 6, 7 );
	if( Lc.mga_enable)
		gtk_widget_show( button );
	gtk_widget_set_tooltip_text( button,
	      "Brings up the MORIS GuideBox Adj Dialog  "
			"to center MORIS's guidebox and Object on SPeX's slit.");

   /* return the table reference to the user */
   *c = table;
}


/*-----------------------------------------------------------------------
**  create_display_parameters_notebook() - create a notebook for the display option parameters.
**  vbox
**   |--hbox
**-----------------------------------------------------------------------
*/
void create_display_parameters_notebook( GtkWidget ** c )
{
   GtkWidget * notebook;
   GtkWidget * page;
   GtkWidget * frame;

   notebook = gtk_notebook_new( );
   gtk_notebook_set_tab_pos( GTK_NOTEBOOK( notebook ), GTK_POS_TOP );
   gtk_notebook_set_show_tabs( GTK_NOTEBOOK( notebook ), FALSE );

   frame = gtk_frame_new( NULL );
   gtk_frame_set_shadow_type( GTK_FRAME(frame), GTK_SHADOW_NONE );
   create_display_image_widgets( &page );
   gtk_container_add( GTK_CONTAINER(frame), page );
   gtk_notebook_append_page( GTK_NOTEBOOK(notebook), frame, NULL );
   gtk_notebook_set_current_page( GTK_NOTEBOOK(notebook), 0 );
   gtk_widget_show( frame );
   gtk_widget_show( page );

   frame = gtk_frame_new( NULL );
   gtk_frame_set_shadow_type( GTK_FRAME(frame), GTK_SHADOW_NONE );
   create_display_header_widgets( &page );
   gtk_container_add( GTK_CONTAINER(frame), page );
   gtk_notebook_append_page( GTK_NOTEBOOK(notebook), frame, NULL );
   gtk_widget_show( frame );
   gtk_widget_show( page );

   frame = gtk_frame_new( NULL );
   gtk_frame_set_shadow_type( GTK_FRAME(frame), GTK_SHADOW_NONE );
   create_display_histogram_widgets( &page );
   gtk_container_add( GTK_CONTAINER(frame), page );
   gtk_notebook_append_page( GTK_NOTEBOOK(notebook), frame, NULL );
   gtk_widget_show( frame );
   gtk_widget_show( page );

   frame = gtk_frame_new( NULL );
   gtk_frame_set_shadow_type( GTK_FRAME(frame), GTK_SHADOW_NONE );
   create_display_linecut_widgets( &page );
   gtk_container_add( GTK_CONTAINER(frame), page );
   gtk_notebook_append_page( GTK_NOTEBOOK(notebook), frame, NULL );
   gtk_widget_show( frame );
   gtk_widget_show( page );

   frame = gtk_frame_new( NULL );
   gtk_frame_set_shadow_type( GTK_FRAME(frame), GTK_SHADOW_NONE );
   create_display_xlinecut_widgets( &page );
   gtk_container_add( GTK_CONTAINER(frame), page );
   gtk_notebook_append_page( GTK_NOTEBOOK(notebook), frame, NULL );
   gtk_widget_show( frame );
   gtk_widget_show( page );

   frame = gtk_frame_new( NULL );
   gtk_frame_set_shadow_type( GTK_FRAME(frame), GTK_SHADOW_NONE );
   create_display_noise_widgets( &page );
   gtk_container_add( GTK_CONTAINER(frame), page );
   gtk_notebook_append_page( GTK_NOTEBOOK(notebook), frame, NULL );
   gtk_widget_show( frame );
   gtk_widget_show( page );

   frame = gtk_frame_new( NULL );
   gtk_frame_set_shadow_type( GTK_FRAME(frame), GTK_SHADOW_NONE );
   create_display_pointer_widgets( &page );
   gtk_container_add( GTK_CONTAINER(frame), page );
   gtk_notebook_append_page( GTK_NOTEBOOK(notebook), frame, NULL );
   gtk_widget_show( frame );
   gtk_widget_show( page );

   frame = gtk_frame_new( NULL );
   gtk_frame_set_shadow_type( GTK_FRAME(frame), GTK_SHADOW_NONE );
   create_display_stats_widgets( &page );
   gtk_container_add( GTK_CONTAINER(frame), page );
   gtk_notebook_append_page( GTK_NOTEBOOK(notebook), frame, NULL );
   gtk_widget_show( frame );
   gtk_widget_show( page );

   frame = gtk_frame_new( NULL );
   gtk_frame_set_shadow_type( GTK_FRAME(frame), GTK_SHADOW_NONE );
   create_display_aofig_widgets( &page );
   gtk_container_add( GTK_CONTAINER(frame), page );
   gtk_notebook_append_page( GTK_NOTEBOOK(notebook), frame, NULL );
   gtk_widget_show( frame );
   gtk_widget_show( page );

   frame = gtk_frame_new( NULL );
   gtk_frame_set_shadow_type( GTK_FRAME(frame), GTK_SHADOW_NONE );
   create_display_spectra_a_widgets( &page );
   gtk_container_add( GTK_CONTAINER(frame), page );
   gtk_notebook_append_page( GTK_NOTEBOOK(notebook), frame, NULL );
   gtk_widget_show( frame );
   gtk_widget_show( page );

   frame = gtk_frame_new( NULL );
   gtk_frame_set_shadow_type( GTK_FRAME(frame), GTK_SHADOW_NONE );
   create_display_spectra_b_widgets( &page );
   gtk_container_add( GTK_CONTAINER(frame), page );
   gtk_notebook_append_page( GTK_NOTEBOOK(notebook), frame, NULL );
   gtk_widget_show( frame );
   gtk_widget_show( page );

   W.dpy_notebook = notebook;
   /* return the notebook reference to the user */
   *c = notebook;
}

/*-----------------------------------------------------------------------
**  create_display_image_widgets() - create the image parameters widgets.
**-----------------------------------------------------------------------
*/
void create_display_image_widgets( GtkWidget ** c )
{
   GtkWidget * table;
   GtkWidget * label;
   GtkWidget * entry;
   GtkWidget * hbox;
   GtkWidget * button;
   GtkWidget * hscale;
   GtkObject * adj;

   int x, i, hid;

   table = gtk_table_new( 5, 20, TRUE );
   gtk_widget_set_size_request( GTK_WIDGET(table), 475, 125 );

   /*------------------------------
   ** Zoom
   */
   label = gtk_label_new( "Zoom: " );
   gtk_misc_set_alignment( GTK_MISC(label), 1, 0.5 );
   gtk_table_attach_defaults( GTK_TABLE(table), label, 0, 2, 0, 1 );
   gtk_widget_show( label );

   adj = gtk_adjustment_new( 1, -20, 20, 1, 1, 0);
   hid = g_signal_connect( G_OBJECT (adj), "value_changed", G_CALLBACK(image_zoom_cb), "ImageZoom");

   hscale = gtk_hscale_new( GTK_ADJUSTMENT (adj));
   gtk_range_set_update_policy( GTK_RANGE(hscale), GTK_UPDATE_DISCONTINUOUS);
   gtk_scale_set_digits( GTK_SCALE(hscale), 0);
   gtk_scale_set_value_pos( GTK_SCALE(hscale), GTK_POS_LEFT);
   gtk_scale_set_draw_value( GTK_SCALE(hscale), TRUE);
   gtk_table_attach_defaults( GTK_TABLE(table), hscale, 2, 8, 0, 1);
   gtk_widget_show(hscale);
   W.image_zoom = adj;
   W.image_zoom_hid = hid;

   /*------------------------------------
   ** Full Image
   */
   button = gtk_button_new_with_label( "Full Image" );
   g_signal_connect( G_OBJECT(button), "clicked", G_CALLBACK(button2_cb), (gpointer)"FullImage" );
   gtk_table_attach_defaults( GTK_TABLE(table), button, 2, 8, 1, 2  );
   gtk_widget_show( button );

   /*------------------------------------
   ** box zoom
   */
   label = gtk_label_new( "Box: " );
   gtk_misc_set_alignment( GTK_MISC(label), 1, 0.5 );
   gtk_table_attach_defaults( GTK_TABLE(table), label, 0, 2, 2, 3 );
   gtk_widget_show( label );

   button = gtk_button_new_with_label( "Zoom" );
   g_signal_connect( G_OBJECT(button), "clicked", G_CALLBACK(button2_cb), (gpointer)"BoxZoom" );
   gtk_table_attach_defaults( GTK_TABLE(table), button, 2, 5, 2, 3  );
   gtk_widget_show( button );

   /*------------------------------------
   ** BoxScale
   */
   button = gtk_button_new_with_label( "Scale" );
   gtk_table_attach_defaults( GTK_TABLE(table), button, 5, 8, 2, 3  );
   g_signal_connect( G_OBJECT(button), "clicked", G_CALLBACK(button2_cb), (gpointer)"BoxScale" );
   gtk_widget_show( button );

   /*------------------------------------
   ** ObjBoxPeak
   */
   button = gtk_button_new_with_label( "Peak" );
   g_signal_connect( G_OBJECT(button), "clicked", G_CALLBACK(button2_cb), (gpointer)"BoxPeak" );
   gtk_table_attach_defaults( GTK_TABLE(table), button, 2, 5, 3, 4  );
   gtk_widget_show( button );

   /*------------------------------------
   ** ObjBoxCentroid
   */
   button = gtk_button_new_with_label( "Centroid" );
   g_signal_connect( G_OBJECT(button), "clicked", G_CALLBACK(button2_cb), (gpointer)"BoxCentroid" );
   gtk_table_attach_defaults( GTK_TABLE(table), button, 5, 8, 3, 4  );
   gtk_widget_show( button );

   /*------------------------------
   ** Set SubArray from Box buttons:
   */
   label = gtk_label_new( "Set SubArray from Box:" );
   gtk_misc_set_alignment( GTK_MISC(label), 1, 0.5 );
   gtk_table_attach_defaults( GTK_TABLE(table), label, 0, 6, 4, 5 );
   gtk_widget_show( label );

   x = 6;
   for( i=0; i < 5; i++ )
   {
      button = gtk_button_new_with_label( Button_names[i] );
      g_signal_connect( G_OBJECT(button), "released",
         G_CALLBACK(inst_subarray_cb), (gpointer) (intptr_t)i );

      gtk_table_attach_defaults( GTK_TABLE(table), button, x, x+1, 4, 5);
      gtk_widget_show( button );
      x += 1;
   }

   /*------------------------------
   ** AutoScale Label & radio buttons
   */
   label = gtk_label_new( "AutoScale: " );
   gtk_misc_set_alignment( GTK_MISC(label), 1, 0.5 );
   gtk_table_attach_defaults( GTK_TABLE(table), label, 9, 12, 0, 1 );
   gtk_widget_show( label );

   hbox = MyCreateRadioButtons( imageautoscale_selection, W.image_autoscale, 
          TRUE, TRUE, G_CALLBACK(image_autoscale_cb) );
   gtk_table_attach_defaults( GTK_TABLE(table), hbox, 12, 18, 0, 1 );
   gtk_widget_show( hbox );

   /*------------------------------
   ** ImageScaleRange
   */
   label = gtk_label_new( "Range: " );
   gtk_misc_set_alignment( GTK_MISC(label), 1, 0.5 );
   gtk_table_attach_defaults( GTK_TABLE(table), label, 10, 12, 1, 2 );
   gtk_widget_show( label );

   entry = gtk_entry_new( );
   g_signal_connect( G_OBJECT(entry), "activate", G_CALLBACK(image_range_cb), (gpointer)NULL);
   gtk_table_attach_defaults( GTK_TABLE(table), entry, 12, 15, 1, 2 );
   gtk_widget_show(entry);
   W.image_range_min = entry;

   label = gtk_label_new( "to" );
   gtk_table_attach_defaults( GTK_TABLE(table), label, 15, 16, 1, 2 );
   gtk_widget_show( label );

   entry = gtk_entry_new( );
   g_signal_connect( G_OBJECT(entry), "activate", G_CALLBACK(image_range_cb), (gpointer)NULL);
	g_signal_connect( G_OBJECT (entry), "focus-out-event", 
							  G_CALLBACK (image_range_fo_cb), (gpointer) NULL );
   gtk_table_attach_defaults( GTK_TABLE(table), entry, 16, 19, 1, 2 );
   gtk_widget_show(entry);
   W.image_range_max = entry;

   /*------------------------------------
   ** 1 to 99% scale
   */
   button = gtk_button_new_with_label( "1-99% Fixed Scale" );
   g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(button2_cb), (gpointer)"ImageScale1P" );
   gtk_table_attach_defaults( GTK_TABLE(table), button, 12, 19, 2, 3 );
   gtk_widget_show( button );

   /*------------------------------
   ** print button
   */
   button = gtk_button_new_with_label( "Print" );
   g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(button_cb), (gpointer)"Print" );
   gtk_table_attach_defaults( GTK_TABLE(table), button, 15, 19, 4, 5 );
   gtk_widget_show( button );

   /* return the table reference to the user */
   *c = table;
}

/*-----------------------------------------------------------------------
**  create_display_header_widgets() - create the header parameters widgets.
**-----------------------------------------------------------------------
*/
void create_display_header_widgets( GtkWidget ** c )
{
   GtkWidget * frame;

   frame = gtk_frame_new( NULL );

   /* return the button reference to the user */
   *c = frame;
}

/*-----------------------------------------------------------------------
**  create_display_histogram_widgets() - create the histogram parameters widgets.
**-----------------------------------------------------------------------
*/
void create_display_histogram_widgets( GtkWidget ** c )
{
   GtkWidget * table;
   GtkWidget * label;
   GtkWidget * entry;
   GtkWidget * hbox;
   GtkWidget * button;
   GtkWidget * checkbutton;

   table = gtk_table_new( 5, 20, TRUE );
   gtk_widget_set_size_request( GTK_WIDGET(table), 475, 125 );

   /*------------------------------
   ** Histogram Range
   */
   label = gtk_label_new( "Range: " );
   gtk_misc_set_alignment( GTK_MISC(label), 1, 0.5 );
   gtk_table_attach_defaults( GTK_TABLE(table), label, 1, 6, 1, 2 );
   gtk_widget_show( label );

   entry = gtk_entry_new( );
   g_signal_connect( G_OBJECT(entry), "activate", G_CALLBACK(hist_range_cb), (gpointer)NULL );
   gtk_table_attach_defaults( GTK_TABLE(table), entry, 6, 10, 1, 2 );
   gtk_widget_show(entry);
   W.hist_range_min = entry;

   label = gtk_label_new( "to" );
   gtk_table_attach_defaults( GTK_TABLE(table), label, 10, 12, 1, 2 );
   gtk_widget_show( label );

   entry = gtk_entry_new( );
   g_signal_connect( G_OBJECT(entry), "activate", 
							  G_CALLBACK(hist_range_cb), (gpointer)NULL );
	g_signal_connect( G_OBJECT (entry), "focus-out-event", 
							  G_CALLBACK (hist_range_fo_cb), (gpointer) NULL );
   gtk_table_attach_defaults( GTK_TABLE(table), entry, 12, 16, 1, 2 );
   gtk_widget_show(entry);
   W.hist_range_max = entry;

   /*------------------------------
   ** Histogram Bin Number
   */
   label = gtk_label_new( "Num of Bins (1-100): " );
   gtk_misc_set_alignment( GTK_MISC(label), 1, 0.5 );
   gtk_table_attach_defaults( GTK_TABLE(table), label, 0, 6, 2, 3 );
   gtk_widget_show( label );

   entry = gtk_entry_new( );
   g_signal_connect( G_OBJECT(entry), "activate", 
							  G_CALLBACK(entry_cb), (gpointer) "HistBin" );
	g_signal_connect( G_OBJECT (entry), "focus-out-event", 
							  G_CALLBACK (entry_fo_cb), (gpointer) "HistBin" );
   gtk_table_attach_defaults( GTK_TABLE(table), entry, 6, 8, 2, 3 );
   gtk_widget_show(entry);
   W.hist_bins = entry;

   /*------------------------------
   ** Histogram Area Selection
   */
   label = gtk_label_new( "Area: " );
   gtk_misc_set_alignment( GTK_MISC(label), 1, 0.5 );
   gtk_table_attach_defaults( GTK_TABLE(table), label, 1, 6, 3, 4 );
   gtk_widget_show( label );

   hbox = MyCreateRadioButtons( allbox_selection, W.hist_area, TRUE, TRUE, G_CALLBACK(hist_area_cb));
   gtk_table_attach_defaults( GTK_TABLE(table), hbox, 6, 10, 3, 4 );
   gtk_widget_show( hbox );

   /*------------------------------
   ** Hist AutoRange 
   */
	checkbutton = gtk_check_button_new_with_label("AutoRange");
	g_signal_connect( G_OBJECT(checkbutton), "toggled",
		G_CALLBACK (checkbutton_offon_cb), (gpointer) "HistAutoRange" );
	gtk_table_attach_defaults (GTK_TABLE(table), checkbutton, 6, 12, 4, 5 );
	gtk_widget_show(checkbutton);
	W.hist_auto_range = checkbutton;

   /*------------------------------
   ** print button
   */
   button = gtk_button_new_with_label( "Print" );
   g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(button_cb), (gpointer)"Print" );
   gtk_table_attach_defaults( GTK_TABLE(table), button, 17, 19, 4, 5 );
   gtk_widget_show( button );

   /* return the table reference to the user */
   *c = table;
}

/*-----------------------------------------------------------------------
**  create_display_linecut_widgets() - create the linecut parameters widgets.
**-----------------------------------------------------------------------
*/
void create_display_linecut_widgets( GtkWidget ** c )
{
   GtkWidget * table;
   GtkWidget * label;
   GtkWidget * entry;
   GtkWidget * hbox;
   GtkWidget * button;

   table = gtk_table_new( 5, 20, TRUE );
   gtk_widget_set_size_request( GTK_WIDGET(table), 475, 125 );

   /*------------------------------
   ** Linecut X Axis
   */
   label = gtk_label_new( "X Axis: " );
   gtk_misc_set_alignment( GTK_MISC(label), 1, 0.5 );
   gtk_table_attach_defaults( GTK_TABLE(table), label, 1, 4, 1, 2 );
   gtk_widget_show( label );

   entry = gtk_entry_new( );
   g_signal_connect( G_OBJECT(entry), "activate", 
							  G_CALLBACK(lcut_xy_cb), (gpointer)NULL );
   g_signal_connect( G_OBJECT(entry), "focus-out-event", 
							  G_CALLBACK(lcut_xy_fo_cb), (gpointer)NULL );
   gtk_table_attach_defaults( GTK_TABLE(table), entry, 4, 7, 1, 2 );
   gtk_widget_show(entry);
   W.lcut_x = entry;

   button = gtk_button_new_with_label( "UP" );
   g_signal_connect( G_OBJECT(button), "clicked", G_CALLBACK(lcut_xarrow), (gpointer) (+1) );
   gtk_table_attach_defaults( GTK_TABLE(table), button, 7, 8, 1, 2 );
   gtk_widget_show( button );

   button = gtk_button_new_with_label( "DN" );
   g_signal_connect( G_OBJECT(button), "clicked", G_CALLBACK(lcut_xarrow), (gpointer) (-1) );
   gtk_table_attach_defaults( GTK_TABLE(table), button, 8, 9, 1, 2 );
   gtk_widget_show( button );

   /*------------------------------
   ** Linecut Area Selection
   */
   label = gtk_label_new( "Area: " );
   gtk_misc_set_alignment( GTK_MISC(label), 1, 0.5 );
   gtk_table_attach_defaults( GTK_TABLE(table), label, 10, 12, 1, 2 );
   gtk_widget_show( label );

   hbox = MyCreateRadioButtons( allbox_selection, W.lcut_area, TRUE, TRUE, G_CALLBACK(lcut_area_cb) );
   gtk_table_attach_defaults( GTK_TABLE(table), hbox, 12, 15, 1, 2 );
   gtk_widget_show( hbox );

   /*------------------------------
   ** Linecut Y Axis
   */
   label = gtk_label_new( "Y Axis: " );
   gtk_misc_set_alignment( GTK_MISC(label), 1, 0.5 );
   gtk_table_attach_defaults( GTK_TABLE(table), label, 1, 4, 2, 3 );
   gtk_widget_show( label );

   entry = gtk_entry_new( );
   g_signal_connect( G_OBJECT(entry), "activate", 
							  G_CALLBACK(lcut_xy_cb), (gpointer)NULL );
   g_signal_connect( G_OBJECT(entry), "focus-out-event", 
							  G_CALLBACK(lcut_xy_fo_cb), (gpointer)NULL );
   gtk_table_attach_defaults( GTK_TABLE(table), entry, 4, 7, 2, 3 );
   gtk_widget_show(entry);
   W.lcut_y = entry;

   button = gtk_button_new_with_label( "UP" );
   g_signal_connect( G_OBJECT(button), "clicked", G_CALLBACK(lcut_yarrow), (gpointer) (+1) );
   gtk_table_attach_defaults( GTK_TABLE(table), button, 7, 8, 2, 3 );
   gtk_widget_show( button );

   button = gtk_button_new_with_label( "DN" );
   g_signal_connect( G_OBJECT(button), "clicked", G_CALLBACK(lcut_yarrow), (gpointer) (-1) );
   gtk_table_attach_defaults( GTK_TABLE(table), button, 8, 9, 2, 3 );
   gtk_widget_show( button );

   /*------------------------------
   ** Linecut Range
   */
   label = gtk_label_new( "Range: " );
   gtk_misc_set_alignment( GTK_MISC(label), 1, 0.5 );
   gtk_table_attach_defaults( GTK_TABLE(table), label, 10, 12, 2, 3 );
   gtk_widget_show( label );

   entry = gtk_entry_new( );
   g_signal_connect( G_OBJECT(entry), "activate", G_CALLBACK(lcut_range_cb), (gpointer)NULL );
   gtk_table_attach_defaults( GTK_TABLE(table), entry, 12, 15, 2, 3 );
   gtk_widget_show(entry);
   W.lcut_range_min = entry;

   label = gtk_label_new( "to" );
   gtk_table_attach_defaults( GTK_TABLE(table), label, 15, 16, 2, 3 );
   gtk_widget_show( label );

   entry = gtk_entry_new( );
   g_signal_connect( G_OBJECT(entry), "activate", 
							  G_CALLBACK(lcut_range_cb), (gpointer)NULL );
   g_signal_connect( G_OBJECT(entry), "focus-out-event", 
							  G_CALLBACK(lcut_range_fo_cb), (gpointer)NULL );
   gtk_table_attach_defaults( GTK_TABLE(table), entry, 16, 19, 2, 3 );
   gtk_widget_show(entry);
   W.lcut_range_max = entry;

   /*------------------------------
   ** AutoScale Label & radio buttons
   */
   label = gtk_label_new( "Auto Scale: " );
   gtk_misc_set_alignment( GTK_MISC(label), 1, 0.5 );
   gtk_table_attach_defaults( GTK_TABLE(table), label, 0, 4, 3, 4  );
   gtk_widget_show( label );

   hbox = MyCreateRadioButtons( imageautoscale_selection, W.lcut_autoscale, 
          TRUE, TRUE, G_CALLBACK(lcut_autoscale_cb) );
   gtk_table_attach_defaults( GTK_TABLE(table), hbox, 4, 8, 3, 4  );
   gtk_widget_show( hbox );

   /*------------------------------
   ** print button
   */
   button = gtk_button_new_with_label( "Print" );
   g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(button_cb), (gpointer)"Print" );
   gtk_table_attach_defaults( GTK_TABLE(table), button, 17, 19, 4, 5 );
   gtk_widget_show( button );

   /* return the table reference to the user */
   *c = table;
}

/*-----------------------------------------------------------------------
**  create_display_xlinecut_widgets() - create the xlinecut parameters widgets.
**-----------------------------------------------------------------------
*/
void create_display_xlinecut_widgets( GtkWidget ** c )
{
   GtkWidget * table;
   GtkWidget * label;
   GtkWidget * entry;
   GtkWidget * hbox;
   GtkWidget * button;

   table = gtk_table_new( 5, 20, TRUE );
   gtk_widget_set_size_request( GTK_WIDGET(table), 475, 125 );

   /*------------------------------
   ** AutoScale Label & radio buttons
   */
   label = gtk_label_new( "Auto Scale: " );
   gtk_misc_set_alignment( GTK_MISC(label), 1, 0.5 );
   gtk_table_attach_defaults( GTK_TABLE(table), label, 0, 4, 1, 2 );
   gtk_widget_show( label );

   hbox = MyCreateRadioButtons( imageautoscale_selection, W.xcut_autoscale, 
          TRUE, TRUE, G_CALLBACK(xcut_autoscale_cb) );
   gtk_table_attach_defaults( GTK_TABLE(table), hbox, 4, 8, 1, 2 );
   gtk_widget_show( hbox );

   /*------------------------------
   ** X Linecut set endpoints
   */
   button = gtk_button_new_with_label( "Set Endpoint from Line" );
   g_signal_connect( G_OBJECT(button), "clicked", G_CALLBACK(xcut_setline_cb), (gpointer)NULL );
   gtk_table_attach_defaults( GTK_TABLE(table), button, 1, 9, 2, 3 );
   gtk_widget_show( button );

   /*------------------------------
   ** X Linecut Beg to End of line
   */
   label = gtk_label_new( "Beg " );
   gtk_misc_set_alignment( GTK_MISC(label), 1, 0.5 );
   gtk_table_attach_defaults( GTK_TABLE(table), label, 11, 12, 2, 3 );
   gtk_widget_show( label );

   entry = gtk_entry_new( );
   g_signal_connect( G_OBJECT(entry), "activate", 
							  G_CALLBACK(xcut_set_cb), (gpointer)NULL );
   g_signal_connect( G_OBJECT(entry), "focus-out-event", 
							  G_CALLBACK(xcut_set_fo_cb), (gpointer)NULL );
   gtk_table_attach_defaults( GTK_TABLE(table), entry, 12, 15, 2, 3 );
   gtk_widget_show(entry);
   W.xcut_beg = entry;

   label = gtk_label_new( "End " );
   gtk_table_attach_defaults( GTK_TABLE(table), label, 15, 16, 2, 3 );
   gtk_widget_show( label );

   entry = gtk_entry_new( );
   g_signal_connect( G_OBJECT(entry), "activate", 
							  G_CALLBACK(xcut_set_cb), (gpointer)NULL );
   g_signal_connect( G_OBJECT(entry), "focus-out-event", 
							  G_CALLBACK(xcut_set_fo_cb), (gpointer)NULL );
   gtk_table_attach_defaults( GTK_TABLE(table), entry, 16, 19, 2, 3 );
   gtk_widget_show(entry);
   W.xcut_end = entry;

   /*------------------------------
   ** Output Data for XGFIT
   */
   button = gtk_button_new_with_label( "Output Data" );
   g_signal_connect( G_OBJECT(button), "clicked",
		G_CALLBACK(xcut_output_data_cb), (gpointer)NULL );
   gtk_table_attach_defaults( GTK_TABLE(table), button, 1, 5, 3, 4  );
   gtk_widget_show( button );

#if USE_GSLFIT
   /*------------------------------
   ** Fit Guassian to Data 
   */
   button = gtk_button_new_with_label( "Gaussian Fit" );
   g_signal_connect( G_OBJECT(button), "clicked", 
		G_CALLBACK(xcut_fit_data_cb), (gpointer)NULL );
   gtk_table_attach_defaults( GTK_TABLE(table), button, 5, 9, 3, 4  );
   gtk_widget_show( button );
#endif // USE_GSLFIT

   /*------------------------------
   ** X Linecut Range
   */
   label = gtk_label_new( "Range: " );
   gtk_misc_set_alignment( GTK_MISC(label), 1, 0.5 );
   gtk_table_attach_defaults( GTK_TABLE(table), label, 10, 12, 3, 4  );
   gtk_widget_show( label );

   entry = gtk_entry_new( );
   g_signal_connect( G_OBJECT(entry), "activate", 
							  G_CALLBACK(xcut_range_cb), (gpointer)NULL );
   gtk_table_attach_defaults( GTK_TABLE(table), entry, 12, 15, 3, 4  );
   gtk_widget_show(entry);
   W.xcut_range_min = entry;

   label = gtk_label_new( "to" );
   gtk_table_attach_defaults( GTK_TABLE(table), label, 15, 16, 3, 4  );
   gtk_widget_show( label );

   entry = gtk_entry_new( );
   g_signal_connect( G_OBJECT(entry), "activate", 
							  G_CALLBACK(xcut_range_cb), (gpointer)NULL );
   g_signal_connect( G_OBJECT(entry), "focus-out-event", 
							  G_CALLBACK(lcut_range_fo_cb), (gpointer)NULL );
   gtk_table_attach_defaults( GTK_TABLE(table), entry, 16, 19, 3, 4  );
   gtk_widget_show(entry);
   W.xcut_range_max = entry;

   /*------------------------------
   ** print button
   */
   button = gtk_button_new_with_label( "Print" );
   g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(button_cb), (gpointer)"Print" );
   gtk_table_attach_defaults( GTK_TABLE(table), button, 17, 19, 4, 5 );
   gtk_widget_show( button );

   /* return the table reference to the user */
   *c = table;
}

/*-----------------------------------------------------------------------
**  create_display_noise_widgets() - create the noise parameters widgets.
**-----------------------------------------------------------------------
*/
void create_display_noise_widgets( GtkWidget ** c )
{
   GtkWidget * table;
   GtkWidget * label;
   GtkWidget * entry;
   GtkWidget * hbox;
   GtkWidget * button;

   table = gtk_table_new( 5, 20, TRUE );
   gtk_widget_set_size_request( GTK_WIDGET(table), 475, 125 );

   /*------------------------------
   ** Noise Mod (1-256)
   */
   label = gtk_label_new( "Mod/Ch (1-256): " );
   gtk_misc_set_alignment( GTK_MISC(label), 1, 0.5 );
   gtk_table_attach_defaults( GTK_TABLE(table), label, 0, 4, 0, 1 );
   gtk_widget_show( label );

   entry = gtk_entry_new( );
   g_signal_connect( G_OBJECT(entry), "activate",
		G_CALLBACK(entry_cb), (gpointer) "NoiseMod" );
	g_signal_connect (G_OBJECT (entry), "focus-out-event",
		G_CALLBACK (entry_fo_cb), (gpointer) "NoiseMod" );
   gtk_table_attach_defaults( GTK_TABLE(table), entry, 4, 6, 0, 1 );
   gtk_widget_show(entry);
   W.noise_mod = entry;

   /*------------------------------
   ** Ch / Size  
   */
   label = gtk_label_new( "Size: " );
   gtk_misc_set_alignment( GTK_MISC(label), 1, 0.5 );
   gtk_table_attach_defaults( GTK_TABLE(table), label, 0, 4, 1, 2 );
   gtk_widget_show( label );

   entry = gtk_entry_new( );
   g_signal_connect( G_OBJECT(entry), "activate",
		G_CALLBACK(entry_cb), (gpointer) "NoiseSize" );
	g_signal_connect (G_OBJECT (entry), "focus-out-event",
		G_CALLBACK (entry_fo_cb), (gpointer) "NoiseSize" );
   gtk_table_attach_defaults( GTK_TABLE(table), entry, 4, 6, 1, 2 );
   gtk_widget_show(entry);
   W.noise_size = entry;

   /*------------------------------
   ** mode
   */
   label = gtk_label_new( "Mode: " );
   gtk_misc_set_alignment( GTK_MISC(label), 1, 0.5 );
   gtk_table_attach_defaults( GTK_TABLE(table), label, 10, 12, 0, 1 );
   gtk_widget_show( label );

   hbox = MyCreateRadioButtons( noise_mode_selection, W.noise_mode, TRUE, TRUE, 
	                             G_CALLBACK(noise_mode_cb));
   gtk_table_attach_defaults( GTK_TABLE(table), hbox, 12, 16, 0, 1 );
   gtk_widget_show( hbox );

   /*------------------------------
   ** Area Label & radio buttons
   */
   label = gtk_label_new( "Area: " );
   gtk_misc_set_alignment( GTK_MISC(label), 1, 0.5 );
   gtk_table_attach_defaults( GTK_TABLE(table), label, 10, 12, 1, 2 );
   gtk_widget_show( label );

   hbox = MyCreateRadioButtons( allbox_selection, W.noise_area, TRUE, TRUE, G_CALLBACK(noise_area_cb));
   gtk_table_attach_defaults( GTK_TABLE(table), hbox, 12, 16, 1, 2 );
   gtk_widget_show( hbox );


   /*------------------------------
   ** Graph Type Label & radio buttons
   */
   label = gtk_label_new( "Graph Type: " );
   gtk_misc_set_alignment( GTK_MISC(label), 1, 0.5 );
   gtk_table_attach_defaults( GTK_TABLE(table), label, 0, 4, 2, 3 );
   gtk_widget_show( label );

   hbox = MyCreateRadioButtons( noisegraphtype_selection, 
          W.noise_graph_type, TRUE, TRUE, G_CALLBACK(noise_graphtype_cb) );
   gtk_table_attach_defaults( GTK_TABLE(table), hbox, 4, 8, 2, 3 );
   gtk_widget_show( hbox );

   /*------------------------------
   ** Noise Graph 1 Range
   */
   label = gtk_label_new( "Graph1 Rng: " );
   gtk_misc_set_alignment( GTK_MISC(label), 1, 0.5 );
   gtk_table_attach_defaults( GTK_TABLE(table), label, 8, 12, 2, 3 );
   gtk_widget_show( label );

   entry = gtk_entry_new( );
   g_signal_connect( G_OBJECT(entry), "activate", 
							  G_CALLBACK(noise_g1range_cb), (gpointer)NULL );
   gtk_table_attach_defaults( GTK_TABLE(table), entry, 12, 15, 2, 3 );
   gtk_widget_show(entry);
   W.noise_g1_range_min = entry;

   label = gtk_label_new( "to" );
   gtk_table_attach_defaults( GTK_TABLE(table), label, 15, 16, 2, 3 );
   gtk_widget_show( label );

   entry = gtk_entry_new( );
   g_signal_connect( G_OBJECT(entry), "activate", 
		G_CALLBACK(noise_g1range_cb), (gpointer)NULL );
	g_signal_connect (G_OBJECT (entry), "focus-out-event",
		G_CALLBACK(noise_g1range_fo_cb), (gpointer)NULL );
   gtk_table_attach_defaults( GTK_TABLE(table), entry, 16, 19, 2, 3 );
   gtk_widget_show(entry);
   W.noise_g1_range_max = entry;

   /*------------------------------
   ** AutoScale Label & radio buttons
   */
   label = gtk_label_new( "Auto Scale: " );
   gtk_misc_set_alignment( GTK_MISC(label), 1, 0.5 );
   gtk_table_attach_defaults( GTK_TABLE(table), label, 1, 4, 3, 4  );
   gtk_widget_show( label );

   hbox = MyCreateRadioButtons( imageautoscale_selection, W.noise_autoscale, 
          TRUE, TRUE, G_CALLBACK(noise_autoscale_cb) );
   gtk_table_attach_defaults( GTK_TABLE(table), hbox, 4, 8, 3, 4  );
   gtk_widget_show( hbox );

   /*------------------------------
   ** Noise Graph 2 Range
   */
   label = gtk_label_new( "Graph2 Rng: " );
   gtk_misc_set_alignment( GTK_MISC(label), 1, 0.5 );
   gtk_table_attach_defaults( GTK_TABLE(table), label, 8, 12, 3, 4  );
   gtk_widget_show( label );

   entry = gtk_entry_new( );
   g_signal_connect( G_OBJECT(entry), "activate", 
		G_CALLBACK(noise_g2range_cb), (gpointer)NULL );
   gtk_table_attach_defaults( GTK_TABLE(table), entry, 12, 15, 3, 4  );
   gtk_widget_show(entry);
   W.noise_g2_range_min = entry;

   label = gtk_label_new( "to" );
   gtk_table_attach_defaults( GTK_TABLE(table), label, 15, 16, 3, 4  );
   gtk_widget_show( label );

   entry = gtk_entry_new( );
   g_signal_connect( G_OBJECT(entry), "activate", 
		G_CALLBACK(noise_g2range_cb), (gpointer)NULL );
	g_signal_connect (G_OBJECT (entry), "focus-out-event",
		G_CALLBACK(noise_g1range_fo_cb), (gpointer)NULL );
   gtk_table_attach_defaults( GTK_TABLE(table), entry, 16, 19, 3, 4  );
   gtk_widget_show(entry);
   W.noise_g2_range_max = entry;

   /*------------------------------
   ** print button
   */
   button = gtk_button_new_with_label( "Print" );
   g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(button_cb), (gpointer)"Print" );
   gtk_table_attach_defaults( GTK_TABLE(table), button, 17, 19, 4, 5 );
   gtk_widget_show( button );

   /* return the table reference to the user */
   *c = table;
}

/*-----------------------------------------------------------------------
**  create_display_pointer_widgets() - create the pointer parameters widgets.
**-----------------------------------------------------------------------
*/
void create_display_pointer_widgets( GtkWidget ** c )
{
   GtkWidget * table;
   GtkWidget * label;
   GtkWidget * entry;
   GtkWidget * checkbutton;

   table = gtk_table_new( 5, 20, TRUE );
   gtk_widget_set_size_request( GTK_WIDGET(table), 475, 125 );

   /*------------------------------
   ** Pointer Box Size in Pixels
   */
   label = gtk_label_new( "Image Size (11-111): " );
   gtk_misc_set_alignment( GTK_MISC(label), 1, 0.5 );
   gtk_table_attach_defaults( GTK_TABLE(table), label, 0, 5, 1, 2 );
   gtk_widget_show( label );

   entry = gtk_entry_new( );
   g_signal_connect( G_OBJECT(entry), "activate", 
		G_CALLBACK(entry_cb), (gpointer)"PtImageSize" );
   g_signal_connect( G_OBJECT(entry), "focus-out-event",
		G_CALLBACK(entry_fo_cb), (gpointer)"PtImageSize" );
   gtk_table_attach_defaults( GTK_TABLE(table), entry, 5, 9, 1, 2 );
   gtk_widget_show(entry);
   W.pt_image_size = entry;

   /*------------------------------
   ** Pointer Show Stats 
	*/
	checkbutton = gtk_check_button_new_with_label("Show Stats");
	g_signal_connect( G_OBJECT(checkbutton), "toggled",
		G_CALLBACK (checkbutton_offon_cb), (gpointer) "PtShowStats" );
	gtk_table_attach_defaults (GTK_TABLE(table), checkbutton, 10, 16, 1, 2 );
	gtk_widget_show(checkbutton);
	W.pt_show_stats = checkbutton;

   /*------------------------------
   ** Pointer Spex S/N Est 
   */
	checkbutton = gtk_check_button_new_with_label("Show Spex S/N Est");
	g_signal_connect( G_OBJECT(checkbutton), "toggled",
		G_CALLBACK (checkbutton_offon_cb), (gpointer) "PtShowSpexSN" );
	gtk_table_attach_defaults (GTK_TABLE(table), checkbutton, 10, 16, 2, 3 );
	gtk_widget_show(checkbutton);
	W.pt_show_spex_sn = checkbutton;

   /*------------------------------
   ** Pointer Show linecut 
   */
	checkbutton = gtk_check_button_new_with_label("Show LineCut");
	g_signal_connect( G_OBJECT(checkbutton), "toggled",
		G_CALLBACK (checkbutton_offon_cb), (gpointer) "PtShowLinecut" );
	gtk_table_attach_defaults (GTK_TABLE(table), checkbutton, 10, 16 , 3, 4 );
	gtk_widget_show(checkbutton);
	W.pt_show_linecut = checkbutton;

   /* return the table reference to the user */
   *c = table;
}

/*-----------------------------------------------------------------------
**  create_display_stats_widgets() - create the stats parameters widgets.
**-----------------------------------------------------------------------
*/
void create_display_stats_widgets( GtkWidget ** c )
{
   GtkWidget * table;
   GtkWidget * button;
   GtkWidget * entry;

   table = gtk_table_new( 5, 20, TRUE );
   gtk_widget_set_size_request( GTK_WIDGET(table), 475, 125 );

   /*---------------------------------------
   ** Set sky button
   */
   button = gtk_button_new_with_label( "Set Sky" );
   gtk_table_attach( GTK_TABLE(table), button, 1, 9, 1, 2, GTK_EXPAND|GTK_FILL, 0, 0, 0);
   g_signal_connect(G_OBJECT(button), "clicked",
      G_CALLBACK(stats_setsky_cb), (gpointer) NULL);
   gtk_widget_show( button );

   /*---------------------------------------
   ** Fix WH check button & entry
   */
   button = gtk_check_button_new_with_label( "Fix Wid & Hgt to" );
   g_signal_connect( G_OBJECT(button), "released",
      G_CALLBACK(stats_fixedwh_cb), (gpointer) NULL );
   gtk_table_attach_defaults( GTK_TABLE(table), button, 1, 6, 2, 3);
   gtk_widget_show( button );
   W.stats_fixedwh = button;

   entry = gtk_entry_new( );
   gtk_table_attach_defaults( GTK_TABLE(table), entry, 6, 9, 2, 3);
   g_signal_connect( G_OBJECT(entry), "activate", 
		G_CALLBACK(stats_wh_entry_cb), (gpointer)NULL);
	g_signal_connect (G_OBJECT (entry), "focus-out-event",
		G_CALLBACK (stats_wh_entry_fo_cb), (gpointer)NULL );

   gtk_widget_show(entry);
   W.stats_wh_entry = entry;

   /* return the container reference to the user */
   *c = table;
}

/*-----------------------------------------------------------------------
**  create_display_aofig_widgets() - create the ao figure parameters widgets.
**-----------------------------------------------------------------------
*/
void create_display_aofig_widgets( GtkWidget ** c )
{
   GtkWidget * table,
             * label,
             * cbox,
             * spinner,
             * button;
   GtkAdjustment * x_adj,
                 * y_adj;

   table = gtk_table_new( 5, 20, TRUE );
   gtk_widget_set_size_request( GTK_WIDGET(table), 475, 125 );

   /*-------------------------------
   **  format
   */
   label = gtk_label_new( "Format: " );
   gtk_misc_set_alignment( GTK_MISC(label), 1, 0.5 );
   gtk_table_attach_defaults( GTK_TABLE(table), label, 0, 4, 1, 2 );
   gtk_widget_show( label );

   cbox = MyComboBoxCreate( aofig_format_selection );
   gtk_table_attach_defaults( GTK_TABLE(table), cbox, 4, 9, 1, 2 );
   W.aofig_format_hid = g_signal_connect( G_OBJECT(cbox), "changed", G_CALLBACK(aofig_format_cb), NULL);
   gtk_widget_show(cbox);
   W.aofig_format = cbox;

   /*-------------------------------
   **  Data (organization)
   */
   label = gtk_label_new( "Data: " );
   gtk_misc_set_alignment( GTK_MISC(label), 1, 0.5 );
   gtk_table_attach_defaults( GTK_TABLE(table), label, 0, 4, 2, 3 );
   gtk_widget_show( label );

   cbox = MyComboBoxCreate( xy_selection );
   gtk_table_attach_defaults( GTK_TABLE(table), cbox, 4, 9, 2, 3 );
   W.aofig_data_hid = g_signal_connect( G_OBJECT(cbox), "changed", G_CALLBACK(aofig_data_cb), NULL);
   gtk_widget_show(cbox);
   W.aofig_data = cbox;

   /*-------------------------------
   **  aofig_x, aofig_y
   */
   label = gtk_label_new( "XY:" );
   gtk_misc_set_alignment( GTK_MISC(label), 1, 1 );
   gtk_table_attach_defaults( GTK_TABLE(table), label, 0, 4, 3, 4 );
   gtk_widget_show( label );

   /* spinner for X */
   x_adj = (GtkAdjustment*) gtk_adjustment_new (0.0, 0, AOFIG_NUM_ELEMENTS-1, 1, 1, 1);

   spinner = gtk_spin_button_new( x_adj, 1, 0);
   gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
   gtk_table_attach_defaults( GTK_TABLE(table), spinner, 5, 7, 3, 4 );
   gtk_widget_show( spinner );
   W.aofig_x = spinner;

   /* spinner for Y */
   y_adj = (GtkAdjustment*) gtk_adjustment_new (0.0, 0, AOFIG_NUM_ELEMENTS-1, 1, 1, 1);

   spinner = gtk_spin_button_new( y_adj, 1, 0);
   gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), TRUE);
   gtk_table_attach_defaults( GTK_TABLE(table), spinner, 7, 9, 3, 4 );
   gtk_widget_show( spinner );
   W.aofig_y = spinner;

   /* connect X & Y adjustment to the same callback */
   g_signal_connect (G_OBJECT(x_adj), "value_changed", G_CALLBACK(aofig_XY_cb), NULL);
   g_signal_connect (G_OBJECT(y_adj), "value_changed", G_CALLBACK(aofig_XY_cb), NULL);

   /*------------------------------
   ** print button
   */
   button = gtk_button_new_with_label( "Print" );
   g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(button_cb), (gpointer)"Print" );
   gtk_table_attach_defaults( GTK_TABLE(table), button, 15, 19, 4, 5 );
   gtk_widget_show( button );

   /* return the container reference to the user */
   *c = table;
}

/*-----------------------------------------------------------------------
**  create_display_spectra_a_widgets() - create the spectra A parameters widgets.
**-----------------------------------------------------------------------
*/
void create_display_spectra_a_widgets( GtkWidget ** c )
{
   GtkWidget * table,
	          * entry,
	          * label,
	          * checkbutton,
	          * cbox,
             * button;

   table = gtk_table_new( 5, 20, TRUE );
   gtk_widget_set_size_request( GTK_WIDGET(table), 475, 125 );

   // objbin       
   button = gtk_button_new_with_label( "Set ObjBin" );
   g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(sa_setobjbin_cb), (gpointer)NULL );
   gtk_table_attach_defaults( GTK_TABLE(table), button, 0, 3, 0, 1 );
   gtk_widget_show( button );

   label = gtk_label_new( "()" );
   gtk_misc_set_alignment( GTK_MISC(label), 0.5, 0.5 );
   gtk_table_attach_defaults( GTK_TABLE(table), label, 3, 5, 0, 1 );
   gtk_widget_show( label );
	W.sa_objbin_label = label;

   entry = gtk_entry_new( );
   gtk_table_attach_defaults( GTK_TABLE(table), entry, 5, 9, 0, 1);
   g_signal_connect( G_OBJECT(entry), "activate", 
		G_CALLBACK(entry_cb), (gpointer)"SAObjBin ");
	g_signal_connect (G_OBJECT (entry), "focus-out-event",
		G_CALLBACK (entry_fo_cb), (gpointer)"SASkyBin " );
   gtk_widget_show( entry );
	W.sa_objbin = entry;

   // skybin       
   button = gtk_button_new_with_label( "Set SkyBin" );
   g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(sa_setskybin_cb), (gpointer)NULL );
   gtk_table_attach_defaults( GTK_TABLE(table), button, 0, 3, 1, 2 );
   gtk_widget_show( button );

   label = gtk_label_new( "()" );
   gtk_misc_set_alignment( GTK_MISC(label), 0.5, 0.5 );
   gtk_table_attach_defaults( GTK_TABLE(table), label, 3, 5, 1, 2 );
   gtk_widget_show( label );
	W.sa_skybin_label = label;

   entry = gtk_entry_new( );
   gtk_table_attach_defaults( GTK_TABLE(table), entry, 5, 9, 1, 2);
   g_signal_connect( G_OBJECT(entry), "activate", 
		G_CALLBACK(entry_cb), (gpointer)"SASkyBin ");
	g_signal_connect (G_OBJECT (entry), "focus-out-event",
		G_CALLBACK (entry_fo_cb), (gpointer)"SASkyBin " );
   gtk_widget_show( entry );
	W.sa_skybin = entry;

   // saRowsPerBin
   label = gtk_label_new( "RowsPerBin " );
   gtk_misc_set_alignment( GTK_MISC(label), 1, 0.5 );
   gtk_table_attach_defaults( GTK_TABLE(table), label, 1, 5, 2, 3 );
   gtk_widget_show( label );

   entry = gtk_entry_new( );
   gtk_table_attach_defaults( GTK_TABLE(table), entry, 5, 9, 2, 3);
   g_signal_connect( G_OBJECT(entry), "activate", 
		G_CALLBACK(entry_cb), (gpointer)"SARowsPerBin ");
	g_signal_connect (G_OBJECT (entry), "focus-out-event",
		G_CALLBACK (entry_fo_cb), (gpointer)"SARowsPerBin " );
   gtk_widget_show( entry );
	W.sa_rows_per_bin = entry;

   // saShift
   label = gtk_label_new( "Shift " );
   gtk_misc_set_alignment( GTK_MISC(label), 1, 0.5 );
   gtk_table_attach_defaults( GTK_TABLE(table), label, 1, 5, 3, 4 );
   gtk_widget_show( label );

   entry = gtk_entry_new( );
   gtk_table_attach_defaults( GTK_TABLE(table), entry, 5, 9, 3, 4);
   g_signal_connect( G_OBJECT(entry), "activate", 
		G_CALLBACK(entry_cb), (gpointer)"SAShift ");
	g_signal_connect (G_OBJECT (entry), "focus-out-event",
		G_CALLBACK (entry_fo_cb), (gpointer)"SAShift " );
   gtk_widget_show( entry );
	W.sa_shift = entry;

   // Subtract Sky
   checkbutton = gtk_check_button_new_with_label("Subtract Sky");
   g_signal_connect( G_OBJECT(checkbutton), "toggled",
      G_CALLBACK (checkbutton_offon_cb), (gpointer) "SASubtractSky" );
   gtk_table_attach_defaults (GTK_TABLE(table), checkbutton, 5, 9, 4, 5 );
   gtk_widget_show(checkbutton);
   W.sa_subtractsky = checkbutton;

   // Stats 
   checkbutton = gtk_check_button_new_with_label("Stats");
   g_signal_connect( G_OBJECT(checkbutton), "toggled",
      G_CALLBACK (checkbutton_offon_cb), (gpointer) "SAStats" );
   gtk_table_attach_defaults (GTK_TABLE(table), checkbutton, 15, 19, 0, 1 );
   gtk_widget_show(checkbutton);
   W.sa_stats = checkbutton;

   // YAutoscale
   label = gtk_label_new( "YAutoScale " );
   gtk_misc_set_alignment( GTK_MISC(label), 1, 0.5 );
   gtk_table_attach_defaults( GTK_TABLE(table), label, 10, 15, 1, 2 );
   gtk_widget_show( label );

   cbox = MyComboBoxCreate( sa_yautoscale_selection );
   gtk_table_attach_defaults( GTK_TABLE(table), cbox, 15, 19, 1, 2 );
   W.sa_yautoscale_hid = g_signal_connect( G_OBJECT(cbox), "changed", 
                         G_CALLBACK(sa_yautoscale_cb), NULL);
   gtk_widget_show(cbox);
   W.sa_yautoscale = cbox;

   // Y Scale Fixed
   label = gtk_label_new( "Y Scale (Fixed) " );
   gtk_misc_set_alignment( GTK_MISC(label), 1, 0.5 );
   gtk_table_attach_defaults( GTK_TABLE(table), label, 10, 15, 2, 3 );
   gtk_widget_show( label );

   entry = gtk_entry_new( );
   gtk_table_attach_defaults( GTK_TABLE(table), entry, 15, 19, 2, 3);
   g_signal_connect( G_OBJECT(entry), "activate", 
		G_CALLBACK(entry_cb), (gpointer)"SAYScale");
	g_signal_connect (G_OBJECT (entry), "focus-out-event",
		G_CALLBACK (entry_fo_cb), (gpointer)"SAYScale" );
   gtk_widget_show( entry );
	W.sa_yscale = entry;

   // X Scale Fixed
   button = gtk_button_new_with_label( "Set XScale" );
   g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(sa_setxscale_cb), (gpointer)NULL );
   gtk_table_attach_defaults( GTK_TABLE(table), button, 12, 15, 3, 4 );
   gtk_widget_show( button );

   entry = gtk_entry_new( );
   gtk_table_attach_defaults( GTK_TABLE(table), entry, 15, 19, 3, 4);
   g_signal_connect( G_OBJECT(entry), "activate", 
		G_CALLBACK(entry_cb), (gpointer)"SAXScale");
	g_signal_connect (G_OBJECT (entry), "focus-out-event",
		G_CALLBACK (entry_fo_cb), (gpointer)"SAXScale" );
   gtk_widget_show( entry );
	W.sa_xscale = entry;

   /*------------------------------
   ** print button
   */
   button = gtk_button_new_with_label( "Print" );
   g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(button_cb), (gpointer)"Print" );
   gtk_table_attach_defaults( GTK_TABLE(table), button, 15, 19, 4, 5 );
   gtk_widget_show( button );

   /* return the container reference to the user */
   *c = table;
}

/*-----------------------------------------------------------------------
**  create_display_spectra_b_widgets() - create the spectra A parameters widgets.
**-----------------------------------------------------------------------
*/
void create_display_spectra_b_widgets( GtkWidget ** c )
{
   GtkWidget * table,
	          * entry,
	          * label,
	          * checkbutton,
             * button;

   table = gtk_table_new( 5, 20, TRUE );
   gtk_widget_set_size_request( GTK_WIDGET(table), 475, 125 );

   // objbin       
   button = gtk_button_new_with_label( "Set ObjBin" );
   g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(sb_setobjbin_cb), (gpointer)NULL );
   gtk_table_attach_defaults( GTK_TABLE(table), button, 0, 3, 0, 1 );
   gtk_widget_show( button );

   label = gtk_label_new( "()" );
   gtk_misc_set_alignment( GTK_MISC(label), 0.5, 0.5 );
   gtk_table_attach_defaults( GTK_TABLE(table), label, 3, 5, 0, 1 );
   gtk_widget_show( label );
	W.sb_objbin_label = label;

   entry = gtk_entry_new( );
   gtk_table_attach_defaults( GTK_TABLE(table), entry, 5, 9, 0, 1);
   g_signal_connect( G_OBJECT(entry), "activate", 
		G_CALLBACK(entry_cb), (gpointer)"SBObjBin ");
	g_signal_connect (G_OBJECT (entry), "focus-out-event",
		G_CALLBACK (entry_fo_cb), (gpointer)"SBSkyBin " );
   gtk_widget_show( entry );
	W.sb_objbin = entry;

   // skybin       
   button = gtk_button_new_with_label( "Set SkyBin" );
   g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(sb_setskybin_cb), (gpointer)NULL );
   gtk_table_attach_defaults( GTK_TABLE(table), button, 0, 3, 1, 2 );
   gtk_widget_show( button );

   label = gtk_label_new( "()" );
   gtk_misc_set_alignment( GTK_MISC(label), 0.5, 0.5 );
   gtk_table_attach_defaults( GTK_TABLE(table), label, 3, 5, 1, 2 );
   gtk_widget_show( label );
	W.sb_skybin_label = label;

   entry = gtk_entry_new( );
   gtk_table_attach_defaults( GTK_TABLE(table), entry, 5, 9, 1, 2);
   g_signal_connect( G_OBJECT(entry), "activate", 
		G_CALLBACK(entry_cb), (gpointer)"SBSkyBin ");
	g_signal_connect (G_OBJECT (entry), "focus-out-event",
		G_CALLBACK (entry_fo_cb), (gpointer)"SBSkyBin " );
   gtk_widget_show( entry );
	W.sb_skybin = entry;

   // Show: DIFF, OBJ, SKY
   label = gtk_label_new( "Show:" );
   gtk_misc_set_alignment( GTK_MISC(label), 1, 0.5 );
   gtk_table_attach_defaults( GTK_TABLE(table), label, 0, 3, 2, 3 );
   gtk_widget_show( label );

   checkbutton = gtk_check_button_new_with_label("Diff");
   g_signal_connect( G_OBJECT(checkbutton), "toggled",
      G_CALLBACK (sb_show_cb), (gpointer) NULL );
   gtk_table_attach_defaults (GTK_TABLE(table), checkbutton, 3, 5, 2, 3 );
   gtk_widget_show( checkbutton );
	W.sb_show_diff = checkbutton;

   checkbutton = gtk_check_button_new_with_label("Obj");
   g_signal_connect( G_OBJECT(checkbutton), "toggled",
      G_CALLBACK (sb_show_cb), (gpointer) NULL );
   gtk_table_attach_defaults (GTK_TABLE(table), checkbutton, 5, 7, 2, 3 );
   gtk_widget_show( checkbutton );
	W.sb_show_obj = checkbutton;

   checkbutton = gtk_check_button_new_with_label("Sky");
   g_signal_connect( G_OBJECT(checkbutton), "toggled",
      G_CALLBACK (sb_show_cb), (gpointer) NULL );
   gtk_table_attach_defaults (GTK_TABLE(table), checkbutton, 7, 9, 2, 3 );
   gtk_widget_show( checkbutton );
	W.sb_show_sky = checkbutton;

   // YAutoScale
   checkbutton = gtk_check_button_new_with_label("YAutoScale");
   g_signal_connect( G_OBJECT(checkbutton), "toggled",
      G_CALLBACK (checkbutton_offon_cb), (gpointer) "SBYAutoScale" );
   gtk_table_attach_defaults (GTK_TABLE(table), checkbutton, 15, 19, 0, 1 );
   gtk_widget_show(checkbutton);
   W.sb_yautoscale = checkbutton;

   // Diff Y Range 
   label = gtk_label_new( "DiffYRange " );
   gtk_misc_set_alignment( GTK_MISC(label), 1, 0.5 );
   gtk_table_attach_defaults( GTK_TABLE(table), label, 10, 15, 1, 2 );
   gtk_widget_show( label );

   entry = gtk_entry_new( );
   gtk_table_attach_defaults( GTK_TABLE(table), entry, 15, 19, 1, 2);
   g_signal_connect( G_OBJECT(entry), "activate", 
		G_CALLBACK(entry_cb), (gpointer)"SBDiffYRange");
	g_signal_connect (G_OBJECT (entry), "focus-out-event",
		G_CALLBACK (entry_fo_cb), (gpointer)"SBDiffYRange" );
   gtk_widget_show( entry );
	W.sb_diffyrange = entry;

   // Data Y Range 
   label = gtk_label_new( "DataYRange " );
   gtk_misc_set_alignment( GTK_MISC(label), 1, 0.5 );
   gtk_table_attach_defaults( GTK_TABLE(table), label, 10, 15, 2, 3 );
   gtk_widget_show( label );

   entry = gtk_entry_new( );
   gtk_table_attach_defaults( GTK_TABLE(table), entry, 15, 19, 2, 3);
   g_signal_connect( G_OBJECT(entry), "activate", 
		G_CALLBACK(entry_cb), (gpointer)"SBDataYRange");
	g_signal_connect (G_OBJECT (entry), "focus-out-event",
		G_CALLBACK (entry_fo_cb), (gpointer)"SBDataYRange" );
   gtk_widget_show( entry );
	W.sb_datayrange = entry;

   // X Scale Fixed
   button = gtk_button_new_with_label( "Set XScale" );
   g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(sb_setxscale_cb), (gpointer)NULL );
   gtk_table_attach_defaults( GTK_TABLE(table), button, 12, 15, 3, 4 );
   gtk_widget_show( button );

   entry = gtk_entry_new( );
   gtk_table_attach_defaults( GTK_TABLE(table), entry, 15, 19, 3, 4);
   g_signal_connect( G_OBJECT(entry), "activate", 
		G_CALLBACK(entry_cb), (gpointer)"SBXScale");
	g_signal_connect (G_OBJECT (entry), "focus-out-event",
		G_CALLBACK (entry_fo_cb), (gpointer)"SBXScale" );
   gtk_widget_show( entry );
	W.sb_xscale = entry;

   /*------------------------------
   ** print button
   */
   button = gtk_button_new_with_label( "Print" );
   g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(button_cb), (gpointer)"Print" );
   gtk_table_attach_defaults( GTK_TABLE(table), button, 15, 19, 4, 5 );
   gtk_widget_show( button );

   /* return the container reference to the user */
   *c = table;
}

/*-----------------------------------------------------------------------
**  create_about_widgets() - create widgets for parameter container.
**-----------------------------------------------------------------------
*/
void create_about_widgets ( GtkWidget ** c )
{
   GtkWidget * table;
   GtkWidget * label;

   char * about_msg =
      "DV is a FITS Data Viewer for the IRTF\n"
      "\n"
      "Developed by the NASA Infared Telescope Facility\n"
      "http://irtfweb.ifa.hawaii.edu\n";

   table = gtk_table_new( 7, 10, TRUE );
   gtk_widget_set_size_request( GTK_WIDGET(table), 500, 125 );

   label = gtk_label_new( about_msg );
   gtk_table_attach_defaults( GTK_TABLE(table), label, 2, 8, 1, 6 );
   gtk_widget_show(label);

   *c = table;
}

/********************************************************************************/
/*  Macro dialog                                                                */
/********************************************************************************/

/*-----------------------------------------------------------------------
**  create_macro_widgets() - create widgets for controlling macro
**    execution.
**-----------------------------------------------------------------------
*/
void create_macro_widgets( GtkWidget ** c )
{
   GtkWidget * table,
             * entry,
             * button,
             * label;

   table = gtk_table_new( TRUE, 8, 10 );
	gtk_widget_set_size_request( GTK_WIDGET(table), 500, 125 );

   /*-----------------------------
   ** Path button & text entry
	** Also create the browseForPath dialog
   */
   button = gtk_button_new_with_label("(set) Path:");
   gtk_table_attach_defaults(GTK_TABLE(table), button, 0, 2, 0, 1);
   g_signal_connect( G_OBJECT(button), "clicked", G_CALLBACK (m_set_path_cb), 
	   (gpointer) &W.bfp_macro );
   gtk_widget_show(button);

   entry = gtk_entry_new( );
   gtk_entry_set_text( GTK_ENTRY(entry), Md.path );
   g_signal_connect( G_OBJECT(entry), "activate", G_CALLBACK (m_path_cb), (gpointer) NULL );
	g_signal_connect (G_OBJECT(entry), "focus-out-event", G_CALLBACK (m_path_fo_cb), (gpointer)NULL );
   gtk_table_attach_defaults(GTK_TABLE(table), entry, 2, 6, 0 , 1);
   gtk_widget_show( entry );
   Md.path_w = entry;

   // creates the macro Browse for Path
   bfp_create_dialog( &W.bfp_macro, "Browse for Macro Path");
	bfp_set_ok_cb( &W.bfp_macro, G_CALLBACK(bfp_macro_ok_cb) );         /* assign a cb to the OK button */

   /*-----------------
   ** Filename Mask
   */
   label = gtk_label_new( " Mask: " );
   gtk_table_attach_defaults(GTK_TABLE(table), label, 6, 8, 0 , 1);
   gtk_widget_show( label );

   entry = gtk_entry_new( );
   gtk_entry_set_text( GTK_ENTRY(entry), Md.filemask ); 
	gtk_table_attach_defaults(GTK_TABLE(table), entry, 8, 10, 0 , 1);
   g_signal_connect( G_OBJECT(entry), "activate", G_CALLBACK (m_filemask_cb), NULL );
	g_signal_connect (G_OBJECT(entry), "focus-out-event", G_CALLBACK (m_filemask_fo_cb), (gpointer)NULL );
   gtk_widget_show( entry );
   Md.file_mask_w = entry;

	/*-------------------------------------------------
	** filelist - display files in the Maco Path.
	*/

	struct sclist_view_t *list;

	list = sclist_view_new( "Files" );
	Md.file_list  = GTK_WIDGET(list->tree_view_w);
	Md.file_store = list->list_store_w;
	gtk_table_attach_defaults(GTK_TABLE(table), list->s_window_w, 0, 4, 1, 7);
	g_signal_connect( G_OBJECT(list->tree_view_w), "cursor-changed", G_CALLBACK (m_filelist_cb), NULL );

   /*---------------------------------------------------------------
   ** text view & buffer to display contents of file.
	*/
	{
		GtkWidget     *swind;
		GtkWidget     *view;
		GtkTextBuffer *buffer;

		// scroll swindow to hold text view
		swind  = gtk_scrolled_window_new(NULL, NULL);
		gtk_table_attach_defaults(GTK_TABLE(table), swind, 4, 10, 1, 7);  // swind attaches to table.
		gtk_widget_show( swind );

		// text view to show text buffer
		view   = gtk_text_view_new();
		gtk_text_view_set_editable( GTK_TEXT_VIEW(view), FALSE);
		gtk_container_add(GTK_CONTAINER(swind), view);    
		gtk_widget_show( view );
		Md.text_view_w = view;

		// text buffer for content of file
		buffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW(view));
		Md.text_buffer_w  = buffer;
	}

   /* Execute button */
   button = gtk_button_new_with_label( "Execute" );
	gtk_table_attach_defaults(GTK_TABLE(table), button, 0, 2, 7,  8);
   g_signal_connect( G_OBJECT(button), "clicked", G_CALLBACK(m_execute_cb), NULL );
   gtk_widget_show( button );

   /* Stop button to stop macro execution */
   button = gtk_button_new_with_label( "Stop" );
	gtk_table_attach_defaults(GTK_TABLE(table), button, 2, 4, 7,  8);
   g_signal_connect( G_OBJECT(button), "clicked", G_CALLBACK(m_stop_cb), NULL );
   gtk_widget_show( button );

   /* Refreash button to refresh display of file list */
   button = gtk_button_new_with_label( "Refresh" );
	gtk_table_attach_defaults(GTK_TABLE(table), button, 6, 8, 7,  8);
   g_signal_connect( G_OBJECT(button), "clicked", G_CALLBACK(m_refresh_cb), NULL );
   gtk_widget_show( button );

   // don't need hide if in a tab.
#if 0
   /* Hide button to cancel/hide entire dialog box */
   button = gtk_button_new_with_label( "Hide" );
	gtk_table_attach_defaults(GTK_TABLE(table), button, 8, 10, 7,  8);
   g_signal_connect( G_OBJECT(button), "clicked", G_CALLBACK(widget_hide_cb), (gpointer) window );
   gtk_widget_show( button );
#endif

   /* return window handle to caller */
   *c = table;
}

/************************************************************************
**  helper functions
*************************************************************************
*/

/*-----------------------------------------------------------------------
**  MyCreateCheckButtons() - Creates a set of check buttons with labels 
**     packed inside a box (hbox or vbox).
**     Returns the pointer to the container (vbox or hbox).
**-----------------------------------------------------------------------
*/ 
GtkWidget *  MyCreateCheckButtons(
   char * selection[],               /* selection_list of radio buttons */
   GtkWidget *check[],               /* created check_button's reference returned here */
   int is_hbox,                      /* TRUE for hbox, else vbox */
   GCallback checkbutton_cb          /* callback on selection of menu items */
)  
{
   int inx;
   GtkWidget *box;

   /* make homogeneous box */
   if( is_hbox )
      box = gtk_hbox_new( TRUE, 0 );
   else
      box = gtk_vbox_new( TRUE, 0 );
   
   inx = 0;
   while( *selection )
   {
      check[inx] = gtk_check_button_new_with_label( *selection );
      g_signal_connect( G_OBJECT( check[inx] ), "released",
         G_CALLBACK(checkbutton_cb), (gpointer) (intptr_t)inx );
      gtk_box_pack_start( GTK_BOX(box),  check[inx] , TRUE, TRUE, 0 );
      gtk_widget_show( check[inx] );

      selection++;
      inx++;
   }

   return box;
}

/*-----------------------------------------------------------------------
**  MyCreateRadioButtons() - Creates a set of radio buttons with labels
**     packed inside a box (hbox or vbox).
**     Returns the pointer to the container (vbox or hbox).
**-----------------------------------------------------------------------
*/
GtkWidget *  MyCreateRadioButtons(
   char * selection[],         // selection_list of radio buttons 
   GtkWidget *radio[],         // created radio_button's reference returned here 
   int is_hbox,                // TRUE for hbox, else vbox 
   int draw_indicator,         // TRUE(separate indicator) or FALSE (looks like button).
   GCallback radiobutton_cb    // callback on selection of menu items 
)
{
   int inx;
   GtkWidget *box;
   GSList  * group;

   /* make homogeneous box */
   if( is_hbox )
      box = gtk_hbox_new( TRUE, 0 );
   else
      box = gtk_vbox_new( TRUE, 0 );

   inx = 0;
   group = NULL;
   while( *selection )
   {
      /* 1st call is NULL, each additional call extends the grouping */
      if( inx > 0 )
         group = gtk_radio_button_get_group( GTK_RADIO_BUTTON(radio[inx-1] ));

      radio[inx] = gtk_radio_button_new_with_label( group, *selection );
      if( draw_indicator == FALSE )
         gtk_toggle_button_set_mode( GTK_TOGGLE_BUTTON(radio[inx]), FALSE) ;
      g_signal_connect( G_OBJECT( radio[inx] ), "released",
         G_CALLBACK(radiobutton_cb), (gpointer) (intptr_t)inx );
      gtk_box_pack_start( GTK_BOX(box),  radio[inx] , TRUE, TRUE, 0 );

      gtk_widget_show( radio[inx] );
      selection++;
      inx++;
   }
   return box;
}

/*----------------------------------------------------------------------------
**  MyComboBoxCreate() - creates a combobox from a selection list (NULL terminated
**     array of strings).
**----------------------------------------------------------------------------
*/ 
GtkWidget* MyComboBoxCreate(
   char * selection_list[]        // items in the combo box.
)
{
   int i;
   GtkWidget * cbox;

#if (GTK_MAJOR_VERSION == 2) && (GTK_MINOR_VERSION < 24)
   // centos 6.5 is GTK 2.20.
   cbox = gtk_combo_box_new_text();

   for( i=0; selection_list[i] != NULL; i++)
   {
      gtk_combo_box_append_text( GTK_COMBO_BOX(cbox), selection_list[i] );
   }
   gtk_combo_box_set_active( GTK_COMBO_BOX(cbox), 0 );   // select the 1st element
#else 
   // centos 6.6 is GTK 2.24 - can use newer combobox API
   cbox = gtk_combo_box_text_new();

   for( i=0; selection_list[i] != NULL; i++)
   {
      gtk_combo_box_text_append_text( GTK_COMBO_BOX_TEXT(cbox), selection_list[i] );
   }
   gtk_combo_box_set_active( GTK_COMBO_BOX(cbox), 0 );   // select the 1st element
#endif
   return cbox;
}

/*-----------------------------------------------------------------------
**  MyCreateMenuFromSelection() - Helper function to create menus for selection[] arrays.
**     The call back menuitem_cb will get passwd the index value of the menu.
**     Each menuitem's user data is set to user_data.
**-----------------------------------------------------------------------
*/ 
GtkWidget *  MyCreateMenuFromSelection(
   char * selection[],        /* null terminating array for list string entry */
   int tearoff,               /* Is this a tearoff menu? */
   GCallback menuitem_cb,     /* callback on selection of menu items */
	 gpointer user_data        /* user data assigned to each menu_item */
)
{  
   int inx;
   GtkWidget *menu;
   GtkWidget *menu_item;

   /* create menu item that holds the pop ups */
   menu = gtk_menu_new();
   
   /* Pop Up option */ 
   if( tearoff )
   {
      menu_item = gtk_tearoff_menu_item_new();
      gtk_menu_shell_append( GTK_MENU_SHELL(menu), menu_item );
      gtk_widget_show( menu_item);
   }

   /* create menu-entries and put them into menu */
   inx = 0;
   while( *selection )
   {
	   printf("MyCreateMenuFromSelection %s %d \n", *selection, inx );
      menu_item = gtk_menu_item_new_with_label( *selection );
      gtk_menu_shell_append( GTK_MENU_SHELL(menu), menu_item );

      g_object_set_data( G_OBJECT(menu_item), MENU_FROM_SELECTION_KEY, (gpointer) user_data);

      if( menuitem_cb )
         g_signal_connect( G_OBJECT(menu_item), "activate",
            G_CALLBACK(menuitem_cb), (gpointer) (intptr_t)inx );
   
      gtk_widget_show( menu_item );
   
      selection++;
      inx++;
   }
   return menu;
}  

/*-----------------------------------------------------------------------
**  MyStyleSetItemColor() - Helper function to change a style.
**-----------------------------------------------------------------------
*/
void MyStyleSetItemColor(
   GdkColor   color,          /* The allocated color to be added to the style */
   char       item,           /* the item to which the color is to be applied */
                              /* 'f' = foreground; 'b' = background;          */
                              /* 'l' = light;      'd' = dark;                */
                              /* 'm' = mid;        't' = text;                */
                              /* 's' = base.                                  */
   GtkStyle * style           /* The old style - changes made to a copy       */
)
{
   int i;
   switch ( item )
   {
      case 'f':
      case 'F':
         for ( i = 0; i < 5; i++ )
            style->fg[i] = color;
         break;
      case 'b':
      case 'B':
         for ( i = 0; i < 5; i++ )
            style->bg[i] = color;
         break;
      case 'l':
      case 'L':
         for ( i = 0; i < 5; i++ )
            style->light[i] = color;
         break;
      case 'd':
      case 'D':
         for ( i = 0; i < 5; i++ )
            style->dark[i] = color;
         break;
      case 'm':
      case 'M':
         for ( i = 0; i < 5; i++ )
            style->mid[i] = color;
         break;
      case 't':
      case 'T':
         for ( i = 0; i < 5; i++ )
            style->text[i] = color;
         break;
      case 's':
      case 'S':
         for ( i = 0; i < 5; i++ )
            style->base[i] = color;
         break;
   }
}

#if USE_PANGO
/*---------------------------------------------------------------------------
**  app_font_init() - return a pointer to app_font_info_t with a 
**  pango Font, and Layout reference, and font information ( wid, hgt,etc).
**  Function gets all information you need to render text in a drawing area.
**---------------------------------------------------------------------------
*/
int get_font(
   struct font_info_t * myfont, // O: Font information returned here
   GtkWidget * gtk_widget,      // I: need to reference a widget. ie: base_window
   char * font_name             // I: name of pango font to allocate.
)
{
   PangoContext *context;
   PangoFontMetrics *metrics;

   if( (myfont->font = pango_font_description_from_string (font_name)) == NULL )
   {
      printf("get_font()->pango_font_description_from_string() error!\n");
      return ERR_NOT_AVAILABLE;
   }

   if( (context = gtk_widget_create_pango_context (gtk_widget)) == NULL )
   {
      printf("get_font()->gtk_widget_create_pango_context() error!\n");
      return ERR_NOT_AVAILABLE;
   }

   if( (myfont->layout = pango_layout_new ( context )) == NULL )
   {
      printf("get_font()->pango_layout_new() error!\n");
      return ERR_NOT_AVAILABLE;
   }

   pango_layout_set_font_description (myfont->layout, myfont->font);

   // determine width & height of font.
   metrics = pango_context_get_metrics (context, myfont->font,
                      pango_context_get_language(context));

   myfont->wid = PANGO_PIXELS( pango_font_metrics_get_approximate_digit_width(metrics) );
   myfont->ascent = PANGO_PIXELS( pango_font_metrics_get_ascent (metrics) );
   myfont->descent = PANGO_PIXELS( pango_font_metrics_get_descent (metrics) );
   myfont->hgt = myfont->ascent + myfont->descent;

   g_object_unref (context);
   pango_font_metrics_unref(metrics);

   printf("get_font() got[%s] size=%dx%d \n", font_name, myfont->wid, myfont->hgt);
   return 0;
}
#endif
