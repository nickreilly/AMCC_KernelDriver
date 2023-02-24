/****************************************************************************
*  pathname.c - misc routines/functions involving path/file names
*****************************************************************************/

#define EXTERN extern

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <pwd.h>
#include <time.h>
#include <semaphore.h>
#include <mqueue.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>

#include "ir2.h"


#define DIRECTORY_MODE  (S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH)
                         // DIRECTORY_MODE mask give user full privilages and
								 // group and others read and execute privilages

/*-------------------------------------------------------------------
**   cat_pathname () - concatenates the path and filename string to
**           pathname. maxlen should be set to sizeof(pathname).
**           returns pathname.
**-------------------------------------------------------------------
*/
char * cat_pathname( char * pathname, char * path, char * filename, int maxlen)
{
   char tempstr[512];
   char * cptr;
   int l;

   maxlen = MIN( maxlen, sizeof(tempstr));
   strxcpy(tempstr, path, maxlen);
   unpad( tempstr, ' ' );

   l = strlen(tempstr);
   if( l>0 && tempstr[l-1] != '/' && l < maxlen)
      strcat(tempstr, "/");
   for( cptr=filename; (*cptr==' ') && (*cptr != 0) ;) cptr++;
   strxcat( tempstr, cptr, sizeof(tempstr) );
   strxcpy( pathname, tempstr, maxlen);
   return pathname;
}

/*----------------------------------------------------------------------------
**  expand_pathname() - Expands a pathname in the form ~username/... or $VAR/...
**----------------------------------------------------------------------------
*/
int expand_pathname( char *pathout, int pathout_size, char *pathin)
{
   int  len, 
		  firsttime,
		  endding_slash,
		  rc;
   char *bufin,
        *bufout;
   char *token,
        *cptr;
   char *st_ptr;   // for strtok_r()
   struct passwd * pw_entry;
   time_t now;
   struct tm *today;
   char date[10];

   /* Allocate and initialize memory */
   bufin  = (char*) malloc((unsigned)MAXPATHLEN);
   bufout = (char*) malloc((unsigned)MAXPATHLEN);
   if( bufin==NULL || bufout==NULL)
   {
      if( bufin )  free( bufin );
      if( bufout ) free( bufout );
      return ERR_MEM_ALLOC;
   }
	bufin[0] = bufout[0] = 0;

   rc = ERR_NONE;

   if( pathin == NULL )
		{ rc = ERR_INV_FORMAT; goto ldone; }

	strxcpy(bufin, pathin, MAXPATHLEN);
   unpad( bufin, ' ');

   if( bufin[0] == 0 )
		{ rc = ERR_INV_FORMAT; goto ldone; }

   /* Does the output string start with a '/' */
	if( pathin[0] == '/' )
	  strcpy( bufout, "/");

   /* do we need to append '/' to end of output string */
   if( pathin[strlen(pathin)-1] == '/')
		endding_slash = TRUE;
   else
		endding_slash = FALSE;

   /*-----------------------------------------------------------------
	** Parse each token, expand if necessary 
	*/
   firsttime = TRUE;
	token = strtok_r( bufin, "/ ", &st_ptr);  /* first token */

	while( token != NULL )
	{
		len = strlen( token );

		if( (firsttime) && (token[0]=='~') && (len==1) )
		{
			/* For ~ on first token. */
			if( NULL != (cptr = getenv("HOME")) )
				token = cptr;
		}
		if( (firsttime) && (token[0]=='~') && (len>1) )
		{
			/* For ~username on first token. */
			if( NULL != ( pw_entry = getpwnam(token+1)) )
				token = (char*)pw_entry->pw_dir;
		}
		else if( (token[0]=='$') && (len > 1) )
		{
         // see if $DATE
         if( !strcmp( token, "$DATE") )
         {
            now = time((time_t *) 0);
            today = gmtime( &now );
            sprintf( date, "%02d%02d%02d", today->tm_year%100, today->tm_mon+1, today->tm_mday );
            token = date;
         }   
			/* expand any other $KEYWORD from enviroment variables */
			else if( NULL != (cptr = getenv(token+1)) )
				token = cptr;
		}
		
		/* copy bufin to bufout */
		strcat( bufout, token);

      /* get next token */
		if( (token = strtok_r(NULL, "/ ", &st_ptr)) != NULL )
			strcat( bufout, "/");

		firsttime = FALSE;
	}

   if( endding_slash && bufout[strlen(bufout)-1] != '/')
		strcat( bufout, "/");

ldone:
   /* copy back to user */
	strxcpy( pathout, bufout, pathout_size);
	
   free( bufin );
   free( bufout );
   return rc;
}


