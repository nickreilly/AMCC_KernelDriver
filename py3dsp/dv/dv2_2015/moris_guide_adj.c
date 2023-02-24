/********************************************************************************/
/*  mga_dialog.c - dialog box to adjust moris's guidebox from DV              */
/********************************************************************************/
/*
** Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php
** Copyright (C) 2012 <NASA IRTF. http://irtfweb.ifa.hawaii.edu>
*/

#define EXTERN extern

/*-------------------------
**  Standard include files
**-------------------------
*/

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <dirent.h>

#include <gtk/gtk.h>

/*------------------------------
**  Non-standard include files
**------------------------------
*/
#include "dv.h"           /* DV MAIN APPLICATION  */

#define SPEX_PS   (0.1160)
#define MORIS_PS  (0.1143)
#define MORIS_ROT (0.0)

struct moris_guide_info_t {
   int   is_guiding;     // T or F, are we guiding.
	float tx;             // target XY
	float ty;
	int   x;              // centroid box x,y WxH
	int   y;
	int   wid;
	int   hgt;
};

/* prototype static functions */

static int  mga_delete_event( GtkWidget *widget, GdkEvent  *event, gpointer data );
static void mga_ok_cb( GtkWidget *widget, gpointer data );
static void mga_cancel_cb( GtkWidget *widget, gpointer data );
static void mga_cal_offset_cb( GtkWidget *widget, gpointer data );
static void mga_apply_offset_cb( GtkWidget *widget, gpointer data );
static void mga_test_cb( GtkWidget *widget, gpointer data );

static int mga_peak ( double *cx, double *cy, struct df_buf_t *bp,  struct array_t *box);
static int mga_centroid ( double *r_cx, double *r_cy, struct df_buf_t *bp, struct array_t *box );
static int mga_bright ( double *r_cx, double *r_cy, struct df_buf_t *bp, struct array_t *box );
static int mga_faint  ( double *r_cx, double *r_cy, struct df_buf_t *bp, struct array_t *box );
static int mga_very_faint  ( double *r_cx, double *r_cy, struct df_buf_t *bp, struct array_t *box );

static void mga_cal_centroid ( double *r_cx, double *r_cy, double *data, int wid, int hgt );
static void mga_do_filter1 ( double *data, int wid, int hgt );
static void mga_do_filter2 ( double *data, int wid, int hgt );
static  int mga_do_smooth  ( double *data, int wid, int hgt );
static  int mga_do_smooth2 ( double *data, int wid, int hgt );
static int qsort_comp_double( const void *a, const void *b);

int moris_guide_query( struct moris_guide_info_t *moris_info,  char *moris_host );

