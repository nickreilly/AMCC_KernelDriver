/***************************************************************************
**  sclist.h - header file for sclist
** Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php
** Copyright (C) 2012 <NASA IRTF. http://irtfweb.ifa.hawaii.edu>
***************************************************************************
*/

#ifndef __SCLIST_H_INCLUDED
#define __SCLIST_H_INCLUDED

#define SC_MAX_LIST_VIEW_ITEM_LENGTH 512

//------------------------------------------------------
// List view widget encapsulation, with scrolling
struct sclist_view_t
{
   GtkTreeView       * tree_view_w;        /* The list widget */
   GtkListStore      * list_store_w;       /* The storage parameters */
   GtkCellRenderer   * renderer_text_w;    /* Text renderer */
   GtkTreeViewColumn * tree_view_column_w; /* The single list column */
   GtkWidget         * s_window_w;         /* The scrolling window container for the TreeView */
	 
};

//------------------------------------------------------
// A single list view item
struct sclist_view_item_t
{
   char   text[SC_MAX_LIST_VIEW_ITEM_LENGTH];   /* Item text */
};

struct sclist_view_t *sclist_view_new(const char *title);
int sclist_view_set_content(struct sclist_view_t *pLV,
   struct sclist_view_item_t *pItems, int count);
void sclist_view_clear( GtkTreeView *list_view, GtkListStore *list_store);
int sclist_view_append( GtkListStore *list_store, struct sclist_view_item_t *pItem);
void update_additional_info_cb(GtkWidget *widget, gpointer data);

#endif
