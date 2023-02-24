/*******************************************************************
** CBL - command buffer list
** 
** A collection of function that provide the mechanism to have  
** input buffer editing (backspace, arrow keys), and a command
** history list. 
**
********************************************************************
*/
#define EXTERN extern

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "ir2.h"

/*--------------------------------------
** static defines / macros / prototypes
*/

/* Minimum value for the maximum number of entries in a list */
#define _CBL_MIN_ENTRIES		1

/* prototypes */
static void cbl_make_new_entry( cbl_list *pThis );

///////////////////////////////////////////////////////////////////////////////
// Public functions 
///////////////////////////////////////////////////////////////////////////////


/*-------------------------------------------------------------------
** cbl_list() - Allocated a cbl_list and its list entries.
**-------------------------------------------------------------------
*/
cbl_list *cbl_new (
	int  max_entries     //  I: Number of history entires to allocated.
)
{
	cbl_list	*list	= NULL;
	
	/* Validate the specified maximum number of entries */
	if( max_entries < _CBL_MIN_ENTRIES )
		return NULL;
	
	/* Allocate the list */
	if( (list = malloc(sizeof(cbl_list))) == NULL )
		return NULL;
	
	/* Clear out the list */
	memset(list, 0, sizeof(cbl_list));
	
	/* Allocate storage for the list entries. We will allocate one extra entry */
	/* to make recycling locations a bit easier later. */
	if( (list->entries = malloc((max_entries + 1) * sizeof(cbl_entry))) == NULL )
	{
		free(list);
		return NULL;
	}

	/* Clear out the list entries */	
	memset(list->entries, 0, (max_entries + 1) * sizeof(cbl_entry));

	/* Initialize members */	
	list->current		= 0;
	list->count		= 1;
	list->entry_cursor	= 0;
	list->max_entries	= max_entries;
	
	return list;
}

/*-------------------------------------------------------------
** cbl_destroy() - Destroys (deallocates) a command buffer list  
**--------------------------------------------------------------
*/
void cbl_destroy(cbl_list *pThis)
{
	/* Free the entries and the list itself */
	free(pThis->entries);
	free(pThis);
}

/*-------------------------------------------------------------
** cbl_get_max_entries() - Retrieve the maximum number of allowable elements in the list 
**--------------------------------------------------------------
*/
int  cbl_get_max_entries(const cbl_list *pThis)
{
	return pThis->max_entries;
}

/*-------------------------------------------------------------
** cbl_get_count() - Retrieve the number of elements currently in the list
**--------------------------------------------------------------
*/
int  cbl_get_count(const cbl_list *pThis)
{
	return pThis->count;
}

/*-------------------------------------------------------------
** cbl_has_new_command() - Returns TRUE if new command is ready.
**--------------------------------------------------------------
*/
int cbl_has_new_command(const cbl_list *pThis)
{
	return pThis->new_command;
}

/*-------------------------------------------------------------
** cbl_get_last_command() - Retrieve the previously entered 
**  command, clearing the "new entry" flag if necessary. 
**  Calls to cbl_has_new_command() will return zero until another 
**  command is comitted. 
**--------------------------------------------------------------
*/
void cbl_get_last_command(cbl_list *pThis, char *buf, int buf_size)
{
	/* Clear the new flag */
	pThis->new_command	= FALSE;
	
	/* Retrieve the last command. If no command has been entered, */
	/* clear the user-specified buffer. */
	if( pThis->count > 1 )
		strxcpy(buf, pThis->entries[1].cmd, buf_size);
	else
		buf[0] = 0;
}

/*------------------------------------------------------------------------
** cbl_get_command() - Retrieve the command at the specified list position
**-------------------------------------------------------------------------
*/
int cbl_get_command(const cbl_list *pThis, int index, char *buf, int buf_size)
{
	/* Validate the index */
	if( index < 0 || index >= pThis->count )
		return ERR_INV_RNG;
	
	/* Copy the command text */
	strxcpy(buf, pThis->entries[index].cmd, buf_size);

	return ERR_NONE;
}