/*-----------------------------------------------------------------------
** mga_create_dialog() - creates the moris dialog.
**-----------------------------------------------------------------------
*/
int mga_create_dialog ( struct moris_guide_adj_t *mga )
{
   GtkWidget * table;
   GtkWidget * button;
   GtkWidget * label;
   GtkWidget * cbox;

   char * my_buffer_selection[4]  = { "A", "B", "C", NULL  }; // use allow ABC for buffers
   char * center_selection[6]     = { "peak", "centroid", "bright", "faint", "very_faint", NULL  }; 

   /* initalize structure */
   mga->dialog_window = NULL;

   /*---------------------------------
   ** Create mga->dialog_window
   */
   mga->dialog_window = gtk_window_new( GTK_WINDOW_TOPLEVEL );
   gtk_window_set_title( GTK_WINDOW (mga->dialog_window), "DV2: MORIS's GuideBox Adjustment and TCS Offset" );
   gtk_window_set_resizable( GTK_WINDOW(mga->dialog_window), TRUE );

   g_signal_connect( G_OBJECT( mga->dialog_window), "delete_event", G_CALLBACK(mga_delete_event), NULL );
	g_signal_connect( G_OBJECT( mga->dialog_window), "destroy", G_CALLBACK(mga_delete_event), NULL );

   /* table to contain my widgets */
   table = gtk_table_new( 10, 100, TRUE );
	gtk_widget_set_size_request( GTK_WIDGET(table), 500, 300);
   gtk_container_add (GTK_CONTAINER (mga->dialog_window), table);

   /*---------------------------------
   **  1. Setup 
   */
	label = gtk_label_new( "1. Setup options, and put a ObjBox on the star." );
	gtk_misc_set_alignment( GTK_MISC(label), 0, 0.5 );
	gtk_table_attach_defaults( GTK_TABLE(table), label, 10, 90, 0, 1 );
	gtk_widget_show( label );

   // Buffer that contains image
	label = gtk_label_new( "Buffer:");
	gtk_misc_set_alignment( GTK_MISC(label), 1, 0.5 );
	gtk_table_attach_defaults( GTK_TABLE(table), label, 10, 20, 1, 2 );
	gtk_widget_show( label );

   cbox = MyComboBoxCreate( my_buffer_selection );
   gtk_table_attach_defaults( GTK_TABLE(table), cbox, 20, 30, 1, 2 );
	gtk_widget_show(cbox);
	mga->bufinx = cbox;

   // Centroid method 
	label = gtk_label_new( "Center:");
	gtk_misc_set_alignment( GTK_MISC(label), 1, 0.5 );
	gtk_table_attach_defaults( GTK_TABLE(table), label, 40, 50, 1, 2 );
	gtk_widget_show( label );

   cbox = MyComboBoxCreate( center_selection );
	gtk_combo_box_set_active( GTK_COMBO_BOX(cbox), 2); // default to bright
   gtk_table_attach_defaults( GTK_TABLE(table), cbox, 50, 70, 1, 2 );
	gtk_widget_show(cbox);
	mga->center_method = cbox;

   /*---------------------------------
   **  2. Calculate Offsets
   */
   button = gtk_button_new_with_label( "2. Calculate Offsets");
	label = gtk_bin_get_child(GTK_BIN(button));  // set the alignment of the button's label
	gtk_misc_set_alignment( GTK_MISC(label), 0, 0.5 );
   gtk_table_attach_defaults( GTK_TABLE(table), button,  10, 90, 3, 4 );
   g_signal_connect( G_OBJECT (button), "clicked", G_CALLBACK(mga_cal_offset_cb ), (gpointer) mga );
   gtk_widget_show(button);

	label = gtk_label_new( " ");
	gtk_misc_set_alignment( GTK_MISC(label), 0.1, 0.5 );
	gtk_table_attach_defaults( GTK_TABLE(table), label,  10, 90, 4, 6 );
	gtk_widget_show( label );
	mga->cal_label = label;

   /*---------------------------------
   **  3. Apply offsets 
   */
   button = gtk_button_new_with_label( "3. Apply Offsets to TCS and MORIS");
	label = gtk_bin_get_child(GTK_BIN(button));  // set the alignment of the button's label
	gtk_misc_set_alignment( GTK_MISC(label), 0, 0.5 );
   gtk_table_attach_defaults( GTK_TABLE(table), button,  10, 90, 6, 7 );
   g_signal_connect( G_OBJECT (button), "clicked", G_CALLBACK(mga_apply_offset_cb ), (gpointer) mga );
   gtk_widget_show(button);

	label = gtk_label_new( " ");
	gtk_misc_set_alignment( GTK_MISC(label), 0.1, 0.5 );
	gtk_table_attach_defaults( GTK_TABLE(table), label,  10, 90, 7, 9 );
	gtk_widget_show( label );
	mga->apply_label = label;

   /*---------------------------------
   **  TEST button 
   */
   button = gtk_button_new_with_label( "TEST");
   gtk_table_attach_defaults( GTK_TABLE(table), button, 20, 30, 9, 10);
   g_signal_connect( G_OBJECT (button), "clicked", G_CALLBACK(mga_test_cb ), (gpointer) NULL );
   //gtk_widget_show(button);

   /*---------------------------------
   **  Cancel button to hide dialog
   */
   button = gtk_button_new_with_label( "Cancel");
   gtk_table_attach_defaults( GTK_TABLE(table), button, 70, 95, 9, 10);
   g_signal_connect( G_OBJECT (button), "clicked", G_CALLBACK(mga_cancel_cb ), (gpointer) mga );
   gtk_widget_show(button);

   /* intialize some mga variables */
	mga->data_ok = FALSE;   

   gtk_widget_show( table );

   return 0;
}

/*-----------------------------------------------------------------------
** moris_show_dialg() - show the window.
**-----------------------------------------------------------------------
*/
void mga_show_dialog ( struct moris_guide_adj_t *mga )
{
   gtk_widget_show( mga->dialog_window );
   gdk_window_raise( mga->dialog_window->window );
}

/*-----------------------------------------------------------------------
** mga_hide_dialog() - hides the mga window.
**-----------------------------------------------------------------------
*/
void mga_hide_dialog ( struct moris_guide_adj_t *mga )
{
   gtk_widget_hide( mga->dialog_window );
}


