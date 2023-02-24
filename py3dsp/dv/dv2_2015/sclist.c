/***************************************************************************
**  sclist.c - Single Column List, uses a TreeView + ListStore & Friends 
**     to provide a single column list widget for the application.
**
** Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php
** Copyright (C) 2012 <NASA IRTF. http://irtfweb.ifa.hawaii.edu>
***************************************************************************
*/

#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdio.h>

#include "sclist.h"

/*-------------------------------------------
**  sclist_view_new() - creates a new sclist
**-------------------------------------------
*/
struct sclist_view_t *sclist_view_new(const char *title)
{
   struct sclist_view_t *pLV = NULL;

   if( (pLV = (struct sclist_view_t*)malloc(sizeof(struct sclist_view_t))) == NULL )
      return NULL;
   
   pLV->s_window_w   = gtk_scrolled_window_new(NULL, NULL);
   
   pLV->list_store_w  = gtk_list_store_new(1, G_TYPE_STRING);       /* Model */
   pLV->tree_view_w = gtk_tree_view_new_with_model(GTK_TREE_MODEL(pLV->list_store_w));  /* Tree */
   pLV->renderer_text_w = gtk_cell_renderer_text_new();             /* Text renderer */
   pLV->tree_view_column_w = gtk_tree_view_column_new_with_attributes(title,
         pLV->renderer_text_w, "text", 0, NULL);

   gtk_tree_view_append_column(pLV->tree_view_w, pLV->tree_view_column_w);  /* Add the column */
   gtk_tree_view_set_headers_visible(pLV->tree_view_w, TRUE);       /* Show the header */

   //g_signal_connect(GTK_WIDGET(pLV->tree_view_w), "cursor-changed", G_CALLBACK( update_additional_info_cb ), NULL);
   
   gtk_container_add(GTK_CONTAINER(pLV->s_window_w), GTK_WIDGET(pLV->tree_view_w));
   gtk_widget_show_all(pLV->s_window_w);
   
   return pLV;
}

/*-----------------------------------------------------------------------
**  sclist_view_set_content() - replaces the contents of a sc list.
**-----------------------------------------------------------------------
*/
int sclist_view_set_content(struct sclist_view_t *pLV, struct sclist_view_item_t *pItems, int count)
{
   int i = 0;

   sclist_view_clear(pLV->tree_view_w, pLV->list_store_w);
   
   gtk_widget_freeze_child_notify(GTK_WIDGET(pLV->tree_view_w));      /* Freeze events */      

   for( i = 0; i < count; i++ )               /* Insert each item */
      sclist_view_append(pLV->list_store_w, &pItems[i]);

   gtk_widget_thaw_child_notify(GTK_WIDGET(pLV->tree_view_w));      /* Re-enable events */
   
   return i;
}

/*-----------------------------------------------------------------------
**  sclist_view_clear() - clears items in the sclist
**-----------------------------------------------------------------------
*/
void sclist_view_clear(GtkTreeView *list_view, GtkListStore *list_store)
{
   gtk_widget_freeze_child_notify(GTK_WIDGET(list_view));      /* Freeze, clear, thaw */
      gtk_list_store_clear(list_store);
   gtk_widget_thaw_child_notify(GTK_WIDGET(list_view));
}

/*-----------------------------------------------------------------------
**  sclist_view_append() - appends a single item in the list
**-----------------------------------------------------------------------
*/
int sclist_view_append(GtkListStore *list_store, struct sclist_view_item_t *pItem)
{
   GtkTreeIter iter;

   gtk_list_store_append(list_store, &iter);
   gtk_list_store_set(list_store, &iter,
         0, pItem->text,
         -1);
   
   return 0;
}
