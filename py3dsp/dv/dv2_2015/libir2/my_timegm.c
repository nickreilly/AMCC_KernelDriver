/*******************************************************************
** my_timegm.c - a reverse function unix's gmtime().
********************************************************************
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

#define MJD_1970_01_01  25567.50    /* Modified Julian Date for 1/1/70     */
static double cal_mjd( int mn, double dy, int yr );

/*----------------------------------------------------------------------
**  my_timegm() - convert the time expressed in years, yday, hr:min:sec
**                to time_t tv_sec (UNIX start representation of time,
**                which is elapse seconds from midnight Jan 1, 1970.
**----------------------------------------------------------------------
*/
int32_t my_timegm( 
   int year,   /* year since 1900   */
   int yday,   /* day of year 0-365 */
   int hr,     /* hour  0 - 23 */
   int min,    /* min   0 - 59 */
   int sec     /* sec   0 - 61 */
)
{
  static short dpm[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  int day,
      mon;
  double mjd, d;

  /* fix up dpm for leap year */
  year += 1900;
  dpm[1] = ((year%4==0 && year%100!=0) || year%400==0 ) ? 29 : 28;

  /* figure out the day & month from yday */
  mon = 0;
  day = yday+1;
  while( day > dpm[mon] )
     day -= dpm[mon++];

  d = day + (double)hr/24.0 + (double)min/1440.0 + (double)sec/86400.0;

  mjd = cal_mjd( mon+1, d, year );
  return (mjd - MJD_1970_01_01) * 86400.0 + 0.5;
}

/*----------------------------------------------------------------------
 * cal_mjd() - given a date in month, day, year, returns the modified
 *             Julian date (number of days elapsed since 1900 jan 0.5),
 *----------------------------------------------------------------------
 */
static double cal_mjd (
   int mn,     /* month of the year (1-12)       */
   double dy,  /* day of the month  (1.0 - 31.9) */
   int yr      /* year */
)
{
   int b, d, m, y;
   int32_t c;

   m = mn;
   y = (yr < 0) ? yr + 1 : yr;
   if (mn < 3) {
       m += 12;
       y -= 1;
   }

   if( (yr < 1582) || (yr == 1582 && ( (mn < 10) || (mn == 10 && dy < 15))) )
       b = 0;
   else {
       int a;
       a = y/100;
       b = 2 - a + a/4;
   }

   if (y < 0)
       c = (int32_t)((365.25*y) - 0.75) - 694025L;
   else
       c = (int32_t)(365.25*y) - 694025L;

   d = 30.6001*(m+1);

   return (b + c + d + dy - 0.5);
}