/***************************************************************************
**  mga's static (private) functions
***************************************************************************
*/

/*-----------------------------------------------------------------------
**  mga_delete_event()
**-----------------------------------------------------------------------
*/
static int mga_delete_event( GtkWidget *widget, GdkEvent  *event, gpointer data )
{
   /* Return FALSE - gtk will call base_destroy_cb();
   **         TRUE - gtk will ignore delete event.
   */
   gtk_widget_hide( GTK_WIDGET(widget) );
   return(TRUE);
}

/*-----------------------------------------------------------------------
** mga_ok_cb() - the defaule ok button callback.
**-----------------------------------------------------------------------
*/
static void mga_ok_cb( GtkWidget *widget, gpointer data )
{
   struct moris_guide_adj_t *mga = (struct moris_guide_adj_t *)data;

   mga_hide_dialog ( mga );
}

/*-----------------------------------------------------------------------
** mga_cancel_cb() - the defaule cancel callback. Just hides the dialog
**   window.
**-----------------------------------------------------------------------
*/
static void mga_cancel_cb( GtkWidget *widget, gpointer data )
{
   mga_hide_dialog ( (struct moris_guide_adj_t *)data );
}

/*-----------------------------------------------------------------------
** mga_cal_offset_cb() - mga_cal_offset_cb 
**-----------------------------------------------------------------------
*/
static void mga_cal_offset_cb( GtkWidget *widget, gpointer data )
{
   int rc, i, 
	    ns_dir,
	    bufinx, 
	    method,
		 slit;
   char buf[256];
   char *str_ptr;

   double end_xy[2];
   double beg_xy[2];
   double gd_arcsec_pixel;

   struct array_t box;
   struct df_buf_t *bp; 
	struct df_fheader_t * f_hdr;

   struct moris_guide_adj_t *mga = (struct moris_guide_adj_t *)data;
   bufinx = gtk_combo_box_get_active(GTK_COMBO_BOX(mga->bufinx));
	method = gtk_combo_box_get_active(GTK_COMBO_BOX(mga->center_method));
   bp = &Buffer[bufinx];
	slit = 0;
	beg_xy[0] = beg_xy[1] = 0;

	printf("mga_cal_offset_cb(): bufinx=%d  method=%d\n", bufinx, method);
	mga->data_ok = FALSE;   

   // clear the apply label
	gtk_label_set_text( GTK_LABEL(mga->apply_label), " ");

   /*-------------------------------------------------
	** get slit, end_xy, plate scale of guidedog. And initialize moris plat 
	*/
	if( bp->status==DF_EMPTY )
	{
	   gtk_label_set_text( GTK_LABEL(mga->cal_label), "Buffer is empty. No action");
	   return;
	}
   if( df_search_fheader( bp->fheader, "SLIT", buf, sizeof(buf), &f_hdr, &i, FALSE) >=0 )
   {
       if( ((slit=parseSelection_r( buf, " /\n", &str_ptr, Gd_slit_selection)) < 0))
		 {
			gtk_label_set_text( GTK_LABEL(mga->cal_label), "No slit information. No action");
			return;
		 }
   }
	end_xy[0] = Gd_slit_auto_guidebox_a[slit].x; 
	end_xy[1] = Gd_slit_auto_guidebox_a[slit].y;
   gd_arcsec_pixel = (bp->arcsec_pixel < 0.01 ? SPEX_PS :  bp->arcsec_pixel);   // guider's platescale 
	printf("   SLIT=%s end_xy=(%.1f,%.1f) PS=%.2f POSANG=%.2f\n", 
		Gd_slit_selection[slit], end_xy[0], end_xy[1], gd_arcsec_pixel, bp->pos_angle);

   /*-------------------------------------------------
	** Get From XY by centroiding on stats box 
	*/
	box.x   = Stats[bufinx].objx;
	box.y   = Stats[bufinx].objy;
	box.wid = Stats[bufinx].objwid;
	box.hgt = Stats[bufinx].objhgt;
	if( ((box.x+box.wid-1) > bp->naxis1) || ((box.y+box.hgt-1) > bp->naxis2) )
	{
	   gtk_label_set_text( GTK_LABEL(mga->cal_label), "Bad Obj Box location. No action");
	   return;
	}

   rc = ERR_NONE;
   switch( method )
	{
	   case 0: // peak
			rc = mga_peak( &beg_xy[0], &beg_xy[1], bp, &box );
			printf("       peak: %.1f %.1f \n", beg_xy[0], beg_xy[1]);
		   break;
	   case 1: // centroid
			rc = mga_centroid( &beg_xy[0], &beg_xy[1], bp, &box );
			printf("   centroid: %.1f %.1f \n", beg_xy[0], beg_xy[1]);
		   break;
	   case 2: // bright
		default:
			rc = mga_bright( &beg_xy[0], &beg_xy[1], bp, &box );
			printf("     bright: %.1f %.1f \n", beg_xy[0], beg_xy[1]);
		   break;
	   case 3: // faint
			rc = mga_faint( &beg_xy[0], &beg_xy[1], bp, &box );
			printf("      faint: %.1f %.1f \n", beg_xy[0], beg_xy[1]);
		   break;
	   case 4: // very_faint
			rc = mga_very_faint( &beg_xy[0], &beg_xy[1], bp, &box );
			printf(" very_faint: %.1f %.1f \n", beg_xy[0], beg_xy[1]);
		   break;
	}
	if( rc )
	{
	   gtk_label_set_text( GTK_LABEL(mga->cal_label), "Error on finding object center. No action");
	   return;
	}

   /*-------------------------------------------------------------
	** Draw from-to lines on image
	*/
	sprintf(buf, "%s %.0f %.0f 1 1 ", buffer_selection[bufinx], beg_xy[0], beg_xy[1]);
	do_drawbox( 0, buf );
	sprintf(buf, "%s %.0f %.0f 1 1 ", buffer_selection[bufinx], end_xy[0], end_xy[1]);
	do_drawbox( 0, buf );
	sprintf(buf, "%s %.0f %.0f %.0f %.0f ", 
	             buffer_selection[bufinx], beg_xy[0], beg_xy[1], end_xy[0], end_xy[1]);
	printf("DrawLine %s \n", buf);
	do_drawline( 0, buf );

   // set the StatsXORLine
#if 0
	sprintf(buf, "%.0f %.0f %.0f %.0f %s ", 
	             beg_xy[0], beg_xy[1], end_xy[0], end_xy[1], buffer_selection[bufinx]);
	do_statsxorline( 0, buf );
#endif

   /*-------------------------------------------------------------
	** Determine TCS and Moris Offset.
	*/

	mga->beg_xy[0] = beg_xy[0];
	mga->beg_xy[1] = beg_xy[1];
	mga->end_xy[0] = end_xy[0];
	mga->end_xy[1] = end_xy[1];
	mga->gd_arcsec_pixel = gd_arcsec_pixel;
	mga->gd_rot_deg = bp->pos_angle;  
	mga->moris_arcsec_pixel = MORIS_PS;
	mga->moris_rot_deg = MORIS_ROT;


	// we can flip the NS axis by setting the value for ns_dir to 1 or -1.
	ns_dir = (Lc.image_compass_flipNS ? -1 : 1 );

   // cal tcs Offsets
	rotate_pt( &mga->tcs_offset[0], &mga->tcs_offset[1],
		mga->gd_arcsec_pixel * (mga->end_xy[0]-mga->beg_xy[0]),
		ns_dir * mga->gd_arcsec_pixel * (mga->end_xy[1]-mga->beg_xy[1]), 
		mga->gd_rot_deg);

   // cal moris guidebox adjustment 
	rotate_pt( &mga->moris_adj[0], &mga->moris_adj[1],
		(mga->gd_arcsec_pixel/mga->moris_arcsec_pixel) * (mga->end_xy[0]-mga->beg_xy[0]),
		ns_dir * (mga->gd_arcsec_pixel/mga->moris_arcsec_pixel) * (mga->end_xy[1]-mga->beg_xy[1]), 
		mga->moris_rot_deg+mga->gd_rot_deg);

	mga->data_ok = TRUE;     // flag data as OK.

	printf("MGA OK %d \n", mga->data_ok); 
	printf("  FROM %.1f,%.1f TO %.1f,%.1f: Offset: %.1f %.1f  \n", 
	   mga->beg_xy[0], mga->beg_xy[1], mga->end_xy[0], mga->end_xy[1],
		(mga->end_xy[0]-mga->beg_xy[0]), (mga->end_xy[1]-mga->beg_xy[1]));
	printf("    GD PS=%.4f  ROT=%.2f deg : TCS RA DEC:  %.1f %.1f\n", 
	   mga->gd_arcsec_pixel, mga->gd_rot_deg, mga->tcs_offset[0], mga->tcs_offset[1]);
	printf(" MORIS PS=%.4f  ROT=%.2f deg :  MORIS ADJ:  %.1f %.1f\n", 
	   mga->moris_arcsec_pixel, mga->moris_rot_deg, mga->moris_adj[0], mga->moris_adj[1]);

   sprintf( buf, "DV Pixels: %.1f,%.1f \n"
	              "TCS Offset: %.1f, %.1f AS\n"
	              "MORIS Adj:  %.1f, %.1f pixels\n",
					  (mga->end_xy[0]-mga->beg_xy[0]), (mga->end_xy[1]-mga->beg_xy[1]),
					  mga->tcs_offset[0], mga->tcs_offset[1], mga->moris_adj[0], mga->moris_adj[1]);
	gtk_label_set_text( GTK_LABEL(mga->cal_label), buf);
}

