/***************************************************************************
**  my_atof.c
****************************************************************************
*/

#define EXTERN extern

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <errno.h>
#include <sys/time.h>


/*------------------------------
**  Non-standard include files
**------------------------------
*/

#include "ir2.h"

/*-------------------------------------------------------------
** my_atof() - convert a string to a double. The string s
**   must consist of  '[white space][+-] [digits.digits] [white spaces]'
**   returns error: 0 = noerror;
**                 -1 = invalid char in string
**-------------------------------------------------------------
*/

static int is_string_ok( char * s )
{
   int digit_cnt,
	    decimal_cnt;

   if( s == NULL)
	{
	   return -1;
	}

   // white space
	while( isspace( *s) ) s++;  // skip leading spaces

   // optional +/- 
   if( *s=='-' || *s=='+' ) s++; 

   // at least 1 digits with optional decimal (dd.dd)
	digit_cnt = 0;
	decimal_cnt = 0;
	while( isdigit(*s) || *s=='.' )
	{
	   s++;
		if( *s=='.' ) 
		   decimal_cnt++;
      else
		   digit_cnt++;
	}
	if( digit_cnt == 0 ) 
	{
	  return -1;
	}
	
	if( decimal_cnt >  1 ) 
	{
	  return -1;
	}

   // if 0, return with OK.
	if( *s == '\0') return 0;

   // if only whitespace left, return with OK
   if( isspace(*s) )
	{
	   while( isspace( *s) ) s++;
	  if( *s == '\0') 
		{
		  return 0;
		}
      else
		{
		  return -1;
		}
	}

   // if 'E', or 'e', .. continue, else error.
	if( *s == 'E' || *s=='e' )
	   s++;
	else
	{
	  return -1;
	}

   // optional +/-
   if( *s=='-' || *s=='+' ) s++; 

   // at least 1 digits (for exponents).
	digit_cnt = 0;
	while( isdigit(*s) )
	{
	   s++;
		digit_cnt++;
	}
	if( digit_cnt == 0 )
	{
	  return -1;
	}

   // if 0, return with OK.
	if( *s == '\0') 
	   return 0;

   // if only whitespace, return with OK
   if( isspace(*s))
	{
	   while( isspace( *s) ) s++;
	  if( *s == '\0') 
		{
		  return 0;
		}
      else
		{
		  return -1;
		}
	}

   return -1;
}

int my_atof( char *s, double *d_ptr )
{
   char *start;
   char *end;
   double d;

   //printf("my_atof: [%s]\n",s);
   // screen string for invalid charaters.
	if( is_string_ok( s ) != 0 )
	{
	   //printf("my_atof: is_string_ok .. NO\n");
	   return -1;
	}

	// skip leading spaces
	start = s;
	while( isspace( *start) ) start++;  

   // do conversion using strtod()
	//printf("calling strtod [%s]\n", s);
   errno = 0;
   d = strtod( start, &end );
	if( errno )       // no conversion
	{
	   //printf("my_atof: errno %d \n", errno);
	   return -1;
	}
	if( start==end )  // no conversion
	{
	   //printf("my_atof: start==end\n");
	   return -1;
	}
   if( isnan(d) ) return -1;
   if( isinf(d) ) return -1;
   if( d == HUGE_VAL  || d == -HUGE_VAL ) return -1;

	*d_ptr = d;
   return 0;
}

