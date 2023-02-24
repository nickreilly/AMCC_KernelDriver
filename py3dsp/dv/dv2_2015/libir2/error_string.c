/*************************************************************
** error_string.c
**************************************************************
*/

#define EXTERN extern

/*--------------------------
*  include files
*--------------------------
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>

#include "ir2.h"

static char * Error_string[NUM_ERR_CODES]  /* Declare a string message for each */
= {
     "no error  ",
     "invalid keyword",
     "invalid range",
     "invalid format or sytax error",
     "socket communication error",
     "socket timeout error",
     "invalid path",
     "memory allocation error",
     "error on creating a file",
     "error while writing to a file",
     "error while reading a file",
     "invalid file format",
     "this is an invalid operation",
     "this option is currently restricted",
     "system error using message queues occured",
     "unable to open file",
     "Semaphore acquisiton error",
     "Reqested action or service is not available",
     "Unable to execute, object is busy",
     "Invalid data or input",
     "error or failure with IPCs",
     "error on starting forked task",
     "stream output command called with non-stream fd",
     "command causes overflow on buffer limits",
     "communication error with hardware device",
     "device did not respond as expected",
     "exceeding a set limit",
     "unable to execute due to safety restriction",
     "unable to comply at this time",
     "Invalid subarray count",
     "Invalid subarray dimension",
     "Requested Data not available",
     "Operation was stopped",
     "timeout error",
     "request or operation failed",
	  "object size not correct ",
} ;

/*----------------------------------------------------------
**  error_string() - returns a pointer to a char[] description
**     for the return code, rc. 
**----------------------------------------------------------
*/
char * error_string( int rc )
{
   char * cptr;

   rc = abs(rc);
   if( INRANGE( 0, rc, NUM_ERR_CODES-1))
      cptr =  Error_string[rc];
   else
      cptr =  "returned invalid error code";

   //printf("error_string(%d) = %s \n", rc, cptr);
   return cptr;
}
                              