/*-----------------------------------------------------------------------
** mga_apply_offset_cb() - send offset to moris and TCS 
**-----------------------------------------------------------------------
*/
static void mga_apply_offset_cb( GtkWidget *widget, gpointer data )
{
   struct moris_guide_adj_t *mga = (struct moris_guide_adj_t *)data;

   int rc;
	double tx, ty;
	struct moris_guide_info_t moris_info;  
	char cmd[80];
	char rpy[80];
	char msg[180];

	printf("mga_apply_offset_cb(): data_ok=%d \n", mga->data_ok );
	if( !mga->data_ok )
   {
	   return;
   }

   /*
	** get moris data via guide.query.box
	*/
   rc = moris_guide_query( &moris_info, "moris");
	printf("   moris_guide_query() rc=%d is_guide=%d target=%.1f,%.1f box=%d,%d %dx%d \n", 
      rc, moris_info.is_guiding, moris_info.tx, moris_info.ty,
      moris_info.x, moris_info.y, moris_info.wid, moris_info.hgt);

   /*
	**  offset TCS
	*/
	sprintf( cmd, "USER.INC %2.1f %2.1f ", mga->tcs_offset[0], mga->tcs_offset[1] );
	if( tcs3_com( cmd, rpy, sizeof(rpy), Lc.tcshostname) != ERR_NONE )
	{
	   printf("   mga_apply_offset_cb() ERROR issuing command to TCS.\n");
	}
	printf("   mga_apply_offset_cb() to TCS: cmd=[%s] rpy=[%s]\n", cmd, rpy);

   /*
   ** do Moris Guidebox.Center
	*/
	tx = moris_info.tx + mga->moris_adj[0];
	ty = moris_info.ty + mga->moris_adj[1];

	sprintf( cmd, "GuideBox.Center %.1f %.1f ", tx, ty);
	if( moris_com( cmd, rpy, sizeof(rpy), "moris") != ERR_NONE )
	{
	   printf("   mga_apply_offset_cb() ERROR issuing command to MORIS.\n");
	}
	printf("   mga_apply_offset_cb() to MORIS: cmd=[%s] rpy=[%s]\n", cmd, rpy);

	// set apply label
	sprintf( msg, "Incremented TCS User Offset by %.1f,%.1f AS\n"
	              "Centered Moris's guidebox to %.1f %.1f",
					  mga->tcs_offset[0], mga->tcs_offset[1], tx, ty);
	gtk_label_set_text( GTK_LABEL(mga->apply_label), msg);
}

