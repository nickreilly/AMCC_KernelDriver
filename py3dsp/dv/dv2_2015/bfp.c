/********************************************************************************/
/*  DIALOG.C - contains bfp (Browse for Path)                                   */
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

/*************************************************************************
**  BFP (Browse for Path) function - A dialog window for seleting a
** directory name by navigating the mouse on an option_menu/list.
**
** To use allocate a struct browseForPath_t structure for housekeeping.
**
** User function are:
**
**    bfp_create_dialog() - create dialog box and initialize bfp structure.
**
**    bfp_hide_dialog() - Hide the dialog window.
**
**    bfp_show_dialog() - Shows the dialog window.
**
**    bfp_set_path() - Sets the current path for the dialog window.
**
**    bfp_set_ok_cb() - Install you callback for the OK button. See bfp_ok_cb() for details.
**
** static functions are:
**    int bfp_delete_event( ) - handles delete window events.
**    void bfp_ok_cb( ) - default OK button callback.
**    void bfp_cancel_cb( ) - default cancel button callback.
**    void bfp_refresh_cb( ) - default refresh button callback.
**    void bfp_cbox_cb( ) - cbox selection callback.
**    void bfp_clist_cb( ) -  clist selection callback.
**
************************************************************************
*/

/* prototype static functions */

static int bfp_delete_event( GtkWidget *widget, GdkEvent  *event, gpointer data );
static void bfp_ok_cb( GtkWidget *widget, gpointer data );
static void bfp_cancel_cb( GtkWidget *widget, gpointer data );
static void bfp_refresh_cb( GtkWidget *widget, gpointer data );
static void bfp_cbox_cb( GtkWidget *widget, gpointer data );
static void bfp_sclist_cb( GtkTreeView *tree_view, GtkTreePath *tree_path, 
                           GtkTreeViewColumn *column, gpointer data );

/*-----------------------------------------------------------------------
** bfp_create_dialog() - Browse For Path create dialog.
**   You must pass it a browseForPath_t structure and
**   callbackup function for the option menu selections.
**-----------------------------------------------------------------------
*/
int bfp_create_dialog ( struct browseForPath_t *bfp, char * title )
{
   GtkWidget * table;
   GtkWidget * button;
   GtkWidget * label;
   GtkWidget * widget;

   char * menu_stub[1];

   /* initalize structure */
   bfp->dialog_window = NULL;
   bfp->path_cbox_w   = NULL;
   bfp->entry         = NULL;
   bfp->ok_button     = NULL;
   bfp->sclist        = NULL;
   dirlist_init( &bfp->dirlist );

   /*---------------------------------
   ** Create bfp->dialog_window
   */
   bfp->dialog_window = gtk_window_new( GTK_WINDOW_TOPLEVEL );
   gtk_window_set_title( GTK_WINDOW (bfp->dialog_window), title );
   gtk_window_set_resizable( GTK_WINDOW(bfp->dialog_window), TRUE );

   g_signal_connect( G_OBJECT( bfp->dialog_window), "delete_event", G_CALLBACK(bfp_delete_event), NULL );
	g_signal_connect( G_OBJECT( bfp->dialog_window), "destroy", G_CALLBACK(bfp_delete_event), NULL );

   /* table to contain my widgets */
   table = gtk_table_new( 100, 100, TRUE );
	gtk_widget_set_size_request( GTK_WIDGET(table), 300, 300);
   gtk_container_add (GTK_CONTAINER (bfp->dialog_window), table);

   /*--------------------------------------------
   **  Path Label & bfp->path_option_w (Option Menu)
   */
   menu_stub[0] = NULL;
	bfp->path_cbox_w = MyComboBoxCreate( menu_stub );
   gtk_table_attach_defaults( GTK_TABLE(table), bfp->path_cbox_w, 10, 90, 0, 10 );
	bfp->path_cbox_hid = g_signal_connect( G_OBJECT(bfp->path_cbox_w), "changed", 
	   G_CALLBACK(bfp_cbox_cb), (gpointer) bfp );

   gtk_widget_show( bfp->path_cbox_w );

   /*----------------------------------------------------------
   **  sclist to show directories (and allow user to select them).
   */
   bfp->sclist = sclist_view_new( "Directories" );
   gtk_table_attach_defaults(GTK_TABLE(table), bfp->sclist->s_window_w, 5, 95, 10, 78);
   g_signal_connect( G_OBJECT(bfp->sclist->tree_view_w), "row-activated", 
      G_CALLBACK (bfp_sclist_cb), (gpointer) bfp );

   /*---------------------------------------------------------
   **  label to let the used know what the current path is:
   */
   label = gtk_label_new( "Path is: " );
   gtk_misc_set_alignment( GTK_MISC(label), 1, 0.5 );
   gtk_table_attach_defaults( GTK_TABLE(table), label, 0, 20, 78, 88 );
   gtk_widget_show( label );

   bfp->entry = gtk_entry_new( );
   gtk_editable_set_editable( GTK_EDITABLE(bfp->entry), FALSE);
   gtk_table_attach_defaults( GTK_TABLE(table), bfp->entry, 20, 90, 78, 88 );
   gtk_widget_show( bfp->entry );

   /*---------------------------------
   **  hseparator
   */
   widget = gtk_hseparator_new();
   gtk_table_attach_defaults( GTK_TABLE(table), widget, 0, 100, 88, 90 );
   gtk_widget_show( widget );

   /*---------------------------------
   **  OK button
   */
   bfp->ok_button = gtk_button_new_with_label( "OK");
   gtk_table_attach_defaults( GTK_TABLE(table), bfp->ok_button, 5, 45, 90, 100);
   //g_signal_connect( G_OBJECT (bfp->ok_button), "clicked", G_CALLBACK(bfp_ok_cb), (gpointer)bfp );
   gtk_widget_show(bfp->ok_button);

   /*---------------------------------
   **  Cancel button to hide dialog
   */
   button = gtk_button_new_with_label( "Cancel");
   gtk_table_attach_defaults( GTK_TABLE(table), button, 45, 70, 90, 100);
   g_signal_connect( G_OBJECT (button), "clicked", G_CALLBACK(bfp_cancel_cb ), (gpointer) bfp );
   gtk_widget_show(button);

   /*---------------------------------
   **  Refresh button
   */
   button = gtk_button_new_with_label( "Refresh");
   gtk_table_attach_defaults( GTK_TABLE(table), button, 75, 95, 90, 100);
   g_signal_connect( G_OBJECT (button), "clicked", G_CALLBACK (bfp_refresh_cb ), (gpointer) bfp );
   gtk_widget_show(button);

   gtk_widget_show( table );

   return 0;
}

