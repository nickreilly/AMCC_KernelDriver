#define EXTERN extern

/*--------------------------
 *  Standard include files
 *--------------------------
 */
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
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
**  sock_setup_server() - Sets up a socket connection to accept
**       connection request.
**       Returns:    -1 =  Unable to estalish connection.
**                    otherwise the fd of socket is returned.
**-----------------------------------------------------------------------
*/
int sock_setup_server( 
	int port_no
)
{
   int fd;
	int one;
   struct sockaddr_in server;

   /*
   **  Create socket...
   */
   if( (fd = socket( AF_INET, SOCK_STREAM, 0)) < 0 )
	{
	   perror("sock_setup_server create");
      return -1;
	}

   // allow address to be reused.
   one = 1;
	setsockopt( fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof( one ));
   /*
   **  name socket using <host_addr>, <UI_PORT>.
   */
   server.sin_family = AF_INET;
   server.sin_addr.s_addr = htonl(INADDR_ANY);
   server.sin_port = htons( (unsigned short)port_no);
   if( bind(fd, (struct sockaddr *) &server, sizeof server) < 0 )
   {
		printf("sock_setup_server: port_no = %d\n", port_no );
		perror("sock_setup_server bind");
		return -1;
	}

   /*
   ** Inquire <address>, <port no> and print it out
   */
	if( 0 )
   {
      char buf[80];
      socklen_t len;
      len = sizeof server;
      getsockname( fd, (struct sockaddr *) &server, &len );
      gethostname( buf, sizeof buf);
      printf("Socket server established [%s] port #%d\n",
             buf, ntohs(server.sin_port));
   }
   /*
   **  Listen for connection request.
   */
   listen( fd, 5);
   return fd;
}

