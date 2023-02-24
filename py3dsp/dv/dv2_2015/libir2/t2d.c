/*****************************************************************************
** t2d are functions that do 2D coordinate transformation. 
** Based on computer graphic algorithms for basic 2D transformation.
** Reference: Computer Graphic, J.Foley, Addison-Wesley
**
** Example: To transforms points from coordinate system A to system B you
** need to the relationship in terms of scale, translation, and rotation:
** 
** Tx,Ty - A(x,y) when B is at (0,0)
** Sx,Sy - ratio of B/A. B = A * scale.         
** R - rotation of A in relationship to B.
** 
** From these inputs, you can generate a forward & reverse translation matix:
** 
**   t2d_matrix A2B_m;
**   t2d_matrix B2A_m;
** 
**   t2d_forward_matrix( A2B_m, Tx, Ty, Sx, Sy, R); // make forward transformation maxtix
**   t2d_reverse_matrix( B2A_m, Tx, Ty, Sx, Sy, R); // make reverse transformation maxtix
** 
** Use these matrix to convert A->B & B->A:
** 
**   t2d_apply_to_point( &Bx, &By, A2B_m, Ax, Ay );  // convert A(x,y) to B(x,y)
** 
**   t2d_apply_to_point( &Ax, &Ay, B2A_m, Bx, By );  // convert B(x,y) to A(x,y)
**
**********************************************************************************
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

//-----------------------------------------------------
// t2d_translation - applies translation: m = m * T(dx, dy)
//
int t2d_tranlation( t2d_matrix m, double dx, double dy )
{
   t2d_matrix t;

	t2d_m_identity( t ); // translation matrix
	t[0][2] = dx;
	t[1][2] = dy;
	t2d_m_mul( m, m, t); // m = m*t
   return 0;
}

//-----------------------------------------------------
// t2d_rotation - applies rotation: m = m * R(deg)
//
int t2d_rotation( t2d_matrix m, double deg )
{
   t2d_matrix r;
   double rad;

	rad = deg * (M_PI/180.0);  /* convert degrees to radians */

	// rotation matrix 
	t2d_m_identity( r ); 
	r[0][0] = cos(rad);
	r[1][0] = sin(rad);
	r[1][1] = r[0][0];
	r[0][1] = -r[1][0];

	// m = m*r
	t2d_m_mul( m, m, r); 
   return 0;
}

//-----------------------------------------------------
// t2d_scaling - applies scaling: m = m * S(sx, sy)
//
int t2d_scaling( t2d_matrix m, double sx, double sy )
{
   t2d_matrix s;

	// scaling matrix 
	t2d_m_identity( s ); 
	s[0][0] = sx;
	s[1][1] = sy;

	// m = m*r
	t2d_m_mul( m, m, s); 
   return 0;
}

//-----------------------------------------------------------------
// t2d_forward_matrix() - form the forward transformation matrix. 
//
int t2d_forward_matrix( 
   t2d_matrix m, // returns matrix to user.
   double Tx,    // Translation X & Y
   double Ty, 
   double Sx,    //  Scaling factor
   double Sy,
   double R      //  Rotation (in degrees)
)
{
	t2d_m_identity( m );
	t2d_scaling( m, Sx, Sy );
	t2d_tranlation( m, Tx, Ty);
	t2d_rotation( m, R );
   return 0;
}

//-----------------------------------------------------------------
// t2d_reverse_matrix() - form the reverse transformation matrix. 
//
int t2d_reverse_matrix( 
   t2d_matrix m, // returns matrix to user.
   double Tx,    // Translation X & Y
   double Ty, 
   double Sx,    //  Scaling factor
   double Sy,
   double R      //  Rotation (in degrees)
)
{
   t2d_m_identity( m );
   t2d_rotation( m, -R );
   t2d_tranlation( m, -Tx, -Ty);
   t2d_scaling( m, 1.0/Sx, 1.0/Sy );
   return 0;
}

//---------------------------------------------------------------------
// t2d_apply_to_point() - applies transformation to points: [dx,dy] = m * [sx,sy]
//
int  t2d_apply_to_point( double *dx, double *dy, t2d_matrix m, double sx, double sy )
{
   t2d_vector s, d;

   s[0] = sx;   // s point as vector.
   s[1] = sy;
   s[2] = 1;

   // d = m * s
   d[0] = (m[0][0] * s[0]) + (m[0][1] * s[1]) + (m[0][2] * s[2]);
   d[1] = (m[1][0] * s[0]) + (m[1][1] * s[1]) + (m[1][2] * s[2]);
   d[2] = (m[2][0] * s[0]) + (m[2][1] * s[1]) + (m[2][2] * s[2]);

   // copy results to user
   *dx = d[0];
   *dy = d[1];
   return 0;
}

/****************************************************************************
**  t2d matrix operations -  all assume a m[3][3] matrix.
*/

//---------------------------
// t2d_m_mul. p = a * b
//
void t2d_m_mul( t2d_matrix p, t2d_matrix a, t2d_matrix b)
{
   t2d_matrix d;
   int r,c,j;

   for( r=0; r<3; r++ )
   {
      for( c=0; c<3; c++ )
      {
         d[r][c] = 0;
         for(j=0; j<3; j++ )
         {
            d[r][c] += a[r][j] * b[j][c];
         }
      }
   }
   t2d_m_copy( p, d );
}

//------------------------------
// t2d_m_copy:  dst = src
//
void t2d_m_copy(t2d_matrix dst, t2d_matrix src)
{
	memcpy(dst, src, sizeof(t2d_matrix));
}

//--------------------------------------
// t2d_m_identity:  m = Idenity_matrix
//
void t2d_m_identity(t2d_matrix m)
{
   int r, c;

   for( r=0; r<3; r++ )
      for( c=0; c<3; c++ )
      {
	      if( r==c )
	         m[r][c]	= 1;
         else
	         m[r][c]	= 0;
      }
}

//--------------------------------------------
//  t2d_m_print() - print maxtix to stdout
//
void t2d_m_print(t2d_matrix m)
{
   int r, c;
	printf("--------------\n");
   for( r=0; r<3; r++ )
	{
      for( c=0; c<3; c++ )
      {
         printf("%g ", m[r][c]);
      }
      printf("\n");
	}
	printf("--------------\n");
}

