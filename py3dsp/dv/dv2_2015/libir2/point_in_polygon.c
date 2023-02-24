/*******************************************************************
** point_in_polygon.c - Test if point is inside Polygon using 
**                      Crossings algorithm
**
** Shoot a test ray along +X axis.  The strategy, from MacMartin, is to
** compare vertex Y values to the testing point's Y and quickly discard
** edges which are entirely to one side of the test ray.
**
** If point is in polygon, returns TRUE.
**
** Originial code from the article "Point in Polygon Strategies"
** by Eric Haines, erich@eye.com
** in "Graphics Gems IV", Academic Press, 1994
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

/*------------------------------------------------------------------
** point_in_polygon.c - Test if point is inside Polygon using 
**                      Crossings algorithm
**
** Shoot a test ray along +X axis.  The strategy, from MacMartin, is to
** compare vertex Y values to the testing point's Y and quickly discard
** edges which are entirely to one side of the test ray.
**
** If point is in polygon, returns TRUE.
**
** Originial code from the article "Point in Polygon Strategies"
** by Eric Haines, erich@eye.com
** in "Graphics Gems IV", Academic Press, 1994
**-------------------------------------------------------------------
*/
int point_in_polygon(
   double   pgon[][2],    // polygon definition.
   int   numverts,        // number of polygon vertices
   double   point[2]      // test point
)
{
   register int   j, yflag0, yflag1, inside_flag, xflag0 ;
   register double ty, tx, *vtx0, *vtx1 ;

   const int X = 0;
   const int Y = 1;

   tx = point[X] ;
   ty = point[Y] ;

   vtx0 = pgon[numverts-1] ;
   /* get test bit for above/below X axis */
   yflag0 = ( vtx0[Y] >= ty ) ;
   vtx1 = pgon[0] ;

   inside_flag = 0 ;
   for ( j = numverts+1 ; --j ; )
   {
      yflag1 = ( vtx1[Y] >= ty ) ;
      /* check if endpoints straddle (are on opposite sides) of X axis
      * (i.e. the Y's differ); if so, +X ray could intersect this edge.
      */
      if ( yflag0 != yflag1 )
      {
         xflag0 = ( vtx0[X] >= tx ) ;
         /* check if endpoints are on same side of the Y axis (i.e. X's
          * are the same); if so, it's easy to test if edge hits or misses.
          */
         if ( xflag0 == ( vtx1[X] >= tx ) )
         {
            /* if edge's X values both right of the point, must hit */
            if ( xflag0 ) inside_flag = !inside_flag ;
         }
         else
         {
            /* compute intersection of pgon segment with +X ray, note
             * if >= point's X; if so, the ray hits it.
             */
            if( (vtx1[X] - (vtx1[Y]-ty)*
              ( vtx0[X]-vtx1[X])/(vtx0[Y]-vtx1[Y])) >= tx )
            {
               inside_flag = !inside_flag ;
            }
         }
      }

      /* move to next pair of vertices, retaining info as possible */
      yflag0 = yflag1 ;
      vtx0 = vtx1 ;
      vtx1 += 2 ;
   }

   return( inside_flag ) ;
}

