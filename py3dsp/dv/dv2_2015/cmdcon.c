/*******************************************************************************
**   cmdcon.c 
**
** Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php
** Copyright (C) 2012 <NASA IRTF. http://irtfweb.ifa.hawaii.edu>
********************************************************************************
*/

#define EXTERN extern
#define DEBUG 0

/*-------------------------
**  Standard include files
**-------------------------
*/
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include <mqueue.h>
#include <sys/mman.h>


#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

/*------------------------------
**  Non-standard include files
**------------------------------
*/

#include "dv.h"

#define _CC_MAX_CMDBUF_ENTRIES	50

///////////////////////////////////////////////////////////////////////////////
// Private interface
///////////////////////////////////////////////////////////////////////////////

/* Helper function from gtkentry.c */
static void paste_received (GtkClipboard *clipboard, const gchar *text, gpointer data);

/* Mouse entry notification message handler "motion-notify-event" */
static void console_motion_notify(GtkWidget *widget, GdkEventCrossing *event, gpointer data);

/* Mouse button click event "button-press-event" */
static gboolean entry_button_press(GtkWidget *widget, GdkEventButton *event, gpointer data);

/* Paste message handler "paste-clipboard" */
static void entry_paste_clipboard(GtkEntry *entry, gpointer data);

/* Key press message handler "key-press-event" */
static gboolean console_key_press(GtkWidget *widget, GdkEventKey *event, gpointer data);

///////////////////////////////////////////////////////////////////////////////
// Public implementation implementation
///////////////////////////////////////////////////////////////////////////////

cc_console *cc_create_console(GtkWindow *main_wnd)
{
	cc_console	*cc	= NULL;
	
	if( (cc = (cc_console*)malloc(sizeof(cc_console))) == NULL )
		return NULL;
	
	if( (cc->cmdbuf = cbl_new(_CC_MAX_CMDBUF_ENTRIES)) == NULL )
	{
		free(cc);
		return NULL;
	}

	cc->cb		= NULL;
	cc->data	   = TRUE;
		
	cc->wnd		   = main_wnd;
	cc->swnd	      = gtk_scrolled_window_new(NULL, NULL);
	cc->container	= gtk_table_new(2, 1, FALSE);
	cc->console	   = gtk_text_view_new();
	cc->entry	   = gtk_entry_new();

   /* set the winndow size */
   //gtk_widget_set_size_request(cc->console, 1240, 135);

	/* Limit the text length on the entry box */
	gtk_entry_set_max_length(GTK_ENTRY(cc->entry), CBL_MAX_CMD_LEN);	
	
	/* Disable editing of the console objects */
	gtk_text_view_set_editable(GTK_TEXT_VIEW(cc->console), FALSE);

	/* Give the console some scroll bars */
	gtk_container_add(GTK_CONTAINER(cc->swnd), cc->console);
	
	/* Fill the console controls into the table */
	gtk_table_attach(GTK_TABLE(cc->container), cc->swnd, 0, 1, 0, 1, GTK_FILL|GTK_EXPAND, GTK_FILL|GTK_EXPAND, 0, 0);
	gtk_table_attach(GTK_TABLE(cc->container), cc->entry, 0, 1, 1, 2, GTK_FILL, GTK_FILL, 0, 0);
	
	/* Get focus when the mouse move over */
	g_signal_connect(G_OBJECT(cc->console), "motion-notify-event", G_CALLBACK(console_motion_notify), cc);
	g_signal_connect(G_OBJECT(cc->entry), "motion-notify-event", G_CALLBACK(console_motion_notify), cc);
	
	/* Retrieve key press messages */
	g_signal_connect(G_OBJECT(cc->console), "key-press-event", G_CALLBACK(console_key_press), cc);
	g_signal_connect(G_OBJECT(cc->entry), "key-press-event", G_CALLBACK(console_key_press), cc);
	
	/* Retrieve mouse click events on the entry box */
	g_signal_connect(G_OBJECT(cc->entry), "button-press-event", G_CALLBACK(entry_button_press), cc);
	
	/* Handle the paste operation */
	g_signal_connect_after(G_OBJECT(cc->entry), "paste-clipboard", G_CALLBACK(entry_paste_clipboard), cc);
	
   /* Create tags */
   cc_create_tags(cc);
	return cc;
}


