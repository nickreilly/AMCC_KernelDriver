/***************************************************************************
**  d_round.c
****************************************************************************
*/
#define EXTERN extern


#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>
#include <math.h>

#include "ir2.h"

// should be in math.h, but compiler complains: implicit declaration of function `round'
double round(double x);

/*---------------------------------------------------------------------------
** d_round( ) - rounds the double based on resolution.
**              example d_round( 0.029, 0.01) = 0.030
**----------------------------------------------------------------------------
*/
double d_round( double d, double resolution )
{
  return round( d/resolution) * resolution;
}

