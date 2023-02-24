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
**  sock_read_data() - reads bufsize bytes to buffer buf. This function
**       returns after it reads the # of bytes requested or a read error occurred.
**       if blocking is false, will return when no data is available.
**       Returns:   <0   error reading socket.
**                  >0   Number of bytes recieved.
**-----------------------------------------------------------------------
*/
int32_t sock_read_data( 
   int     fd,           /* I: Socket's file descriptor   */
   char    * rbuf,       /* O: Recieve buffer             */
   int32_t rbuf_size,    /* I: len of buffer in bytes     */
   int     blocking,     /* I: specify blocking (TRUE) or non-blocking (FALSE) */
   int     timeout_ms     /* I: if blocking, timeout for waiting on data in milliseconds */
)
{
   int rc,
		 bytes_read,
       n;

   fd_set readfds;
   struct timeval wait;

   bytes_read = 0;

   // read until: error, timeout, or ack.
   while ( 1 )
   {
      FD_ZERO( &readfds );
      FD_SET( fd, &readfds );

      wait.tv_sec = timeout_ms/1000;
      wait.tv_usec = (timeout_ms%1000)*1000;

      n = select( fd+1, &readfds, NULL, NULL, (blocking?NULL:&wait) );
      //printf("sock_read_data()select()=%d  b=%d ms=%d bread=%d \n", n, blocking, timeout_ms, bytes_read);
      if( n < 0 ) 
      {
         perror("sock_read_data().select() error!");
         return ERR_SOCKET_ERR;
      }
      else if( n == 0 )
      {
         printf("sock_read_data().select() TIMED OUT! timeout_ms=%d\n", timeout_ms);
         return ERR_TIMEOUT;
      }
      else if( n > 0 ) /* data is available */
      {
         //printf("recv() rbuf_size=%ld ...\n", rbuf_size);
         rc =  recv( fd, rbuf+bytes_read, rbuf_size, 0 );  /* read available data */
         if( rc < 0 )
         {
            perror("sock_read_data().recv() error");
            return ERR_SOCKET_ERR;
         }
         //printf("recv() rc=%d \n", rc);

         bytes_read += rc;                      /* increment bytes_read */
         rbuf_size = MAX(rbuf_size-rc, 0);      /* decrease available rbuf size */

         /* read all the requested data, return */
         if( rbuf_size < 1 )
         {
            return bytes_read;
         }
         /* if !blocking and no data, return bytes_read */
			if( (!blocking) && (rc==0) )
			   return bytes_read;
      }

   } // while(1)

   return ERR_NONE;
}

int32_t sock_read_data_old( 
   int  fd,                 /* Socket's file descriptor   */
   char * buf,              /* Recieve buffer             */
   int32_t bufsize,            /* len of buffer in bytes     */
   int  blocking,           /* specify blocking (TRUE) or non-blocking (FALSE) */
   int timeout_ms           /* number of millisecond for a timeout error */
)
{
   int32_t  total;
   int   len;
   int   status_flags;
   int   sleep_cnt;

	//printf("sock_read_data() bufsize %d blocking %d\n", bufsize, blocking);
	status_flags = 0;
	timeout_ms = MAX( timeout_ms, 50); /* 0.05 sec minimum */

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
      len = read( fd, (char *) buf, len );
      if( len < 0)
      {
		   //printf("errno is %d \n", errno);
         if( (errno==EWOULDBLOCK)  || (errno==EINTR) )
         {
            if( !(sleep_cnt--))
               return ERR_SOCKET_TIMEOUT;
            printf("sock_read_data() sleeping %d ZZZ\n", sleep_cnt);
            usleep(10000);        /* sleep for 0.01 seconds */
         }
         else
         {
            //perror( "sock_read_data() error @1"); 
            return ERR_SOCKET_ERR;
         }
      }
      else
      {
         if( len > 0  )
         {
            bufsize -= len;   /* update counter & pointers */
            buf += len;
            total += len;

				sleep_cnt = timeout_ms/10;  /* reset sleep counter when you read data  */
         }
         else                        /* if connection is terminated, read() */
         {
            //perror("sock_read_data() error@2"); 
            return ERR_SOCKET_ERR;   /* returns 0 with len = 0.             */
         }
      }
   }

   if( !blocking )                   /* restore flages */
   {
      fcntl( fd, F_SETFL, status_flags);
   }

   return total;
}