void cc_create_tags(cc_console *cc)
{
   GtkTextBuffer     *pBuf;
   pBuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(cc->console));

   /* Black tag */
   cc->color_tags[CMDCON_BLACK] = gtk_text_buffer_create_tag(pBuf, "Black", "foreground", "black", NULL);

   /* Red tag */
   cc->color_tags[CMDCON_RED] = gtk_text_buffer_create_tag(pBuf, "red", "foreground", "red", NULL);

   /* Green tag */
   cc->color_tags[CMDCON_GREEN] = gtk_text_buffer_create_tag(pBuf, "green", "foreground", "green", NULL);

   /* Blue tag */
   cc->color_tags[CMDCON_BLUE] = gtk_text_buffer_create_tag(pBuf, "blue", "foreground", "blue", NULL);

   /* Gray tag */
   cc->color_tags[CMDCON_GRAY] = gtk_text_buffer_create_tag(pBuf, "gray", "foreground", "gray", NULL);

   /* Cyan tag */
   cc->color_tags[CMDCON_CYAN] = gtk_text_buffer_create_tag(pBuf, "cyan", "foreground", "cyan", NULL);

   /* Magenta tag */
   cc->color_tags[CMDCON_MAGENTA] = gtk_text_buffer_create_tag(pBuf, "magenta", "foreground", "magenta", NULL);

   /* Yellow tag */
   cc->color_tags[CMDCON_YELLOW] = gtk_text_buffer_create_tag(pBuf, "yellow", "foreground", "yellow", NULL);

   /* White tag */
   cc->color_tags[CMDCON_WHITE] = gtk_text_buffer_create_tag(pBuf, "white", "foreground", "white", NULL);
}

void cc_destroy_console(cc_console *cc)
{
	cbl_destroy(cc->cmdbuf);
	free(cc);
}

/* Set the callback routine that will be invoked when the */
/* user enters a new string */
void cc_set_entry_callback(cc_console *cc, cc_cmd_callback cb, int data)
{
	cc->cb		= cb;
	cc->data	   = data;
}

/* cc_printf() - command console's printf function */
void cc_printf( cc_console *cc, int color, const char *fmt, ...)
{
	int n;
	char buf[256];
	va_list argptr;

	GtkTextBuffer * buffer_w = NULL;
	GtkTextIter iter, iter2;
	GtkTextMark * insert_mark;

	va_start( argptr, fmt);
	vsprintf( buf, fmt, argptr);
	va_end( argptr );
	
	buffer_w = gtk_text_view_get_buffer(GTK_TEXT_VIEW(cc->console));

   // keep text buffer to reasonable size
	n = gtk_text_buffer_get_char_count(buffer_w);
	if( n > 10240 )
	{
      gtk_text_buffer_get_iter_at_offset(buffer_w, &iter, 0);
      gtk_text_buffer_get_iter_at_offset(buffer_w, &iter2, 5120);
		gtk_text_buffer_delete(buffer_w, &iter, &iter2);
	}
	
	// insert new text
	n = gtk_text_buffer_get_char_count(buffer_w);
	gtk_text_buffer_get_end_iter( buffer_w, &iter );
	gtk_text_buffer_insert_with_tags(buffer_w, &iter, buf, -1, cc->color_tags[color], NULL);

	// scroll window to the end
	gtk_text_buffer_get_end_iter( buffer_w, &iter );
	gtk_text_buffer_place_cursor( buffer_w, &iter);        // places cursor at end.
   insert_mark = gtk_text_buffer_get_insert( buffer_w );  // Get the insert mark (at the cursor).
	gtk_text_view_scroll_to_mark( GTK_TEXT_VIEW(cc->console), insert_mark, 0.0, TRUE, 0.0, 1.0);
}

gboolean cc_is_command_pending(const cc_console *cc)
{
	return cbl_has_new_command(cc->cmdbuf);
}

gboolean cc_get_last_command(cc_console *cc, char *buf, int n)
{
	if( !cc_is_command_pending(cc) )
		return FALSE;
		
	cbl_get_last_command(cc->cmdbuf, buf, n);

	return TRUE;
}

