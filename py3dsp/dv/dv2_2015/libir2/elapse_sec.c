#define EXTERN extern

#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include "ir2.h"

/*------------------------------------------------------
** elapse_sec() - returns the elapse seconds as a double
**  from start to end (timeval).
**------------------------------------------------------
*/

double elapse_sec( struct timeval *start, struct timeval *end)
{
   int32_t sec, usec;

   sec  = end->tv_sec - start->tv_sec;
   usec  = end->tv_usec - start->tv_usec;

   return  (double)sec + (usec/1000000.0);
}
