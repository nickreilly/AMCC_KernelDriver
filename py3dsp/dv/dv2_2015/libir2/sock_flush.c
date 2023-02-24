#define EXTERN extern

/*--------------------------
 *  Standard include files
 *--------------------------
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

/*-----------------------------
 *  Non-standard include files
 *-----------------------------
 */
#include "ir2.h"

/*-----------------------------------------------------------------
** sock_flush() - read the input buffer so that are no char waiting.
**       ERR_NONE         - A line with at \n is returned.
**       ERR_SOCKET_ERR   - Socket IO error. (maybe socket was terminated )
**-----------------------------------------------------------------
*/
int sock_flush(
   int fd                 // I: file descriptor
)
{
   int n;
   fd_set readfds;
   struct timeval wait;
   char buf[512];
   int  x;

   while( 1 )
   {
      FD_ZERO( &readfds );
      FD_SET( fd, &readfds );
      wait.tv_sec  = 0;       // zero so select() returns Immediately.
      wait.tv_usec = 0;

      n = select( fd+1, &readfds, NULL, NULL, &wait );
		//printf("sock_flush.select return %d rfds %d \n", n, FD_ISSET(fd, &readfds));
      if( n < 0 )
         return ERR_SOCKET_ERR;
      if( n == 0 )
         return ERR_NONE;   // no data, read buffer are empty.

      x = read( fd, buf, sizeof(buf));
      if( x < 0 )                  // read return an error
         return ERR_SOCKET_ERR;
      else if ( x == 0 )           // select was 1, but no data, error (happend if socket was closed).
         return ERR_SOCKET_ERR;
      else // ( x > 0 )            // has data, try again.
      {
         buf[x] = 0;
         //printf( "sock_flush read %d bytes: %s \n", x, buf);
      }
   }
}