/*-------------------------------------------------------------
** cbl_get_current_command() - Retrieve the text of the command 
**    currently being edited. 
**--------------------------------------------------------------
*/
void cbl_get_current_command(const cbl_list *pThis, char *buf, int buf_size)
{
	const cbl_entry *entry = ((pThis->browsing) ? (&pThis->edited):(pThis->entries));
	
	/* Copy the command text into the buffer */
	strxcpy(buf, entry->cmd, buf_size);
}

/*--------------------------------------------------------------------------------
** cbl_set_current_command() - Set the text of the command currently being edited. 
**---------------------------------------------------------------------------------
*/
void cbl_set_current_command(cbl_list *pThis, const char *text)
{
	cbl_entry *entry = ((pThis->browsing) ? (&pThis->edited):(pThis->entries));
	
	/* Copy the command text into the buffer */
	strxcpy(entry->cmd, text, sizeof(entry->cmd));
	
	/* Update the entry cursor position */
	pThis->entry_cursor = strlen(entry->cmd);
}

/*-------------------------------------------------------------
** cbl_get_entry_cursor() - Retrieve the entry cursor position 
**    of the command currently being edited. 
**--------------------------------------------------------------
*/
int cbl_get_entry_cursor(const cbl_list *pThis)
{
	return pThis->entry_cursor;
}

/*-------------------------------------------------------------
** cbl_set_entry_cursor() -  Set the entry cursor 
** position of the command currently being edited.
**--------------------------------------------------------------
*/
void cbl_set_entry_cursor(cbl_list *pThis, int cursor)
{
	cbl_entry *entry = ((pThis->browsing) ? (&pThis->edited):(pThis->entries));
	
	/* Move the cursor */
	if( cursor < 0 )
		pThis->entry_cursor = 0;
	else if( cursor >= strlen(entry->cmd) )
		pThis->entry_cursor = strlen(entry->cmd);
	else
		pThis->entry_cursor = cursor;
}

