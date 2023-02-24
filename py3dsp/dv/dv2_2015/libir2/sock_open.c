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

/*-----------------------------------------------------------------------
**  sock_open() - create and connect to a AF_INET socket for a client. 
**     Using internet domain with stream.
**       return:  <0  unable to establish connection
**                >0 This is the socket number.
**-----------------------------------------------------------------------
*/
int sock_open(
    char * hostname,     /*  Host name          */
    int port,            /*  Port Number        */
    int type,            /* type for socket(): SOCK_STREAM, SOCK_DGRAM */
    int protocol         /* protocol for socket() system call */
    )
{
   int sock,
       error;
   struct sockaddr_in server;
   struct hostent *hp;

   hp = gethostbyname( hostname );    /* Get host address */
   if( hp == 0 )
     return -1;                       /* Unknown host */

   error = 0;
   /*
   **  Create Socket
   */
   if( (sock = socket( AF_INET, type, protocol)) < 0 )
      return  -2;
   /*
   **  Make connection to server
   */
   if( ! error )
   {
      memset((char*)&server, 0, sizeof(server));
      memcpy((char *) &server.sin_addr, (char *)hp->h_addr, hp->h_length);

      server.sin_family = AF_INET;
      server.sin_port = htons( (unsigned short)port );

      if( connect(sock, (struct sockaddr*) &server, sizeof(server) ) < 0 )
      {
         /* perror("connect"); */
         error =  -3;
      }
   }

   if( error )
   {
      close(sock);
      return error;
   }
   return sock;
}