/*-----------------------------------------------------------------------
** mga_test_cb() - the defaule cancel callback. Just hides the dialog
**   window.
**-----------------------------------------------------------------------
*/
static void mga_test_cb( GtkWidget *widget, gpointer data )
{
   int rc;
	struct moris_guide_info_t moris_info;  

	printf("mga_test_cb(): moris_guide_query() \n" );
   rc = moris_guide_query( &moris_info, "moris");
   printf("   rc=%d is_guide=%d target=%.1f,%.1f  box=%d,%d %dx%d \n", 
      rc, moris_info.is_guiding, moris_info.tx, moris_info.ty,
      moris_info.x, moris_info.y, moris_info.wid, moris_info.hgt);
}

/*-----------------------------------------------------------------------
** mga_peak() - finds a peak pixel in the box.
**-----------------------------------------------------------------------
*/
static int mga_peak ( 
   double *r_cx, double *r_cy,   // return center x, y (offset absolute pixel x y)
	struct df_buf_t *bp,          // data buffer
	struct array_t *box           // defines the box
)
{
   int x, y;
   double cx, cy, d, max;

   cx = box->wid;
   cy = box->hgt;
	max = dfdataxy( bp, box->x, box->y);

	for( y=box->y; y<box->y+box->hgt; y++ )
	{
	   for( x=box->x; x<box->x+box->wid; x++ )
		{
		   d = dfdataxy( bp, x, y);
			if( max < d )
			{
			   cx = x;
			   cy = y;
				max = d;
			}
		}
	}
	*r_cx = cx;
	*r_cy = cy;
	return ERR_NONE;
}

