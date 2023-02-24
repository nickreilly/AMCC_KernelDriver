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

/*-----------------------------------------------------------------------
**  sock_write_data() - writes bufsize number of bytes at location
**       buf at file description sock.
**       return:  -1   error writing to sock.
**                >0   number of bytes written.
**-----------------------------------------------------------------------
*/
int32_t sock_write_data( 
   int  fd,                 /* Socket's file descriptor   */
   char * buf,              /* Pointer to buffer area */
   int32_t bufsize,            /* number of byte to send */
   int  blocking,           /* specify blocking (TRUE) or non-blocking (FALSE) */
   int  timeout_ms          /*  timeout in ms for non-blocking sockets */ 
)
{
   int  len,
        sleep_cnt,
        status_flags;
   int32_t total;

	status_flags = 0;
	timeout_ms = MAX(50,timeout_ms);  /* 0.05 sec minimum */

   if( !blocking )                 /* making it non-blocking */
   {
      status_flags = fcntl( fd, F_GETFL, 0);
      if( fcntl(fd, F_SETFL, O_NDELAY) < 0)
         return ERR_SOCKET_ERR;
   }

   sleep_cnt = timeout_ms/10; 
   total = 0;
   while( bufsize > 0 )
   {
      len = ( bufsize > SOCK_PACKET_SIZE ? SOCK_PACKET_SIZE : bufsize );
      len = write(fd, (char *) buf, len);

      if( len  < 0 )
      {
         if( (errno == EWOULDBLOCK) || (errno==EINTR) )
         {
            if( !(sleep_cnt--))
               return ERR_SOCKET_TIMEOUT;
            printf("sock_write_data() usleep ZZZZ\n");
            usleep( 10000);          /* sleep for 0.01 seconds */
         }
         else
         {
            return ERR_SOCKET_ERR;
         }
      }
      else
      {
         bufsize -= len;   /* update counter & pointer */
         buf += len;
         total += len;

			sleep_cnt = timeout_ms/10;  /* reset sleep counter when you write data */
      }
   }

   if( !blocking )                   /* restore flages */
   {
      fcntl( fd, F_SETFL, status_flags);
   }
   return total;
}