/*-----------------------------------------------------------------------
** bfp_show_dialog() - show the window.
**-----------------------------------------------------------------------
*/
void bfp_show_dialog ( struct browseForPath_t *bfp )
{
   gtk_widget_show( bfp->dialog_window );
   gdk_window_raise( bfp->dialog_window->window );
}

/*-----------------------------------------------------------------------
** bfp_hide_dialog() - hides the bfp window.
**-----------------------------------------------------------------------
*/
void bfp_hide_dialog ( struct browseForPath_t *bfp )
{
   gtk_widget_hide( bfp->dialog_window );
}

/*-----------------------------------------------------------------------
** bfp_set_path() - Set a new path.
**-----------------------------------------------------------------------
*/
void bfp_set_path( struct browseForPath_t *bfp, char * path )
{
   int i;
   int nele_to_delete;

   /*-------------------------------------
   ** Make diretory list
   */
   nele_to_delete = bfp->dirlist.nele; 
   dirlist_newpath( &bfp->dirlist, path );

   /*----------------------------------------------
   ** update the combo box menu
   */
	g_signal_handler_block( GTK_OBJECT(bfp->path_cbox_w), bfp->path_cbox_hid );

   // remove old list
   for( i=0; i<nele_to_delete; i++ )
      gtk_combo_box_remove_text( GTK_COMBO_BOX(bfp->path_cbox_w) , 0);

   // add new list
   for( i=0; i < bfp->dirlist.nele; i++ )
		gtk_combo_box_append_text( GTK_COMBO_BOX(bfp->path_cbox_w) , bfp->dirlist.list[i]);
   gtk_combo_box_set_active(  GTK_COMBO_BOX(bfp->path_cbox_w), 0);

	g_signal_handler_unblock( GTK_OBJECT(bfp->path_cbox_w), bfp->path_cbox_hid );

   /*----------------------------------------------
   ** update the directory text view 
   */
   update_file_list( TRUE, bfp->dirlist.path, "*", bfp->sclist->list_store_w,
		bfp->sclist->tree_view_w );

   /*----------------------------------------------
   ** update the label
   */
   gtk_entry_set_text( GTK_ENTRY(bfp->entry), bfp->dirlist.path );
}