/* Mimick execution of a command. The use_history parameter is a flag */
/* that should be set to non-zero if the specified command is to be added to */
/* the command history list.  The echo_cmd parameter is a second flag that should */
/* be set to non-zero if the command text should be echoed in the console output box. */
void cc_execute_command(cc_console *cc, const char *cmd, gboolean use_history, gboolean echo_cmd)
{
	/* Copy location for the command string */
	char buf[CBL_MAX_CMD_LEN + 1];
		
	/* Print out the command if the echo flag is set */	
	if( echo_cmd )
		cc_printf(cc, CMDCON_BLACK, "%s\n", cmd);
		
	/* Copy the command string and NULL-terminate */
	strxcpy(buf, cmd, CBL_MAX_CMD_LEN);
	buf[CBL_MAX_CMD_LEN] = 0;

	/* Using history? */
	if( use_history )
	{
		cbl_set_current_command(cc->cmdbuf, cmd);
		cbl_handle_command(cc->cmdbuf, CBL_CMD_COMMIT);
	}

	/* Invoke the user callback */
	if( cc->cb )
		(*cc->cb)(cc, buf, cc->data);
}



///////////////////////////////////////////////////////////////////////////////
// Private implementation
///////////////////////////////////////////////////////////////////////////////

/* Internal functions */
/* Code from gtkentry.c from the coders of GTK 2.0 */
static void paste_received (GtkClipboard *clipboard, const gchar *text, gpointer data)
{
  GtkEntry *entry = GTK_ENTRY (data);
  GtkEditable *editable = GTK_EDITABLE (entry);

  if (text)
    {
      gint pos, start, end;
      
      if (gtk_editable_get_selection_bounds (editable, &start, &end))
        gtk_editable_delete_text (editable, start, end);

      pos = entry->current_pos;
		
      gtk_editable_insert_text (editable, text, -1, &pos);
      gtk_editable_set_position (editable, pos);
    }

  g_object_unref (G_OBJECT (entry));
}


void console_motion_notify(GtkWidget *widget, GdkEventCrossing *event, gpointer data)
{
	cc_console *cc = (cc_console*)data;
	
	/* Checks to see if the entry widget has focus */
   #if (GTK_MAJOR_VERSION == 2) && (GTK_MINOR_VERSION < 20 )
	if(!GTK_WIDGET_HAS_FOCUS(cc->entry))
	#else
	if(!gtk_widget_has_focus(cc->entry))
	#endif
	{
		/* If the entry widget doesn't have focus then grab the focus and set the cursor position */
		gtk_widget_grab_focus(cc->entry);
		gtk_editable_set_position(GTK_EDITABLE(cc->entry), cbl_get_entry_cursor(cc->cmdbuf));
	}
}