/*----------------------------------------------------------------------------
**  get_full_pathname() - Get a full pathname of an existing dir.
**                        ie: gets the full name of stuff like "../home".
**----------------------------------------------------------------------------
*/
int get_full_pathname( 
	char *pathout,         // output: full pathname stored here.
	int   pathout_size,    //  input: sizeof(pathout)
	char *pathin           //  input: directory to evaluate.
)
{
   int  rc;
   char *workingdir,
        *dirname;
 
   /* Allocate and initialize memory */
   workingdir  = (char*) malloc((unsigned)MAXPATHLEN);
   dirname     = (char*) malloc((unsigned)MAXPATHLEN);
   if( workingdir==NULL || dirname==NULL)
   {
      if( workingdir )  free( workingdir );
      if( dirname ) free( dirname );
      return ERR_MEM_ALLOC;
   }
 
   rc = ERR_NONE;
 
   /* Get the current working directory */
   getcwd( workingdir, MAXPATHLEN);
 
   strxcpy( dirname, pathin, MAXPATHLEN);
   unpad( dirname, ' ');
 
   /* change to dirname & use getcwd() to expand to full name */
   if( chdir( dirname )==0 )
   {
      getcwd( dirname, MAXPATHLEN);
      strxcpy( pathout, dirname, pathout_size);
   }
   else
      rc = ERR_INV_PATH;
 
   /* return to original working directory */
   chdir( workingdir );
 
   free( workingdir );
   free( dirname );
   return rc;
}


/*--------------------------------------------------------------------------
 * create_path() - creates a new path
 *                return value: -1 : unsucessful
 *                               0 : pathname exist
 *--------------------------------------------------------------------------
 */
int create_path( p )
	char * p;
{
   char * path,
        * newpath,
        * name;
   int slash;
   char * st_ptr; // for strtok_r()

   if( NULL == (path = malloc((u_int) MAXPATHLEN)) )  /* allocate temporary buffers */
      return -1;

   if( NULL == (newpath = malloc((u_int) MAXPATHLEN)) )
      { free( path ); return -1; }

   strxcpy( path, p, MAXPATHLEN );            /* make copies of the data */
   unpad(path, ' ');

   memset( newpath, '\0', MAXPATHLEN);

   slash = (*path == '/' ? TRUE: FALSE);
   if( NULL == (name = strtok_r( path, "/ ", &st_ptr)))
   {
      free(path);
      free(newpath);
      return -1;
   }

   do {                                       /* Process each token      */
      if( slash )
        strcat(newpath, "/");
      else
        slash = TRUE;

      strcat( newpath, name);
      if( exist_path( newpath) < 0 )
      {
         if( mkdir(newpath, DIRECTORY_MODE) < 0 )
         {
            free(path);
            free(newpath);
            return -1;
         }
      }
   }
   while( NULL != (name = strtok_r( (char*)NULL, "/ ", &st_ptr)) );

   free(path);
   free(newpath);
   return 0;
}

 
/*-------------------------------------------------------------------
**   dir_from_path () - extracts the file name portion from a
**           full pathname.
**-------------------------------------------------------------------
*/
void dir_from_path( char *dir, char *pathname, int dir_size )
{
   int  size;
   char * slash;
   char buf[512];
 
   /* initialize to null */
   *dir = 0;
 
   /* find last '/' in path */
   if( NULL == (slash = strrchr( pathname, '/')) )
      return;
 
   size = MIN(slash - pathname+1, dir_size);
   size = MAX( 0, size);
 
   /* copy directory to user. We copy to local variable buf just in case
      the destination filename is the same or overlaps the pathname. */
   strxcpy( buf, pathname, size);
   strcpy( dir, buf);
}
 

/*-----------------------------------------------------------
**   exist_path() - Test of pathname is a ready a directory.
**         Returns:   0   pathname exist.
**                   -1   pathname is not a directory.
**-----------------------------------------------------------
*/
int exist_path( char * pathname )
{
   struct stat sbuf;

   if( -1 == stat( pathname, &sbuf))
      return -1;

   if( sbuf.st_mode & (S_IFDIR | S_IREAD | S_IEXEC ))
      return 0;
   else
      return -1;
}

 
/*-------------------------------------------------------------------
**   filename_from_path () - extracts the file name portion from a
**           full pathname.
**-------------------------------------------------------------------
*/
void filename_from_path( 
	char *filename,   // output: filename here
	char *pathname,   //  input: pathname
	int filename_size 
)
{
   char * slash,
        * start;
   char buf[80];
 
   /* initialize to null */
   *filename = 0;
 
   /* find last '/' in path */
   if( NULL == (slash = strrchr( pathname, '/')) )
      start = pathname;
   else
      start = slash+1;
 
   /* copy filename to user. We copy to local variable buf just in case
      the destination filename is the same or overlaps the pathname. */
   strxcpy( buf, start, filename_size);
   strcpy(  filename, buf);
}

/************************ eof ************************/
