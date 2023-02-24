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
#include <time.h>

/*-----------------------------
 *  Non-standard include files
 *-----------------------------
 */
#include "ir2.h"

#define VERBOSE 0
/*-----------------------------------------------------------------
** sock_readline() - tries to return the next line from the socket.
**    A line is a string terminated by the '\n' neline character.
**    If ERR_TIMEOUT, ERR_EXCEED_LIMIT any data read is returned. All data is null termination.
**    return code:
**       ERR_NONE         - A line with at \n is returned.
**       ERR_TIMEOUT      - A full line was not returned, check byte_read for partial data.
**       ERR_EXCEED_LIMIT - socket data exceed sizeof(rbuf). Partial data returned.
**       ERR_SOCKET_ERR   - Socket IO error. (maybe socket was terminated )
**-----------------------------------------------------------------
*/
int sock_readline(
   int fd,                // I: file descriptor
   char * rbuf,           // O: data read return here. Always null terminated.
   int sizeof_rbuf,       // I: sizeof(rbuf) buffer
   int * bytes_read,      // O: return bytes read
   int timeout_ms         // I: Total timeout to wait for data. 
)
{
   int  rc,
         n,
         wait_ms;
   struct timeval start;
   struct timeval now;
   struct timeval wait;

   fd_set readfds;

if(VERBOSE) printf("sock_readline()....\n");

   rbuf[0] = 0;                          /* zero the string */
   (*bytes_read) = 0;
   sizeof_rbuf = MAX(sizeof_rbuf-1, 0);  /* minus 1, so we can null terminate buffer. */
   gettimeofday( &start, NULL);

   // read until: error, timeout, or ack.
   while ( 1 )
   {
      gettimeofday( &now, NULL);
      wait_ms = timeout_ms - elapse_msec( &start, &now);
      if( wait_ms < 0 ) wait_ms = 0;

      FD_ZERO( &readfds );
      FD_SET( fd, &readfds );
      wait.tv_sec = wait_ms/1000;
      wait.tv_usec = (wait_ms%1000)*1000;

      n = select( fd+1, &readfds, NULL, NULL, &wait );
      if(VERBOSE) printf("sock_readline.select return %d rfds %d \n", n, FD_ISSET(fd, &readfds));
      if( n < 0 )
      {
         perror("sock_readline().select() error!");
         return ERR_SOCKET_ERR;
      }
      else if( n == 0 )
      {
         if(VERBOSE) printf("sock_readline().select() TIMED OUT! timeout_ms=%d\n", timeout_ms);
         if(VERBOSE) printf("  bytes_read %d:%s\n", (*bytes_read), rbuf);
         return ERR_TIMEOUT;
      }
      else if( n > 0 ) /* data is available */
      {
         rc =  recv( fd, rbuf+(*bytes_read), 1, 0 );  /* read 1 char */
         if( rc <= 0 )  // should have data, so 0 means peer has shutdown.
         {
            //perror("sock_readline().recv() error");
            return ERR_SOCKET_ERR;
         }

         *bytes_read += rc;                      /* increment bytes_read */
         sizeof_rbuf = MAX(sizeof_rbuf-rc, 0);   /* decrease available rbuf size */
         rbuf[*bytes_read] = 0;                  /* NULL terminate the buffer */

         /* return if end-of-line */
         if( rbuf[(*bytes_read)-1] == '\n')
         {
           return ERR_NONE;
         }

         /* if we don't have any rbuf space, return */
         if( sizeof_rbuf < 1 )
         {
            if(VERBOSE) printf("sock_readline() rbuf ERR_NOSPACE bytes_read=%d\n", (*bytes_read));
            return ERR_EXCEED_LIMIT;
         }
      }

   } // while(1)

   return ERR_NONE;
}