/*-----------------------------------------------------------------------
** mga_centroid() - finds a centroid in the box.
**-----------------------------------------------------------------------
*/
static int mga_centroid ( 
   double *r_cx, double *r_cy,   // return center x, y (absolute x,y )
	struct df_buf_t *bp,          // data buffer
	struct array_t *box           // defines the box
)
{
   int x, y, i, n;
   double cx, cy;

	double *data;

   // allocate and copy data 
	n = box->wid*box->hgt;
	if( NULL == ( data = ( double *)malloc( sizeof(double)*n)) )
	   return ERR_MEM_ALLOC;
   i = 0;
	for( y=box->y; y<box->y+box->hgt; y++ )
	   for( x=box->x; x<box->x+box->wid; x++ )
		   data[i++] = dfdataxy( bp, x, y);

	mga_cal_centroid( &cx, &cy, data, box->wid, box->hgt);

	*r_cx = box->x + cx;
	*r_cy = box->y + cy;
	free( data );
	return ERR_NONE;
}

/*-----------------------------------------------------------------------
** mga_bright() - finds a filter1+centroid in the box.
**-----------------------------------------------------------------------
*/
static int mga_bright ( 
   double *r_cx, double *r_cy,   // return center x, y (absolute x,y )
	struct df_buf_t *bp,          // data buffer
	struct array_t *box           // defines the box
)
{
   int x, y, i, n;
   double cx, cy;

	double *data;

   // allocate and copy data 
	n = box->wid*box->hgt;
	if( NULL == ( data = ( double *)malloc( sizeof(double)*n)) )
	   return ERR_MEM_ALLOC;
   i = 0;
	for( y=box->y; y<box->y+box->hgt; y++ )
	   for( x=box->x; x<box->x+box->wid; x++ )
		   data[i++] = dfdataxy( bp, x, y);

	mga_do_filter1( data, box->wid, box->hgt);
	mga_cal_centroid( &cx, &cy, data, box->wid, box->hgt);

	*r_cx = box->x + cx;
	*r_cy = box->y + cy;
	free( data );
	return ERR_NONE;
}

/*-----------------------------------------------------------------------
** mga_faint() - finds a filter2+centroid in the box.
**-----------------------------------------------------------------------
*/
static int mga_faint ( 
   double *r_cx, double *r_cy,   // return center x, y (absolute x,y )
	struct df_buf_t *bp,          // data buffer
	struct array_t *box           // defines the box
)
{
   int x, y, i, n;
   double cx, cy;

	double *data;

   // allocate and copy data 
	n = box->wid*box->hgt;
	if( NULL == ( data = ( double *)malloc( sizeof(double)*n)) )
	   return ERR_MEM_ALLOC;
   i = 0;
	for( y=box->y; y<box->y+box->hgt; y++ )
	   for( x=box->x; x<box->x+box->wid; x++ )
		   data[i++] = dfdataxy( bp, x, y);

	mga_do_filter2( data, box->wid, box->hgt);
	mga_cal_centroid( &cx, &cy, data, box->wid, box->hgt);

	*r_cx = box->x + cx;
	*r_cy = box->y + cy;
	free( data );
	return ERR_NONE;
}

/*-----------------------------------------------------------------------
** mga_very_faint() - finds a smooth2+filter2+centroid in the box.
**-----------------------------------------------------------------------
*/
static int mga_very_faint ( 
   double *r_cx, double *r_cy,   // return center x, y (absolute x,y )
	struct df_buf_t *bp,          // data buffer
	struct array_t *box           // defines the box
)
{
   int x, y, i, n, rc;
   double cx, cy;

	double *data;

   // allocate and copy data 
	n = box->wid*box->hgt;
	if( NULL == ( data = ( double *)malloc( sizeof(double)*n)) )
	   return ERR_MEM_ALLOC;
   i = 0;
	for( y=box->y; y<box->y+box->hgt; y++ )
	   for( x=box->x; x<box->x+box->wid; x++ )
		   data[i++] = dfdataxy( bp, x, y);

	rc = mga_do_smooth2( data, box->wid, box->hgt);
	mga_do_filter2( data, box->wid, box->hgt);
	mga_cal_centroid( &cx, &cy, data, box->wid, box->hgt);

	*r_cx = box->x + cx;
	*r_cy = box->y + cy;
	free( data );
	return rc;
}