/*-------------------------------------------------------------
** cbl_handle_command() -  Set the entry cursor Send a command 
**   to the list. command can be a printable character 
**   or one of the CBL_CMD_* constants defined above 
**--------------------------------------------------------------
*/
int cbl_handle_command(cbl_list *pThis, int command)
{
	/* The command was not handled and did not fail - let the list handle it */
	switch( command )
	{
		case CBL_CMD_COMMIT:
			if( pThis->browsing )
			{
				/* Copy the edit buffer into the first list entry */
				memcpy(&pThis->entries[0], &pThis->edited, sizeof(cbl_entry));
				
				/* Make room for a new entry */
				cbl_make_new_entry(pThis);
				
				/* Update the current pointer and clear the browse flag. */
				pThis->browsing	= FALSE;			
			}
			else
			{
				/* Make room for a new entry */
				cbl_make_new_entry(pThis);
			}
			
			/* Set the current entry to the first entry */
			pThis->current		= 0;
			pThis->entry_cursor	= 0;
			
			/* Set the new command flag */
			pThis->new_command	= TRUE;

			break;
		
		case CBL_CMD_UP:
			/* Are we already at the end of the list? */
			if( pThis->current >= pThis->count - 1 )
				break;

			/* Set the browsing flag and advance the current pointer */
			pThis->browsing = TRUE;
			pThis->current++;
			
			/* Store the current command in the edit buffer */
			memcpy(&pThis->edited, &pThis->entries[pThis->current], sizeof(cbl_entry));
			
			/* Update the entry cursor */
			pThis->entry_cursor	= strlen(pThis->edited.cmd);
			break;
			
		case CBL_CMD_DOWN:
			if( pThis->current <= 0 )
				break;

			/* Decrement the current pointer */		
			pThis->current--;
			
			/* If the current pointer becomes zero, clear the browsing flag. */
			/* Otherwise, update the edit buffer. */
			if( pThis->current == 0 )
			{
				pThis->browsing		= FALSE;
				pThis->entry_cursor	= strlen(pThis->entries[0].cmd);
			}
			else
			{
				memcpy(&pThis->edited, &pThis->entries[pThis->current], sizeof(cbl_entry));
				pThis->entry_cursor	= strlen(pThis->edited.cmd);
			}
			break;
			
		case CBL_CMD_LEFT:
			if( pThis->entry_cursor > 0 )
				pThis->entry_cursor--;
			break;
				
		case CBL_CMD_RIGHT:
			{
				int		cmd_len	= 0;
				cbl_entry	*entry	= NULL;

				/* Get a pointer to the working entry */			
				entry = (pThis->browsing) ? (&pThis->edited):(&pThis->entries[0]);
				
				cmd_len = strlen(entry->cmd);
			
				if( pThis->entry_cursor < cmd_len )
					pThis->entry_cursor++;
			}
			break;

		case CBL_CMD_DELETE:
			{
				int		cmd_len	= 0;
				cbl_entry	*entry	= NULL;

				/* Get a pointer to the working entry */			
				entry = (pThis->browsing) ? (&pThis->edited):(&pThis->entries[0]);
				
				cmd_len = strlen(entry->cmd);

				/* Don't need to do anything if we're at the end */
				if( pThis->entry_cursor >= cmd_len )
					break;
				
				/* Move all characters after the cursor down one position */
				memmove(&entry->cmd[pThis->entry_cursor],
						&entry->cmd[pThis->entry_cursor + 1], cmd_len - pThis->entry_cursor);
			}
			break;
		
		case CBL_CMD_BACKSPACE:
			{
				int		cmd_len	= 0;
				cbl_entry	*entry	= NULL;

				/* Get a pointer to the working entry */			
				entry = (pThis->browsing) ? (&pThis->edited):(&pThis->entries[0]);
				
				cmd_len = strlen(entry->cmd);

				/* Don't need to do anything if we're at the beginning */
				if( pThis->entry_cursor == 0 )
					break;
				
				/* Move all characters from the cursor position down */
				memmove(&entry->cmd[pThis->entry_cursor - 1],
						&entry->cmd[pThis->entry_cursor],
						cmd_len - pThis->entry_cursor + 1);
				
				/* Update the cursor */
				pThis->entry_cursor--;
			}
			break;

		default:
			/* If the command is a printable character... */
			if( isprint(command) )
			{
				int		cmd_len	= 0;
				cbl_entry	*entry	= NULL;

				/* Get a pointer to the working entry */			
				entry = (pThis->browsing) ? (&pThis->edited):(&pThis->entries[0]);
				
				cmd_len = strlen(entry->cmd);

				/* Can't handle another character? */
				if( cmd_len >= (CBL_MAX_CMD_LEN-1) )
					break;
			
				/* Special case - appending characters */
				if( pThis->entry_cursor == cmd_len )
				{
					entry->cmd[pThis->entry_cursor] = (char) command;
					pThis->entry_cursor++;
					entry->cmd[pThis->entry_cursor] = 0;  // terminate string.;
					break;
				}
			
				/* Make room for the new character */
				memmove(&entry->cmd[pThis->entry_cursor + 1],
						&entry->cmd[pThis->entry_cursor],
						cmd_len - pThis->entry_cursor);
			
				/* Insert the character and update the cursor */
				entry->cmd[pThis->entry_cursor]	= (char)command;
				pThis->entry_cursor++;
			}
			break;
	}
	
	return ERR_NONE;
}


//////////////////////////////////////////////////////////////////////////////
// static functions 
///////////////////////////////////////////////////////////////////////////////

/*---------------------------------------------------------------------------
** cbl_make_new_entry() - Move all entries in the list up one position, 
**  making room for a new entry. The new entry is cleared and the list count 
**  is updated if necessary. 
**---------------------------------------------------------------------------
*/
void cbl_make_new_entry(cbl_list *pThis)
{
	int i;
	
	/* Move entries up one */
	for( i = pThis->count - 1; i >= 0; i-- )
		memcpy(&pThis->entries[i + 1], &pThis->entries[i], sizeof(cbl_entry));

	/* Increment the count by one if we have not surpassed the max */
	if( pThis->count < pThis->max_entries )
		pThis->count++;
	
	/* Clear the new entry */
	memset(&pThis->entries[0], 0, sizeof(cbl_entry));
}

