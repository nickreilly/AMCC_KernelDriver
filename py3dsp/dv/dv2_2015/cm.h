/********************************************************************************
**  CM.H - header file for color map code. 
** Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php
** Copyright (C) 2012 <NASA IRTF. http://irtfweb.ifa.hawaii.edu>  
********************************************************************************
*/ 

#ifndef __CM_H_INCLUDED
#define __CM_H_INCLUDED

/*  Static colors indexes */
#define CM_NUM_STATIC_COLORS  9
#define CM_BLACK              0
#define CM_RED                1
#define CM_GREEN              2
#define CM_BLUE               3
#define CM_GRAY               4
#define CM_CYAN               5
#define CM_MAGENTA            6
#define CM_YELLOW             7
#define CM_WHITE              8

#define CM_NUM_RW_COLORS    200
#define CM_NUM_COLORS (CM_NUM_STATIC_COLORS+CM_NUM_RW_COLORS)
#define CM_RED_INX            0     /* color index into cgraph[color][][] */
#define CM_GREEN_INX          1
#define CM_BLUE_INX           2
#define CM_X                  0     /* xy index into cgraph[][xy][] */
#define CM_Y                  1
#define CM_NUM_ELE           20     /* number of points in cgraph[][][num_ele] */

/*----------------------------------------------------------------------------
**  structures
**----------------------------------------------------------------------------
*/

/*----------------------------------------------------------------------------
** cm_t - structure to keep track of colormap information
*/
struct cm_t
{
   GdkVisual * visual;              /* pointer to display's system visual type */
   GdkColormap *colormap;           /* ponter to application's colormap */
   int max_intensity;               /* max intensity value for this visual mode */
   int swap_rgb;                    /* Tells the draw routine to swap if suing gtk_image_() stuff */

   int beg;         /* indexes to the begining and end of read/write colormap entries */
   int end;

   GdkColor colors[CM_NUM_COLORS];  /* color definition for colormap cells */

   float cgraph[3][2][CM_NUM_ELE];  /* Table to hold color map description */
   int   cgraph_num_ele[3];         /* Number of point in each color graph */

                                    /* Variable mapping cgraph to real colormap   */
  float width;                     /* width of cgraph: >0 to <3; 0.5 = full size */
   float center;                    /* center of cgraph: >0 to < 1; 0.5 = center  */
};

struct cm_colordef_t                /* define a color in RGB */
{
   float red;
   float green;
   float blue;
};

/*----------------------------------------------------------------------------
** function prototypes
**----------------------------------------------------------------------------
*/

int cm_setup( int pc_ok, int tc_ok, int v );
int cm_setup_pseudoColor( int v );
int cm_setup_trueColor( int v );
int cm_set_colormap( struct cm_t *cm );
uint32_t cm_pixel_from_rgb( int red, int green, int blue, GdkVisual * visual );
void cm_fill( int rs, int re, double ps, double pe, double vs, double ve,
   struct cm_t *cm, int color, int beg, int end );
int cm_free( void );
uint32_t cm_swap_int32( uint32_t sl, int swap_me );


#endif // #ifndef __CM_H_INCLUDED