/*------------------------------------------------------------
** mga_cal_centroid() - caculates a centroid value on an array.
**------------------------------------------------------------
*/
static void mga_cal_centroid ( 
   double *r_cx, double *r_cy,   // return centroid
	double *data,                 // points to data
	int    wid,                   // data W x H
	int    hgt 
)
{
   int x, y, i, n;
	double cx, cy, d;
	double min, max, mean,
			 sum_dx, sum_dy, sum_d;

   n = wid*hgt;
	d_stats_p1( &min, &max, &mean, data, n);

	sum_dx =  sum_dy =  sum_d = 0;
   i = 0;
	for( y=0; y<+hgt; y++)
	{
	   for( x=0; x<wid; x++ )
		{
		   d = min - data[i++];
			sum_d += d;
			sum_dx += d * (x+1);
			sum_dy += d * (y+1);
		}
	}

	cx = wid/2.0;
	cy = hgt/2.0;

	if( sum_d != 0 )
	{
	   cx = ( sum_dx / sum_d ) - 1.0;
	   cy = ( sum_dy / sum_d ) - 1.0;
	}

	*r_cx = cx;
	*r_cy = cy;
}

/*------------------------------------------------------------
** mga_do_filter1() - 
**    a. rescale so that old [mean-std, mean+std] maps to -25 to +25
**    b. reset negative values to 0.
**------------------------------------------------------------
*/
static void mga_do_filter1 ( 
	double *data,                 // points to data
	int    wid,                   // data W x H
	int    hgt 
)
{
   int i, n;
	double d;
	double min, max, mean, std,
			 low, high;

   n = wid*hgt;
	d_stats_p1( &min, &max, &mean, data, n);
	d_stats_p2( &std, data, n, mean);

   /* adjust so the medium is near zero,  */
	if( std < 0.5 ) std = 0.5; // so we have some reasonable range for low & high.
	low  = mean - std;
	high = mean + std;

	for( i=0; i<n; i++ )
	{
	   d = data[i];
		d = map( d, low, high, -25, 25);
		if( d < 0 ) d = 0;

	   data[i] = d;
	}
}

/*------------------------------------------------------------
** mga_do_filter2() - 
**    a. reset image so mean equals 0.
**    b. divide data by std
**    c. then zero value that are <= 1.
**------------------------------------------------------------
*/
static void mga_do_filter2 ( 
	double *data,                 // points to data
	int    wid,                   // data W x H
	int    hgt 
)
{
   int    i, n;
	double d;
	double min, max, mean, std;

   n = wid*hgt;
	d_stats_p1( &min, &max, &mean, data, n);
	d_stats_p2( &std, data, n, mean);

   /* adjust so the medium is near zero,  */
	if( std < 0.5 ) std = 0.5; // so we have some reasonable range for low & high.

	for( i=0; i<n; i++ )
	{
	   d = data[i];

		d = ( d - mean ) / std;
		if( d < 1 ) d = 0;

	   data[i] = d;
	}
}

/*------------------------------------------------------------
** mga_do_smooth() - 
**    a. reset image so mean equals 0.
**    b. divide data by std
**    c. then zero value that are <= 1.
**------------------------------------------------------------
*/
static int mga_do_smooth ( 
	double *data,                 // IO: points to data to smooth
	int    wid,                   //  I: data W x H
	int    hgt 
)
{
   int    n, x, y, xi, yi;
	double sum;
	double *tempbuf;

   // for smooth we need a copy of the data
   n = wid*hgt;
	if( NULL == ( tempbuf = ( double *)malloc( sizeof(double)*n)) )
	   return ERR_MEM_ALLOC;
   memcpy( tempbuf, data, n*sizeof(double));


   /* smooth data by averaging data & surrounding pixels */
	for( y=0; y<+hgt; y++)
	{
	   for( x=0; x<wid; x++ )
		{
		   sum = 0;
			n = 0;

			for( yi=y-1; yi<=y+1; yi++ )
			{
				for( xi=x-1; xi<=x+1; xi++ )
				{
				   if( INRANGE(0, yi, hgt-1) && INRANGE(0, xi, wid-1))
					{
					   sum += tempbuf[yi*wid+xi];
						n++;
					}
				}
			}
			data[y*wid + x] = sum/n;
		}
	}

   free( tempbuf );
	return ERR_NONE;
}

