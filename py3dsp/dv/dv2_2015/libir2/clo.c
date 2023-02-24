/*****************************************************************************
** 
** Command Line Option - This function helps with command line arguments.
** 
** user must provide a clo_option_t array to describe options. clo_parse() then:
** 
**    Initializes user_var to their default values.
**    When clo_parse() find any options on the command line. The
**    user_var is set. And the option are removed from argc and
**    argv, which are adjusted appropriately.
**    When clo_parse() returns, argv/argc only contains
**    unrecognized options.
** 
**    clo_parse() returns an ERR_ code to indicated if it encounters any
**    input data errors.
** 
** Example:
**    int verbose;
**    char hostname[80];
** 
**    struct clo_option_t clo_option =
**    {
**       // ____type______   _flag_    _default__   _user_var_ _sizeof_user_var
**       CLO_TYPE_NO_ARG,   "-v",   "0",         &verbose, sizeof(verbose),
**       CLO_TYPE_STRING,   "-h",   "localhost", hostname, sizeof(hostname),
**    };
** 
**    rc = clo_parse( argc, argv, clo_option, sizeof(clo_option)/sizeof(struct clo_option_t));
** 
** 
******************************************************************************
*/

#define EXTERN extern

/*--------------------------
**  include files
**--------------------------
*/
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

/*-----------------------------
** Non-standard include files
**-----------------------------
*/

#include "ir2.h"

/*---------------------------------
** clo_parse()
**---------------------------------
*/
int clo_parse( 
   int *argc,                          // process's argc variable
   char * argv[],                      // process's argv[] variable
   struct clo_option_t clo_options[],  // command line option structure
   int num_clo_options                 // number of options
)
{
   int i, j, k, err;
   int found;

   /* Set default values for options */
   err = ERR_NONE;
	for( k=0; k<num_clo_options; k++ )
	{
		switch ( clo_options[k].type )
		{
			case CLO_TYPE_NO_ARG:
				sscanf( (char *)clo_options[k].default_str, "%d", (int*)clo_options[k].user_var );
				break;
			case CLO_TYPE_INT32:
				sscanf( (char *)clo_options[k].default_str, "%d", (int32_t*)clo_options[k].user_var );
				break;
			case CLO_TYPE_DOUBLE:
				sscanf( (char *)clo_options[k].default_str, "%lf", (double*)clo_options[k].user_var );
				break;
			case CLO_TYPE_STRING:
				strxcpy( (char *)clo_options[k].user_var, clo_options[k].default_str,
							 clo_options[k].sizeof_var );
				break;
			default:
			   err = ERR_INV_RNG;
				break;
		}      

		if( err )
		   return err;
	}                        

   /* loop on argv[] and see if we find any matching arguments */
   j = 1;
   for(i=1; i<*argc; i++ )
   {
      found = FALSE;

      // test arg against clo options
      for( k=0; k<num_clo_options; k++ )
      {
         if( strcmp( clo_options[k].flag, argv[i] ) == 0 )
         {
            // found a match - load user variable
            switch ( clo_options[k].type )
            {
               case CLO_TYPE_NO_ARG:
                  *(int *)clo_options[k].user_var = *(int *)clo_options[k].user_var ? 0 : 1;
                  found = TRUE;
                  break;  // no arguement, just return

               case CLO_TYPE_INT32:
                  if( *argc < (i+2))             // must have an additional argument
                     return ERR_INV_FORMAT;
                  sscanf( argv[i+1], "%d", (int32_t*)clo_options[k].user_var );
                  found = TRUE;
                  i++;                     // increment argv[] index
                  break;

               case CLO_TYPE_DOUBLE:
                  if( *argc < (i+2))             // must have an additional argument
                     return ERR_INV_FORMAT;
                  sscanf( argv[i+1], "%lf", (double*)clo_options[k].user_var );
                  found = TRUE;
                  i++;                     // increment argv[] index
                  break;

               case CLO_TYPE_STRING:   
                  if( *argc < (i+2))             // must have an additional argument
                     return ERR_INV_FORMAT;
                  strxcpy( (char *)clo_options[k].user_var, argv[i+1], clo_options[k].sizeof_var );
                  found = TRUE;
                  i++;                     // increment argv[] index
                  break;

               default:
						err = ERR_INV_RNG;
                  break;
            }      
         }          

         if( err )          // abort on any error.
            return err;
      }

      if( !found ) // if not a clo option, but back in the argv list 
      {
         argv[j] = argv[i];
         j++;
      }   
   }   
   *argc = j;    // adjust caller's argc to show remaining argument count.

   return ERR_NONE;
}

