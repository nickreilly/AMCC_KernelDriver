
#define EXTERN extern

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>
 
#include "ir2.h"


/*----------------------------------------------------------
** map() - map a point from one coordinate system to another
**----------------------------------------------------------
*/
double map( double point, double pref1, double pref2, double nref1, double nref2 ) 
{
	return (double) ((point - pref1) / (pref2-pref1)) * (nref2-nref1) + nref1;
}