/*------------------------------------------------------------
** mga_do_smooth2() - 
**    a. Look at pixel & all neighbors (3x3)
**    b. Remove data that is medium+1_std (a hot pixel).
**    c. Replace pixel with mean.
**------------------------------------------------------------
*/
static int mga_do_smooth2 ( 
	double *data,                 // IO: points to data to smooth
	int    wid,                   //  I: data W x H
	int    hgt 
)
{
   int x, y,
       xi, yi,
       i, m, n;
   double *tempbuf;
   double d[9];
   double min, max, mean, std, median;


   /* allocate working buffer & copy original data into tempbuf */
   n = wid * hgt * sizeof(double);  /* number of bytes */
   if( (tempbuf = malloc( n ) ) == NULL )
      return ERR_MEM_ALLOC;
   memcpy( tempbuf, data, n );

   /* smooth data by averaging data & surrounding pixels */
   for( y=0; y < hgt; y++ )
   {
      for( x=0; x < wid; x++ )
      {
         n = 0;
         for( yi=y-1; yi<=y+1; yi++ )
         {
            for( xi=x-1; xi<=x+1; xi++ )
            {
               if( INRANGE(0, yi, hgt-1) && INRANGE(0, xi, wid-1))
              {
                  d[n] = tempbuf[yi*wid + xi];
                  n++;
               }
            }
         }

         // sort and get median of 3x3 array
         qsort( d, n, sizeof(double), qsort_comp_double);
         if( isodd(n) )
            median = d[(n/2)+1];
         else
            median = (d[n/2] + d[(n/2)+1]) / 2.0;

         // stats on d.
         d_stats_p1( &min, &max, &mean, d, n );
         d_stats_p2( &std, d, n, mean );

         // remove hot pixel & recalculate mean.
         m = n;
         for(i=0; i<m; i++ )
         {
            if( d[i] > median+(0.8*std) )
            {
               n--;
            }
         }
         d_stats_p1( &min, &max, &mean, d, n );

         // replace pixel data with mean.
         data[y*wid + x] = mean;
      }
   }

   free( tempbuf );
   return ERR_NONE;
}

static int qsort_comp_double( const void *a, const void *b)
{
   return ( *(double*)a > *(double*)b) - ( *(double*)a < *(double*)b);
}


/*----------------------------------------------------------------------------
**  moris_guide_query_box() - rturn info about moris's guidebox
**----------------------------------------------------------------------------
*/
int moris_guide_query(
	struct moris_guide_info_t *moris_info,   /* O: information returned by query */
	char         *moris_host                 /* I: hostname  */
)
{
   int rc;
   char  buf[80];
   char  tbuf[40];
   char * str_ptr;

	if( (rc = moris_com( " guide.query.box ", buf, sizeof(buf), moris_host )) != ERR_NONE )
      return rc;

   /* buf should start with with 'OK' */
   if( parseString_r( tbuf, sizeof(tbuf), buf, " ", &str_ptr ) != ERR_NONE )
   {
     return ERR_INV_FORMAT;
   }
   if( stricmp ( tbuf, "OK" ) != ERR_NONE )
   {
     return  ERR_INV_FORMAT;
   }

   // is_guiding
	rc = parseInt_r( &moris_info->is_guiding, NULL, " ", &str_ptr);
   if( rc != ERR_NONE)
      return rc;

   // target XY 
	rc = parseFloat_r( &moris_info->tx, NULL, " ", &str_ptr );
   if( rc != ERR_NONE)
      return rc;

	rc = parseFloat_r( &moris_info->ty, NULL, " ", &str_ptr );
	if( rc != ERR_NONE)
			return rc;

   // box X Y wid hgt
	rc = parseInt_r( &moris_info->x, NULL, " ", &str_ptr);
   if( rc != ERR_NONE)
      return rc;

	rc = parseInt_r( &moris_info->y, NULL, " ", &str_ptr);
   if( rc != ERR_NONE)
      return rc;

	rc = parseInt_r( &moris_info->wid, NULL, " ", &str_ptr);
   if( rc != ERR_NONE)
      return rc;

	rc = parseInt_r( &moris_info->hgt, NULL, " ", &str_ptr);
   if( rc != ERR_NONE)
      return rc;

   return ERR_NONE;
}



/********************************************************************************/

