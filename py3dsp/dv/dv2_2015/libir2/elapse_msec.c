#define EXTERN extern

#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include "ir2.h"


/*------------------------------------------------------
** elapse_msec() - returns the elapse milliseconds as a double
**  from start to end (timeval). int32_t has a limit of 24.885 days.
**------------------------------------------------------
*/

int32_t elapse_msec( struct timeval *start, struct timeval *end)
{
   int32_t sec, usec;
	   
	sec  = end->tv_sec - start->tv_sec;
	usec  = end->tv_usec - start->tv_usec;

	return  (sec*1000.0) + (usec/1000.0);
}  

