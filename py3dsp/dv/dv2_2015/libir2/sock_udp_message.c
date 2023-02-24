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
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <memory.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/param.h>

/*-----------------------------
 *  Non-standard include files
 *-----------------------------
 */

#include "ir2.h"

/*--------------------------------------------------------------------
** sock_udp_message -  sends a packet and receives a reply via udp
**   success: Returns ERR_NONE and the data.
**   failure: ERR_ code; caller should close the socket.
**--------------------------------------------------------------------
*/
int sock_udp_message(
   int    fd,                   /* I: file descriptor for socket */
   void * send_buf,             /* I: send buffer */
   int    nbytes_send,          /* I: number of bytes to send */
   void * read_buf,             /* O: recd buffer */
   int    nbytes_read,          /* I: number of bytes to read  */
   int    timeout_ms            /* I: timeout in millseconds */
)
{
   int rc, n;
   fd_set fds;
   struct timeval wait;

   // send UDP data
   rc = send( fd, send_buf, nbytes_send, 0 );
   if( rc != nbytes_send )
   {
      perror("send");
      printf( "sock_udp_message().sendto error %d errno=%d\n", rc, errno);
      return ERR_SOCKET_ERR;
   }

   //-----------------------------
   // read response
   //

   FD_ZERO( &fds );
   FD_SET( fd, &fds );
   wait.tv_sec = timeout_ms/1000;
   wait.tv_usec = (timeout_ms%1000)*1000;

   n = select( fd+1, &fds, NULL, NULL, &wait );
   if( n > 0 )
   {
      rc = recvfrom( fd, read_buf, nbytes_read, 0, NULL, NULL );
      if( rc < 0 )
      {
         //perror("recvfrom");
         //printf( "sock_udp_message().recvfrom() error %d\n", rc);
         return ERR_SOCKET_ERR;
      }

      return ERR_NONE;
   }

   /* select return -1 (error) or 0 (timed out) */
   if( n < 0 )
	{
      //perror("select() error!");
	}
   if( n == 0)
	{
      //printf("select() timed out\n");
	}
   //printf("sock_udp_message().select() returned %d.\n", n);

   return ERR_SOCKET_ERR;
}

