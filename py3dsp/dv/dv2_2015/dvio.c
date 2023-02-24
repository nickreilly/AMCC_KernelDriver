/****************************************************************************
*  dvio.c - send a command to dv. Command are send to DV, but no
*     feedback is returned.
*     
* Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php
* Copyright (C) 2012 <NASA IRTF. http://irtfweb.ifa.hawaii.edu>
****************************************************************************
*/

#define EXTERN
#define MAIN    1
#define DEBUG 0

/*--------------------------
 *  Standard include files
 *--------------------------
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <stdarg.h>
#include <math.h>
#include <signal.h>
#include <ctype.h>
#include <errno.h>

/*-----------------------------
 *  Non-standard include files
 *-----------------------------
 */
#include "./libir2/ir2.h"

/*-----------------------------
 *  Prototypes 
 *-----------------------------
 */

int main (int argc, char *argv[]);
void usage (void);
int dv_cmd (char *cmd, char *reply, int reply_size, char *hostname, int port);

/*-----------------------------
 *  Global Variable 
 *-----------------------------
 */

char *Program_name;
int Verbose;         /* verbose flag */


/*-----------------------------------------------------------------------
**  main()
**-----------------------------------------------------------------------
*/

int main (int argc, char *argv[])
{   
   char hostname[25];
   char cmd[256];
   char reply[256];
   int  rc,
	     port,
        i, c;

   extern char *optarg;
   extern int optind, opterr;

   Program_name = argv[0];

	rc = ERR_NONE;
	strxcpy( hostname, "localhost", sizeof(hostname));
	port = IRTF_DV_PORT;
	Verbose = 0;

   /*----------------------------------------------------------
   ** parse command line arguments for application options.
   */
	while( (c = getopt( argc, argv, "vp:h:")) > 0 )
	{
		switch( c )
		{
			case 'p':
				i = atoi(optarg);
				if( INRANGE( 30000, i, 32000))
					port = i;
				break;

			case 'h':
				strxcpy( hostname, optarg, sizeof(hostname));
				break;

			case 'v':
				Verbose = 1;
				break;

			default:  
				rc = ERR_INV_OPERATION;
				break;

		}
	}
    /* must have at least 1 argument left (the tcs3 commmand) */
	if(  ((argc-optind) < 1)  || (rc != ERR_NONE ) )
    {
         usage();
         exit (1);
    }

    /*
     * append the rest of the parameter into a single string. 
     * This should be the command
     */
    strcpy (cmd, "");
    for (i = optind; i < argc; i++) {
         strcat (cmd, argv[i]);
         strcat (cmd, " ");
    }

    if (Verbose)
         printf ("  %s: command [%s] to %s:%d\n", Program_name, cmd, hostname, port);

    /*
     * send command. DV doesn't return anything via it's socket.
     */
    dv_cmd(cmd, reply, sizeof (reply), hostname, port);

    return 0;
}

/*-------------------------------------------------------------------------------
**   usage() - display usage to user.
**-------------------------------------------------------------------------------
*/
void usage (void)
{
    char *msg =
   " dvio sends a single command to dv using hostname & port \n"
   "\n"
   "    usage: %s [-v] [-h hostname] [-p port ] command \n"
   "\n";

    printf (msg, Program_name);
}

/*-------------------------------------------------------------------------------
**  dv_cmd()
**-------------------------------------------------------------------------------
*/
int dv_cmd (char *cmd, char *reply, int reply_size, char *hostname, int port)
{
   int fd;
   int len;

	if( (fd = sock_open( hostname, port, SOCK_STREAM, 0)) < 0)
   {
      return ERR_SOCKET_ERR;
   }

   len = sock_write_msg( fd, cmd, TRUE, 10);
   shutdown( fd, 2);
   close( fd );
   return (len==SOCKET_MSG_LEN ? 0 : ERR_FAILED);
}