/*-----------------------------------------------------------------------
**  bfp_set_ok_cb() - allows user to install their own OK button callback.
**-----------------------------------------------------------------------
*/
void bfp_set_ok_cb( struct browseForPath_t *bfp, GCallback func )
{
   g_signal_connect( G_OBJECT (bfp->ok_button), "clicked", func,
      (gpointer) bfp );
}

/***************************************************************************
**  BFP's static (private) functions
***************************************************************************
*/

/*-----------------------------------------------------------------------
**  bfp_delete_event()
**-----------------------------------------------------------------------
*/
int bfp_delete_event( GtkWidget *widget, GdkEvent  *event, gpointer data )
{
   /* Return FALSE - gtk will call base_destroy_cb();
   **         TRUE - gtk will ignore delete event.
   */
   gtk_widget_hide( GTK_WIDGET(widget) );
   return(TRUE);
}

/*-----------------------------------------------------------------------
** bfp_ok_cb() - the defaule ok button callback.
**-----------------------------------------------------------------------
*/
void bfp_ok_cb( GtkWidget *widget, gpointer data )
{
   struct browseForPath_t *bfp = (struct browseForPath_t *)data;

   printf("bfp_ok_cb() the path is %s\n", bfp->dirlist.path );
   bfp_hide_dialog ( bfp );
}

/*-----------------------------------------------------------------------
** bfp_cancel_cb() - the defaule cancel callback. Just hides the dialog
**   window.
**-----------------------------------------------------------------------
*/
void bfp_cancel_cb( GtkWidget *widget, gpointer data )
{
   bfp_hide_dialog ( (struct browseForPath_t *)data );
}

/*----------------------------------------------------------------------
** bfp_refresh_cb() - the refresh callback.
**-----------------------------------------------------------------------
*/
void bfp_refresh_cb( GtkWidget *widget, gpointer data )
{
   struct browseForPath_t *bfp = (struct browseForPath_t *)data;

   /* update the directory list */
   update_file_list( TRUE, bfp->dirlist.path, "*", bfp->sclist->list_store_w,
		bfp->sclist->tree_view_w );
}

/*-----------------------------------------------------------------------
** bfp_cbox_cb - CB for the cbox, use selected a new path.
**-----------------------------------------------------------------------
*/
void bfp_cbox_cb( GtkWidget *widget, gpointer data )
{
   int inx;
   struct browseForPath_t *bfp;
   char newpath[DIRLIST_MAX_PATH];

   inx = (int)  gtk_combo_box_get_active( GTK_COMBO_BOX(widget) );
   bfp = (struct browseForPath_t *) data;

   if( INRANGE( 0, inx, bfp->dirlist.nele-1) )
	{
		strxcpy( newpath,  bfp->dirlist.list[inx], sizeof(newpath) );
		bfp_set_path( bfp, newpath );
	}
}

/*-----------------------------------------------------------------------
** bfp_sclist_cb - double click on directory list, make it the new path.
**-----------------------------------------------------------------------
*/
void bfp_sclist_cb( GtkTreeView *tree_view, GtkTreePath *tree_path, GtkTreeViewColumn *column, gpointer data )
{
   /* Get file name from list */
   GtkTreeSelection  *selection;
   GtkTreeModel      *model;
   GtkTreeIter       iter;

   char              newpath[256];
   struct browseForPath_t *bfp;

   bfp = (struct browseForPath_t *) data;

   /* Get the newpath from the list widget if there is a selection */
   model     = GTK_TREE_MODEL(bfp->sclist->list_store_w);
   selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(bfp->sclist->tree_view_w));

   if(gtk_tree_selection_get_selected( selection, &model, &iter))
   {
      gchar *str_data;
      gtk_tree_model_get( model, &iter, 0, &str_data, -1 );

      /* Create whole path of the file */
      cat_pathname( newpath, bfp->dirlist.path, str_data, sizeof(newpath) );
      g_free( str_data );

		printf("bfp_sclist_cb() newpath: %s\n", newpath);
		bfp_set_path( bfp, newpath );
   }
}

/********************************************************************************/