/* Mouse button click event "button-press-event" */
gboolean entry_button_press(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	cc_console	*cc	= (cc_console*)data;
	GtkEntry 	*entry = GTK_ENTRY(widget);
	int			pos;
	int			tmp_pos;
	char			*pChar;
	char			*pManip;
   const char  *clip_buf;
	const char	*buf;
   char        current_text[256];
   
	char			entry_text[CBL_MAX_CMD_LEN + 1];
	char 			entry_data[CBL_MAX_CMD_LEN + 1];
	GtkTextBuffer *pBuf;
	GtkTextIter	iter;
	int			n;

	/* If the middle button is pressed then process it otherwise let the default handler do it */
	if (event->button == 2 && event->type == GDK_BUTTON_PRESS && entry->editable)
  	{
		/* Used to check if something was pasted*/
		tmp_pos = gtk_editable_get_position(GTK_EDITABLE(widget));

      clip_buf = gtk_clipboard_wait_for_text(gtk_clipboard_get(GDK_SELECTION_PRIMARY));
      if(clip_buf == NULL)
         return TRUE;
  		g_object_ref (G_OBJECT (entry));
      paste_received(gtk_clipboard_get(GDK_SELECTION_PRIMARY), clip_buf, entry);

		/* Code to update the command buffer */
      printf("Before attempt to get text\n");
      cbl_get_current_command(cc->cmdbuf, current_text, 256); 
      strcat(current_text, clip_buf);
      buf = current_text;
		cbl_set_current_command(cc->cmdbuf, buf);
		printf("Paste result: [%s]\n", current_text);

      /* TODO: Find a way to deallocate buf */
//      g_free(buf);
		
		/* Uses the previous position to determine if a paste occured */
		pos = gtk_editable_get_position(GTK_EDITABLE(widget));
		if(tmp_pos != pos)
		{
			/* Make a copy for manipulation */
			strcpy(entry_data, buf);

			/* Setup the manipulation pointers */
			pChar = strchr(entry_data, '\n');
			pManip = entry_data;
			
			/* As long as there is a '\n' do this */
			while( pChar )
			{
				/* Manipulate text */
				*pChar = '\0';
				printf("manip=[%s]\n", pManip);

				/* Send the command to the command buffer */
				cbl_set_current_command(cc->cmdbuf, pManip);
				
				{
					/* Send off the key press */
					cbl_handle_command(cc->cmdbuf, CBL_CMD_COMMIT);
		
					/* Retrieve the last command */
					cbl_get_last_command(cc->cmdbuf, entry_text, sizeof(entry_text));
		
					/* Make sure we place each command on a new line */
					strcat(entry_text, "\n");
		
					/* Insert the new text into the text box */
					cc_printf(cc, CMDCON_BLACK, entry_text);
		
					/* Clear the command entry box */
					gtk_entry_set_text(entry, "");
		
					if( cc->cb )
					{
						entry_text[strlen(entry_text) - 1] = 0;
						(*cc->cb)(cc, entry_text, cc->data);
					}
				}

				/* Update pointers */
				pChar++;
				pManip = pChar;
				pChar = strchr(pChar, '\n');
			}

			/* TODO set focus and cursor*/
			pBuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(cc->console));
			n = gtk_text_buffer_get_char_count(pBuf);
			gtk_text_buffer_get_iter_at_offset(pBuf, &iter, n);
			gtk_text_buffer_place_cursor(pBuf, &iter);
			gtk_window_set_focus(GTK_WINDOW(cc->wnd), cc->console);
			
			if(pManip)
			{
				/* Put the current command in the entry and set cursor*/
				gtk_entry_set_text(entry, pManip);
				cbl_set_current_command(cc->cmdbuf, pManip);
				gtk_widget_grab_focus(GTK_WIDGET(entry));
				gtk_editable_set_position(GTK_EDITABLE(entry), cbl_get_entry_cursor(cc->cmdbuf));
			}
			
		}
      printf("end of middle button paste\n");
		/* Stop other handlers from being called */		
		return TRUE;
	}
	
	/* Retrieve the cursor position from the entry box */
	pos = gtk_editable_get_position(GTK_EDITABLE(widget));
	
	/* Synchronize the command buffer's cursor with the entry box */
	cbl_set_entry_cursor(cc->cmdbuf, pos);
	
	return FALSE;
}

/* Paste message handler "paste-clipboard" */
void entry_paste_clipboard(GtkEntry *entry, gpointer data)
{
	cc_console*	cc	= (cc_console*)data;
	const char *	txt	= NULL;
	int		pos;
   printf("is this called?\n");	
	/* Retrieve the new text and the new cursor position */
	txt = gtk_entry_get_text(GTK_ENTRY(cc->entry));
	pos = gtk_editable_get_position(GTK_EDITABLE(cc->entry));
	
	/* Synchronize the command buffer with the new text */
	cbl_set_current_command(cc->cmdbuf, txt);
	cbl_set_entry_cursor(cc->cmdbuf, pos);
	
}

