#ifndef __CMDCON_H_INCLUDED
#define __CMDCON_H_INCLUDED

/*
** Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php
** Copyright (C) 2012 <NASA IRTF. http://irtfweb.ifa.hawaii.edu>
*/

#define CMDCON_BLACK             0
#define CMDCON_RED               1
#define CMDCON_GREEN             2
#define CMDCON_BLUE              3
#define CMDCON_GRAY              4
#define CMDCON_CYAN              5
#define CMDCON_MAGENTA           6
#define CMDCON_YELLOW            7
#define CMDCON_WHITE             8
#define CMDCON_NUM_STATIC_COLORS 9

typedef struct _cc_console	cc_console;

typedef void (*cc_cmd_callback)(cc_console*,char*,int);

struct  _cc_console
{
	GtkWindow	*wnd;		   // Main app window
	GtkWidget	*container;	// Table widget to is the container
	GtkWidget	*swnd;		// Scroller window
	GtkWidget	*console;	// text view for display status
	GtkWidget	*entry;		// Entry widget for entering commands
	
	cc_cmd_callback	cb;		// User's can register callback for cmdconsole
	int		         data; 	// User's Callback data for cmdconsole.

	cbl_list	*cmdbuf;	// Command list

   GtkTextTag  *color_tags[CMDCON_NUM_STATIC_COLORS];
};

// Create a new GTK command console. The main_wnd parameter
// should be a pointer to the application's main window.
cc_console *cc_create_console(GtkWindow *main_wnd);

void cc_create_tags(cc_console *cc);

void cc_destroy_console(cc_console *cc);

// Set the callback routine that will be invoked when the
// user enters a new string
void cc_set_entry_callback(cc_console *cc, cc_cmd_callback cb, int data);

// Print a message, printf-style, to the specified console.
// Pass in NULL as col to use the default color
void cc_printf( cc_console *cc, int color, const char *fmt, ...);

// Returns non-zero if there is a new command waiting to be retrieved
gboolean cc_is_command_pending(const cc_console *cc);

// Get the last command and clear the "new command" flag
// (i.e., the value returned by cc_is_command_pending()
// will now be FALSE.
gboolean cc_get_last_command(cc_console *cc, char *buf, int n);

/* Mimick execution of a command. The use_history parameter is a flag */
/* that should be set to non-zero if the specified command is to be added to */
/* the command history list.  The echo_cmd parameter is a second flag that should */
/* be set to non-zero if the command text should be echoed in the console output box. */
void cc_execute_command(cc_console *cc, const char *cmd, gboolean use_history, gboolean echo_cmd);

#endif	// #ifndef __CMDCON_H_INCLUDED
