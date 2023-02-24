
#define EXTERN extern

/*-------------------------
**  Standard include files
**-------------------------
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <math.h>
#include <sys/time.h>

/*------------------------------
**  Non-standard include files
**------------------------------
*/

#include "ir2.h"

/*--------------------------------------------------------------------
**  timeStr2Sec() - convert a string in time format "hh:mm:ss.ss"
**      to seconds (double).
**--------------------------------------------------------------------
*/
int timeStr2sec(
   double *Rsec,       /* Returns seconds to the caller */
   char   *str         /* string in time format (hh:mm:ss.ss) to convert */
   )
{
   char time[30];
   int hh, mm;
   float ss, sign;
   char * cptr;
   char * st_ptr; // for strtok_r()

   if( str == NULL )
      return ERR_INV_FORMAT;

   strxcpy( time, str, sizeof(time));

   sign = 1;    /* assume default of positive time value */

   if( NULL == (cptr = strtok_r( time, " :", &st_ptr)) )
      return( ERR_INV_FORMAT );
	if( strchr( cptr, '-') )
		sign = -1;
	hh = atoi(cptr);
	if( hh < 0 )
	{
		hh = -hh;
		sign = -1;
	}

   if( NULL == (cptr = strtok_r( (char*)NULL, " :", &st_ptr)) )
      return( ERR_INV_FORMAT );
   mm = ( cptr == NULL ? 0 : atoi(cptr) );

   if( NULL == (cptr = strtok_r( (char*)NULL, " :", &st_ptr)) )
      return( ERR_INV_FORMAT );
   ss = ( cptr == NULL ? 0 : atof(cptr) );

   *Rsec = (3600.0*hh + 60.0*mm + ss) * sign;
   return ERR_NONE;
}

/*----------------------------------------------------------------------
** sec2TimeStr() convert a double (number of seconds) into a string
**    with the format "hh:mm:ss.ss".
**    Return the the address containing the formatted string.
**----------------------------------------------------------------------
*/
char *  sec2timeStr (
   char *outbuf,        /* O: return result here: char  buf[30] */
   int  outbuf_size,    /* I: sizeof(buf) */
   double sec,          /* I: convert time in seconds to a string */
   int decimals,        /* I: number of decmial points for ss.ss */
   int show_plus        /* I: TRUE '+', FALSE ' ' */
)
{
   char buf[30];
   char fmt[30];
   int  hh, mm;
   char sign;

   if( sec < 0 )
   {
      sec = -sec;
      sign = '-';
   }
   else
   {
      sign = ( show_plus ? '+':' ');
   }

   hh = (int) floor( sec / 3600.0 );
   sec -= 3600.0 * hh;

   mm = (int) floor( sec / 60.0 );
   sec -= 60.0 * mm;

   if( decimals <= 0 )
      sprintf( buf, "%c%02d:%02d:%2.0f", sign, hh, mm, sec);
   else
   {
      sprintf( fmt, "%%c%%02d:%%02d:%%0%d.%df", decimals+3, decimals);
      sprintf( buf, fmt, sign, hh, mm, sec);
   }

   strxcpy( outbuf, buf, outbuf_size);
   return outbuf;
}