gboolean console_key_press(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	cc_console	*cc	= (cc_console*)data;
	int		keyval	= event->keyval;
	char 		buf[CBL_MAX_CMD_LEN + 1];

	/*  We should move focus to the entry box so the cursor is visible */
   #if (GTK_MAJOR_VERSION == 2) && (GTK_MINOR_VERSION < 20 )
	if(!GTK_WIDGET_HAS_FOCUS(cc->entry))
	#else
	if(!gtk_widget_has_focus(cc->entry))
	#endif
	{
		gtk_window_set_focus(GTK_WINDOW(cc->wnd), cc->entry);
		gtk_editable_set_position(GTK_EDITABLE(cc->entry), cbl_get_entry_cursor(cc->cmdbuf));
	}

	if( keyval == GDK_Return || keyval == GDK_KP_Enter )
	{
		/* Send off the key press */
		cbl_handle_command(cc->cmdbuf, CBL_CMD_COMMIT);
		
		/* Retrieve the last command */
		cbl_get_last_command(cc->cmdbuf, buf, sizeof(buf));
		
		/* Make sure we place each command on a new line */
		strcat(buf, "\n");
		
		/* Insert the new text into the text box */
		//cc_printf(cc, CMDCON_GREEN, buf);
		
		/* Clear the command entry box */
		gtk_entry_set_text(GTK_ENTRY(cc->entry), "");
		
		if( cc->cb )
		{
			/* Theoretically should set the focus to the console thus scrolling it */
			gtk_window_set_focus(GTK_WINDOW(cc->wnd), cc->console);
			buf[strlen(buf) - 1] = 0;
			(*cc->cb)(cc, buf, cc->data);
		}
		
	}
	else if( keyval == GDK_BackSpace )
	{
		/* Handle the key */
		cbl_handle_command(cc->cmdbuf, CBL_CMD_BACKSPACE);
		
		/* Retrieve the current command */
		cbl_get_current_command(cc->cmdbuf, buf, sizeof(buf));
		
		gtk_entry_set_text(GTK_ENTRY(cc->entry), buf);
		gtk_editable_set_position(GTK_EDITABLE(cc->entry), cbl_get_entry_cursor(cc->cmdbuf));
	}
	else if( keyval == GDK_Delete )
	{
		/* Handle the key */
		cbl_handle_command(cc->cmdbuf, CBL_CMD_DELETE);
		
		/* Retrieve the current command */
		cbl_get_current_command(cc->cmdbuf, buf, sizeof(buf));
		
		gtk_entry_set_text(GTK_ENTRY(cc->entry), buf);
		gtk_editable_set_position(GTK_EDITABLE(cc->entry), cbl_get_entry_cursor(cc->cmdbuf));
	}
	else if( keyval == GDK_Right )
	{
		cbl_handle_command(cc->cmdbuf, CBL_CMD_RIGHT);

		/* Retrieve the current command and move the cursor to the left */
		cbl_get_current_command(cc->cmdbuf, buf, sizeof(buf));
		gtk_editable_set_position(GTK_EDITABLE(cc->entry), cbl_get_entry_cursor(cc->cmdbuf));
	}
	else if( keyval == GDK_Left )
	{
		cbl_handle_command(cc->cmdbuf, CBL_CMD_LEFT);
		
		/* Retrieve the current command and move the cursor to the left */
		cbl_get_current_command(cc->cmdbuf, buf, sizeof(buf));
		gtk_editable_set_position(GTK_EDITABLE(cc->entry), cbl_get_entry_cursor(cc->cmdbuf));
	}
	else if( keyval == GDK_Up )
	{
		cbl_handle_command(cc->cmdbuf, CBL_CMD_UP);
		
		/* Retrieve the current command */
		cbl_get_current_command(cc->cmdbuf, buf, sizeof(buf));
		
		/* Set the new text and position the cursor */
		gtk_entry_set_text(GTK_ENTRY(cc->entry), buf);
		gtk_editable_set_position(GTK_EDITABLE(cc->entry), cbl_get_entry_cursor(cc->cmdbuf));
	}
	
	else if( keyval == GDK_Down )
	{
		cbl_handle_command(cc->cmdbuf, CBL_CMD_DOWN);
		
		/* Retrieve the current command */
		cbl_get_current_command(cc->cmdbuf, buf, sizeof(buf));
		
		/* Set the new text and position the cursor */
		gtk_entry_set_text(GTK_ENTRY(cc->entry), buf);
		gtk_editable_set_position(GTK_EDITABLE(cc->entry), cbl_get_entry_cursor(cc->cmdbuf));
	}
	else if( isprint(event->keyval) )
	{
		char	txt[2]	= {(char)keyval, 0};

		cbl_handle_command(cc->cmdbuf, txt[0]);

		/* Retrieve the current command */
		cbl_get_current_command(cc->cmdbuf, buf, sizeof(buf));
		
		/* Set the new text and position the cursor */
		gtk_entry_set_text(GTK_ENTRY(cc->entry), buf);
		gtk_editable_set_position(GTK_EDITABLE(cc->entry), cbl_get_entry_cursor(cc->cmdbuf));
		
	}
	
	return TRUE;
}
