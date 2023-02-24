/***************************************************************************
 *
 *  socket.c - routine for socket communcation
 *
 ***************************************************************************
 */

#define EXTERN extern

/*--------------------------
 *  Standard include files
 *--------------------------
 */
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/param.h>

/*-----------------------------
 *  Non-standard include files
 *-----------------------------
 */

#include "ir2.h"

/*-----------------------------------------------------------------------
**  sock_write_msg() - writes a message to the socket fd.
**       return:  -1   error writing to sock.
**                >0   number of bytes written. (will always write
**                     SOCKET_MSG_LEN bytes to stream.)
**-----------------------------------------------------------------------
*/
int sock_write_msg( 
   int  fd,           /* Socket ID */
   char * msg,        /* Message to send to PC */
   int  blocking,     /* specify blocking (TRUE) or non-blocking (FALSE) */
	int socket_timeout_ms
)
{
   int len;

   if( ( len = sock_write_data(fd, msg, SOCKET_MSG_LEN, blocking,
										 socket_timeout_ms)) < 0 )
   {
      len =  ERR_SOCKET_ERR;
   }
   return len;
}


