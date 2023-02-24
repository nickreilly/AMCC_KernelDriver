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
**  sock_read_msg() - read a message from the pc.
**       Returns:    ERR_SOCKET_ERR  error reading socket.
**                  >0   message received. Len of message should always
**                       be SOCKET_MSG_LEN.
**-----------------------------------------------------------------------
*/
int sock_read_msg(
   int  fd,              /* Socket ID */
   char * msg,           /* Places message in this buffer */
   int  blocking,        /* specify blocking (TRUE) or non-blocking (FALSE) */
	int socket_timeout_ms /* number of millisecond for a timeout error */
)
{
   int  rc, l;

   rc = sock_read_data( fd, msg, SOCKET_MSG_LEN, blocking, socket_timeout_ms);

	/* If partial data recieved, clear out user's buffer */
	l = ( rc < 0 ? SOCKET_MSG_LEN : rc );
	if( l != SOCKET_MSG_LEN )
      memset( msg+l, 0, SOCKET_MSG_LEN-l);

   return rc;
}

