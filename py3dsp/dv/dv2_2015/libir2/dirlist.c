
#define EXTERN extern

/*-------------------------
**  Standard include files
**-------------------------
*/
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>

/*------------------------------
**  Non-standard include files
**------------------------------
*/

#include "ir2.h"

/*-------------------------------------------------------------------
** dirlist_* - set of functions to breakup a path into is parents directory
** names. These name are stored in a list in the structure dirlist_t.
**
**   dir_init()    - initialize structure to NULLs.
**   dir_newpath() - create list from path input.
**   dir_free() - deletes any allocated memory for list.
** -- Internal fuctions --
**   dir_makelist() - used by newpath().
**   dir_showlist() - prints structure variables
**
*/
void dirlist_init( struct dirlist_t *dl )
{
   memset( dl, 0, sizeof(*dl));
}

void dirlist_newpath( struct dirlist_t *dl, char *path )
{
   int i;

   dirlist_free( dl );

   /* copy path and make sure it end with a '/' */
   strxcpy( dl->path, path, sizeof(dl->path)-3);
   i = strlen( dl->path );
   if( dl->path[i-1] != '/' )
   {
      dl->path[i]   = '/';
      dl->path[i+1] = 0;
   }

   /* make list */
   dirlist_makelist( dl );
}

void dirlist_makelist( struct dirlist_t * dl )
{
   int l, i, end;
   char path[DIRLIST_MAX_PATH];

   strxcpy( path, dl->path, sizeof(path));

   l = 0;
   end = strlen( path );
   for( i=end; (i >= 0) && (l < DIRLIST_MAX_LIST-1); i-- )
   {
      if( path[i] == '/' )
      {
         dl->list[l] = malloc(end+1);
         strxcpy( dl->list[l], path, end+1);
         l++;
      }
      path[i] = 0;
   }
   dl->nele = l;
}
void dirlist_free( struct dirlist_t *dl )
{
   int i;

   dl->path[0] = 0;
   for( i=0; i<dl->nele; i++ )
	{
      free( dl->list[i] );
		dl->list[i] = NULL;
	}
   dl->nele = 0;
}

void dirlist_showlist( struct dirlist_t * dl )
{
   int i;
   printf("PATH: %s \n", dl->path);
   printf("nele: %d \n", dl->nele);
   for( i=0; i<dl->nele; i++ )
      printf("   %d. %s \n", i, dl->list[i]);
}



/************************ eof ************************/
