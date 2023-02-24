/***************************************************************************
**  dv.c - main source file for application
** Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php
** Copyright (C) 2012 <NASA IRTF. http://irtfweb.ifa.hawaii.edu>
**
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
***************************************************************************
*/

#define EXTERN
#define MAIN 1

/*-----------------------------
**  Standard include files
**-----------------------------
*/
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <getopt.h>
#include <netinet/in.h>

#include <gtk/gtk.h>

/*-----------------------------
**  Non-standard include files
**-----------------------------
*/

#include "dv.h"

/*-----------------------------------------------------------------------
**  main()
**-----------------------------------------------------------------------
*/
int main( int argc, char *argv[] )
{
   int pseudocolor_ok,
       truecolor_ok,
       opt_index, 
       rc,
       c, i;
   char *font_name;
	char buf[80];

   extern char *optarg;
   extern int optind, opterr;

	struct option long_options[] = 
	{
	   {"mga",  no_argument,    0, 1 },   // --mga - enables moris guide adjustment dialog
	   {0,      0,              0, 0 }
	};

   /* initialize globals */
   initialize_globals( argv[0] );

   /* initialize gtk & parse gtk related command line arguments. */
   gtk_init(&argc, &argv);


	/*----------------------------------------------------------
	** parse arguments options .
   */
   rc = ERR_NONE;
	font_name      = NOR_FONT_NAME;
   pseudocolor_ok = TRUE;
   truecolor_ok   = TRUE;
	Lc.mga_enable  = FALSE;
   while( (c = getopt_long( argc, argv, "PTf:mlR:C:S:p:w:h:W", long_options, &opt_index)) > 0 )
   {
      switch( c )
      {
		   case 1: 
            // --mga - set mga_enable to TRUE.
				Lc.mga_enable = TRUE;
			   break;
         case 'P':
            /* Force pseudoColor, by disabling trueColor */
            pseudocolor_ok = TRUE;
            truecolor_ok = FALSE;
            break;

         case 'T':
            /* Force trueColor, by disabling pseudoColor */
            truecolor_ok = TRUE;
            pseudocolor_ok = FALSE;
            break;

         case 'f':
            font_name = optarg;
            break;
            /* font */

         case 'm':            // medium: 2x3 dpys.
            /* medium DV */
            Lc.num_dpy = 7;
            Lc.dpy_r = 2;
            Lc.dpy_c = 3;
            break;

         case 'l':             // large: 2x4 dpys.
            /* large DV */
            Lc.num_dpy = 9;
            Lc.dpy_r   = 2;
            Lc.dpy_c   = 4;
            break;

         // R %d - number of row for dpys
         case 'R':
            i = atoi(optarg);
            if( INRANGE( 1, i, 10))
               Lc.dpy_r = i;
            break;

         // C %d - number of columns for dpys
         case 'C':
            i = atoi(optarg);
            if( INRANGE( 1, i, 10))
               Lc.dpy_c = i;
            break;

         // S %d - size of dpy in pixels. 
         case 'S':
            i = atoi(optarg);
            if( INRANGE( 100, i, 1024))
            { 
               Lc.dpy_default_wid = i;
               Lc.dpy_default_hgt = i;
            }
            break;

         case 'p':
            i = atoi(optarg);
            if( INRANGE( 30000, i, 32000))
               Lc.dvport = i;
            break;

         case 'w':
            i = atoi(optarg);
            if( INRANGE( 100, i, 1024))
               Lc.dpy_default_wid = i;
            break;

         case 'h':
            i = atoi(optarg);
            if( INRANGE( 100, i, 1024))
               Lc.dpy_default_hgt = i;
            break;

         case 'W':
            Lc.use_config_window = TRUE;
            break;

         default:
            rc = ERR_INV_OPERATION;
            break;
      }
   }


   // cal num_dpy based on number of row & col on main window + 1 (floating dpy)
   Lc.num_dpy = (Lc.dpy_r * Lc.dpy_c) + 1;
   if( Lc.num_dpy > MAX_DPY )
   {
      printf("Number of display (drawing area) exceeds MAX_DPY %d \n", MAX_DPY);
      printf("This is something that can be increased by the programmer.\n");
      /* Yeah, like make num_dpy allocate the Dpy[] and other display variables.  */
      exit(0);
   }


#if 1
   if( font_name )
      printf("     font_name=%s \n", font_name );
   printf("  truecolor_ok=%d \n", truecolor_ok );
   printf("pseudocolor_ok=%d \n", pseudocolor_ok );
   printf("    Lc.num_dpy=%d %dx%d\n", Lc.num_dpy, Lc.dpy_r, Lc.dpy_c );
   printf("    Num_buffer=%d \n", NUM_BUFFER );
   printf("    MGA_enable=%d \n", Lc.mga_enable );
#endif

   /* any command line error? Display usage and exit */
   if( rc )
   {
      usage();
      exit(0);
   }

   /*--------------------------------
   ** Initialize dflib options.
   */
   printf("Initialize dflib options\n");
   df_init();
   dfset_divbycoadd( Lc.divbydivisor );      // Lc.divbydivisor must match this setting.
   dfset_origin( DF_ORIGIN_TL );

   /*------------------------------------
	** create gui widgets
	*/
   if( cm_setup(  pseudocolor_ok, truecolor_ok, TRUE ) != ERR_NONE )
   {
      printf("Error initializing colors for application. Exiting..\n");
      exit(0);
   }

   create_base( font_name );

   /*-------------------------------------------------------
   ** Setup socket & accept Event input on file descriptor.
   */
   printf("setting up sockets\n");
   if( (Lc.sock_fd = sock_setup_server( Lc.dvport )) < 0 )
   {
      cc_printf( W.cmd.main_console, CM_RED, "WARNING: dv executing without socket suppport!\n");
   }
   else
   {
#if USE_GIO
	   Lc.sock_gio = g_io_channel_unix_new( Lc.sock_fd );
		g_io_add_watch( Lc.sock_gio, G_IO_IN, socket_accept_connection, NULL);
      printf("g_io_add_watch on sock_fd %d. Using Port %d\n", Lc.sock_fd, Lc.dvport);
#else
	Lc.sock_tag = gdk_input_add( Lc.sock_fd, GDK_INPUT_READ,
		(GdkInputFunction) socket_accept_connection, NULL );
	printf("AddInput on sock_fd %d, tag=%d. Using Port %d\n", Lc.sock_fd, Lc.sock_tag, Lc.dvport);
#endif
      sprintf( buf, "DV socket port is %d \n", Lc.dvport);
      cc_printf( W.cmd.main_console, CM_BLUE, buf );
   }

   /*--------------------------------
   ** execute the dv startup file 
   */
   printf("executing .dv-init\n");
   {
      FILE *fp;

      expand_pathname( buf, sizeof(buf), "$HOME");
      cat_pathname( buf, buf, ".dv-init", sizeof(buf));
      if( NULL != ( fp = fopen( buf, "rt")) )
      {
         while( NULL != fgets( buf, sizeof(buf), fp))
         {
            unpad( buf, '\n');
			   //printf(">>> %s\n", buf);
            cmd_execute( W.cmd.main_console, buf, FALSE );
         }
         fclose( fp );
      }
   }

   /*--------------------------------
   ** GTK main event loop
   */
   printf("Entering GTK event loop\n");
   gtk_main ();
   return 0;

}

/*-----------------------------------------------------------------------
**  initialize_globals( ) - Initialize the Dpy, Buffer, Stats, and other
**                          DV globals variables.
**-----------------------------------------------------------------------
*/
void initialize_globals( char * program_name )
{
   int i;
	char buf[256];

   /*------------------- 
   **  Lc.variables
   */
	memset( &Lc, 0, sizeof(struct lc_t) );  // initialize all globals to 0

	Lc.program_name = program_name;
   Lc.debug        = FALSE;
	Lc.timer_ms     = 1000;

   /* Initialize LC structure.  */
   if( NULL == (Lc.app_home = getenv( APP_HOME_VAR )))
      Lc.app_home = "./";
   printf("home_dir = %s\n", Lc.app_home );

	Lc.dpy_r       = 2;
	Lc.dpy_c       = 2;
	Lc.dpy_default_wid = 256;
	Lc.dpy_default_hgt = 256;
	Lc.use_config_window = FALSE;

   expand_pathname( buf, sizeof(buf), "./");
   strxcpy( Lc.path, buf, sizeof(Lc.path));
   strxcpy( Lc.dialog_path, buf, sizeof(Lc.dialog_path));
   //strxcpy( Md.path, buf, sizeof(Md.path));

   Lc.dvport             = IRTF_DV_PORT;
   Lc.divbydivisor       = TRUE;
   Lc.offset_angle       = COORD_SPEX_ANGLE;
   Lc.offset_platescale  = COORD_SPEX_PSCALE;
   Lc.image_compass_show = TRUE;
   Lc.image_compass_flipNS = FALSE;
   Lc.image_show_gbox    = TRUE;
   Lc.movie_show_delay   = 0.000;

   strxcpy( Lc.printer,     "printer",  sizeof(Lc.printer) );
   strxcpy( Lc.tcshostname, "t1", sizeof(Lc.tcshostname) );
   Lc.stack_inx = 0;
   for( i=0; i<NUM_CMD_STACK; i++)
      Lc.cmd_stack[i] = NULL;

   /*------------------- 
   **  Md.variables
   */
#if 0
   Md.fp = NULL;
   Md.fbutton_index = 0;
   Md.filename = NULL;
   for( i=0; i<NUM_FUN_BUT; i++)
   {
      Md.fb_ok[i] = FALSE;
      Md.fb_path[i][0] = 0;
      Md.fb_fname[i][0] = 0;
   }
#endif 

   /*------------------------------- 
   **  Dpy[], Buffer[], Stats[]
   */

   for(i=0; i<MAX_DPY; i++)
   {
      Dpy[i].bufinx = (i < NUM_BUFFER ? i : NUM_BUFFER-1 );

      Dpy[i].image_min = 0;
      Dpy[i].image_max = 10000;
      Dpy[i].image_zoom = 1;
      Dpy[i].image_autoscale = 1;

      Dpy[i].hist_bin = 50;
      Dpy[i].hist_auto_range = 1;

      Dpy[i].lcut_autoscale = 1;
      Dpy[i].lcut_min = 0;
      Dpy[i].lcut_max = 10000;

      Dpy[i].sa_objbin_min = 50;
      Dpy[i].sa_objbin_max = 99;
      Dpy[i].sa_skybin_min = 100;
      Dpy[i].sa_skybin_max = 149;
      Dpy[i].sa_subtractsky = FALSE;
      Dpy[i].sa_rows_per_bin = 10;
      Dpy[i].sa_xmin = 0;
      Dpy[i].sa_xmax = 255;
      Dpy[i].sa_ymin = 0;
      Dpy[i].sa_ymax = 50;
      Dpy[i].sa_yautoscale = 2;  // global

      Dpy[i].sb_showgraph = 0x03;
      Dpy[i].sb_objbin_min = 50;
      Dpy[i].sb_objbin_max = 99;
      Dpy[i].sb_skybin_min = 100;
      Dpy[i].sb_skybin_max = 149;
      Dpy[i].sb_diff_ymin = -100;
      Dpy[i].sb_diff_ymax = 500;
      Dpy[i].sb_data_ymin = 0;
      Dpy[i].sb_data_ymax = 500;
      Dpy[i].sb_xmin = 0;
      Dpy[i].sb_xmax = 255;
      Dpy[i].sb_yautoscale = 1;

      Dpy[i].noise_mode = 1;
      Dpy[i].noise_mod = 32;
      Dpy[i].noise_size = 64;
      Dpy[i].noise_graph_type = 1;
      Dpy[i].noise_autoscale = 1;
      Dpy[i].noise_area = 0;
      Dpy[i].noise_g1_min = 0;
      Dpy[i].noise_g1_max = 65000;
      Dpy[i].noise_g2_min = 0;
      Dpy[i].noise_g2_max = 500;

      Dpy[i].xcut_autoscale = 1;
      Dpy[i].xcut_min = -100;
      Dpy[i].xcut_max =  256;
      Dpy[i].xcut_xbeg = 0;
      Dpy[i].xcut_ybeg = 0;

      Dpy[i].pt_image_size = 57;
      Dpy[i].pt_show_stats = 1;
      Dpy[i].pt_show_linecut = 1;
      Dpy[i].pt_show_spex_sn = 0;
   }

   for(i=0; i<NUM_BUFFER; i++)
   {
      Buffer[i].status  = DF_EMPTY;
      Buffer[i].fheader = NULL;
      Buffer[i].fdata   = NULL;

      Stats[i].objx   = Stats[i].skyx   = 0;
      Stats[i].objy   = Stats[i].skyy   = 0;
      Stats[i].objwid = Stats[i].skywid = 1;
      Stats[i].objhgt = Stats[i].skyhgt = 1;
      Stats[i].ln_xbeg = 0;
      Stats[i].ln_ybeg = 0;
      Stats[i].ln_xend = 1;
      Stats[i].ln_yend = 1;
   }

}

/*----------------------------------------------------------------------------
**  cpu_msb() - returns TRUE if running under a CPU with MSB archaecture
**----------------------------------------------------------------------------
*/
int cpu_msb( void )
{
   return (0x001234 == htonl(0x001234));
}

/*-------------------------------------------------------------------------------
**   usage() - display usage to user.
**-------------------------------------------------------------------------------
*/
void usage( void )
{
   printf( "   usage: dv [OPTION] \n\n"
           "   options: -m       Medium DV - Provides 7 data display drawing areas. Default is 5.\n"
           "            -l       Large  DV - Provides 9 data display drawing areas.\n"
           "            -P       PseudoColor - Runs in 8 bit PseudoColor Visual Mode (default).\n"
           "            -T       TrueColor - Runs in True Color Visual Mode.\n"
           "            -p num   Port Number - Specifies the TCP/IP data port for DV.\n"
           "            -f font  Font - Sets the application font to 'font'\n"
           "            -R num   Row - number of dpys in a rows on the base window. 1 to 10.\n"
           "            -C num   Column - number of dpys in a columns the on base window. 1 to 10.\n"
           "            -S pixel Size - size of dpys in pixels (dpys are sqaure). 100 to 1024 \n\n");
}

/*--------------------------------------------------------------
** cal_box_stats_subf() - calculates STD on a box
**--------------------------------------------------------------
*/
void cal_box_stats_subf( bp, type, objx, objy, objwid, objhgt, skyx, skyy,
                         rN, rmin, rmax, rsum, rmean, rstd)
struct df_buf_t *bp;                    /* buffer pointer */
int    type;                            /* 1=box; 2=box-sky */
int    objx, objy, objwid, objhgt;      /* object box location */
int    skyx, skyy;                      /* sky    box location */
double *rN, *rmin, *rmax, *rsum, *rmean, *rstd; /* statistics returned here */
{
   int x, y,
       objinx, skyinx;
   double min, max, mean, Sdd, sum, data, N, stddev;

   Sdd = sum = N = 0;
   min = DF_MAX_SIGNED_INT32;
   max = DF_MIN_SIGNED_INT32;
   skyinx = 0;

   if( bp->status != DF_EMPTY )
      for( x = 0; x < objwid; x++)
         for( y = 0; y < objhgt; y++)
         {
            /*  Determine index to pixel for object & sky */
            if( !INRANGE(0, objx+x, bp->naxis1-1) || !INRANGE(0, objy+y, bp->naxis2-1) )
               goto Lskip;
            objinx = (objy+y) * bp->naxis1 + (objx+x);
            if( type == 2 )
            {
               if( !INRANGE(0, skyx+x, bp->naxis1-1) || !INRANGE(0, skyy+y, bp->naxis2-1) )
                  goto Lskip;
               skyinx = (skyy+y) * bp->naxis1 + (skyx+x);
            }

            /*  Determine value of obj pixel */
            data = dfdatainx(bp, objinx);

            /*  Determine value of sky pixel */
            if( type == 2 )
               data -= dfdatainx(bp, skyinx);

            if( min > data ) min = data;
            if( max < data ) max = data;
            sum += data;
            Sdd += data * data;
            N++;
   Lskip:
            objinx = 0; /* nop */
         }

   if( N < 1 )
      mean = 0;
   else
      mean = sum / N;

   if( N < 2 )
   {
      Sdd = 0;
      stddev = 0;
   }
   else
   {
      Sdd -= N * (mean*mean);
      stddev = sqrt(Sdd / (N-1));
   }

   *rmin  = min;
   *rmax  = max;
   *rN    = N;
   *rsum  = sum;
   *rmean = mean;
   *rstd  = stddev;
   return;
}

/*----------------------------------------------------------------
**  cal_box_stats( ) - Update the box stats variables for
**     the buffer Buffer[bufinx].
**----------------------------------------------------------------
*/
void cal_box_stats( bufinx )
int bufinx;    /* Index to buffer */
{  
   double N, min, max, sum, mean, stddev;

   struct df_buf_t *bp;
   struct stats_t *sptr;

   if( !INRANGE( 0, bufinx, NUM_BUFFER ))
      return;

   bp = &Buffer[bufinx];
   sptr = &Stats[bufinx];

   /* Do object             */
   cal_box_stats_subf(  bp, 1,
                        sptr->objx, sptr->objy,
                        sptr->objwid, sptr->objhgt, 
                        0, 0, &N, &min, &max, &sum, &mean, &stddev);
   sptr->objmin = min;
   sptr->objmax = max;
   sptr->objsum = sum;
   sptr->objmean= mean;
   sptr->objstd = stddev;
   sptr->objN   = N;

   /* Do sky                */
   cal_box_stats_subf(  bp, 1,
                        sptr->skyx, sptr->skyy,
                        sptr->skywid, sptr->skyhgt, 
                        0, 0, &N, &min, &max, &sum, &mean, &stddev);
   sptr->skymin = min;
   sptr->skymax = max;
   sptr->skysum = sum;
   sptr->skymean= mean;
   sptr->skystd = stddev;
   sptr->skyN   = N;
   /* Do object - sky       */
   if( sptr->objwid == sptr->skywid &&
       sptr->objhgt == sptr->skyhgt )
   {
      cal_box_stats_subf(  bp, 2,
                           sptr->objx, sptr->objy,
                           sptr->objwid, sptr->objhgt,
                           sptr->skyx, sptr->skyy,
                           &N, &min, &max, &sum, &mean, &stddev);
      sptr->redmin = min;
      sptr->redmax = max;
      sptr->redsum = sum;
      sptr->redstd = stddev;
      sptr->redN   = N;
   }
   else
   {
      sptr->redmin = 0;
      sptr->redmax = 0;
      sptr->redsum = 0;
      sptr->redstd = 0;
      sptr->redN   = 0;
   }
}

/*----------------------------------------------------------------------------
**  rotate_pt() - rotate a point around the orgin routine.
**----------------------------------------------------------------------------
*/
void rotate_pt( double *Rx, double *Ry, double x, double y, double angle )
{  
   double x1, y1, rad;

   rad = angle * (M_PI/180.0); 
   x1 = ( x * cos(rad)) + (y * sin(rad)) ;
   y1 = (-x * sin(rad)) + (y * cos(rad)) ;
   *Rx = x1;
   *Ry = y1;
}

/*------------------------------------------------------------------------
** normalize_0_360() - returns an angel between 0 & 360.
**------------------------------------------------------------------------
*/
double normalize_0_360( double angle )
{
   /* adjust angle to be in the range 0..360 */
   if( angle < 0.0 )
      angle = 360.0 - fmod( -angle, 360.0 );
   if( angle >= 360.0 )
   {
      angle = fmod( angle, 360.0 );
   }
   return angle;
}

/********************************************************************************/
/*  TCS communciation routines                                                  */
/********************************************************************************/

/*----------------------------------------------------------------------------
**  tcs3_com() - basic routine for communicating with TCS.
**----------------------------------------------------------------------------
*/
int tcs3_com(
   char * command,        /* command to send to the tcs */
   char * reply,          /* reply is copied here */
   int    reply_size,     /* size of reply buffer */
   char * tcshostname     /* hostname of tcs */
)
{
   char  buf[256];
   FILE  *fp = NULL;

   sprintf( buf, "/usr/local/bin/t3io -h %s %s  ", tcshostname, command);
   if( (fp = popen( buf, "r")) == NULL )
   {
     return ERR_NOT_AVAILABLE;
   }

   /*-----------------------------------
   ** read reply 
   */
   fgets( reply, reply_size, fp);
   pclose(fp);
   return ERR_NONE;
}

/********************************************************************************/
/*  Instrument communciation routines                                           */
/********************************************************************************/

/*----------------------------------------------------------------------------
**  bigdog_com() - basic routine for communicating with bigdog.
**----------------------------------------------------------------------------
*/
int bigdog_com(
   char * command,        /* command to send to the tcs */
   char * reply,          /* reply is copied here */
   int    reply_size,     /* size of reply buffer */
   char * hostname        /* hostname of tcs */
)
{
   char  buf[256];
   FILE  *fp = NULL;

   sprintf( buf, "/usr/local/bin/bigdogio -h %s %s  ", hostname, command);
   if( (fp = popen( buf, "r")) == NULL )
   {
     return ERR_NOT_AVAILABLE;
   }

   /*-----------------------------------
   ** read reply 
   */
   fgets( reply, reply_size, fp);
   pclose(fp);
   return ERR_NONE;
}

/*----------------------------------------------------------------------------
**  guidedog_com() - basic routine for communicating with bigdog.
**----------------------------------------------------------------------------
*/
int guidedog_com(
   char * command,        /* command to send to the tcs */
   char * reply,          /* reply is copied here */
   int    reply_size,     /* size of reply buffer */
   char * hostname        /* hostname of tcs */
)
{
   char  buf[256];
   FILE  *fp = NULL;

   sprintf( buf, "/usr/local/bin/guidedogio -h %s %s  ", hostname, command);
   if( (fp = popen( buf, "r")) == NULL )
   {
     return ERR_NOT_AVAILABLE;
   }

   /*-----------------------------------
   ** read reply 
   */
   fgets( reply, reply_size, fp);
   pclose(fp);
   return ERR_NONE;
}

/*----------------------------------------------------------------------------
**  moris_com() - basic routine for communicating with MORIS.
**----------------------------------------------------------------------------
*/
int moris_com(
   char * command,        /* command to send to the tcs */
   char * reply,          /* reply is copied here */
   int    reply_size,     /* size of reply buffer */
   char * hostname        /* hostname of tcs */
)
{
   char  buf[256];
   FILE  *fp = NULL;

   sprintf( buf, "/usr/local/bin/morisio -h %s %s  ", hostname, command);
   if( (fp = popen( buf, "r")) == NULL )
   {
     return ERR_NOT_AVAILABLE;
   }

   /*-----------------------------------
   ** read reply 
   */
   fgets( reply, reply_size, fp);
   pclose(fp);
   return ERR_NONE;
}

/*----------------------------------------------------------------------------
**  smokey_com() - basic routine for communicating with smokey . 
**----------------------------------------------------------------------------
*/
int smokey_com(
   char * command,        /* command to send to the tcs */
   char * reply,          /* reply is copied here */
   int    reply_size,     /* size of reply buffer */
   char * hostname        /* hostname of tcs */
)
{
   char  buf[256];
   FILE  *fp = NULL;

   sprintf( buf, "/usr/local/bin/smokeyio -h %s %s  ", hostname, command);
   if( (fp = popen( buf, "r")) == NULL )
   {
     return ERR_NOT_AVAILABLE;
   }

   /*-----------------------------------
   ** read reply 
   */
   fgets( reply, reply_size, fp);
   pclose(fp);
   return ERR_NONE;
}

/*----------------------------------------------------------------------------
**  spex_fix_subarray_dim() -  This function takes a subarray dimension and
**     fixes it so that:
**       - the q2, q3, q4 (on a 1024x1024) device spection are specific in q1
**         (so that the mirroring will include these pixels)
**       - adjust x,y,w,h it fits the limitation of the readout and includes
**         all the pixels.
**----------------------------------------------------------------------------
*/
int spex_fix_subarray_dim( int *ax, int *ay, int *awid, int *ahgt)
{   
    int x, y, wid, hgt;
    int ts, te, s, e, w;

    /* copy parameters from caller */
    x = *ax; y = *ay; wid = *awid; hgt = *ahgt;

#if DEBUG
printf("spex_fix_subarray_dim( ):\n" );
printf("-------------------------\n" );
printf("  before: %d,%d  %dx%d \n", x, y, wid, hgt);
#endif
    /*----------------------------------------
    ** do x axis
    */
    if( (x > 512) || (x+wid > 512) )
    {
      // printf("adjusting X axis. \n" );
      /* get x,wid in 2 quad */
      ts = MAX(512, x);
      te = (x+wid-1);
      // printf("  ts=%d te=%d\n", ts, te );

      /* transpose ts, te to start/end points in 1st quad */
      s = 1023 - te;    /* start pixel */
      e = 1023 - ts;    /* end pixel */
      w = e-s+1;        /* width */
      // printf("  s=%d e=%d w=%d \n", s, e, w );

      /* Adust x,wid to include these points */
      x = MIN( x, s);
      wid = MAX( wid, w);
      wid = MIN( 512-x, wid);
      // printf("  adjusted x=%d wid=%d\n", x, wid );
   }

   /*----------------------------------------
   ** make adjustment for readout boundaries.
   */

   /* x should be on 16 pixel boundary */
   s = x % 16;
   x -= s;
   wid += s;
   // printf("  adj. bod. x=%d wid=%d\n", x, wid );

   /* wid should on 16 pixel boundary, but 32 or greater */
   s    = 16 - (wid % 16);
   wid += s;
   wid = MIN( 512-x, wid);
   // printf("  adj. bod. x=%d wid=%d\n", x, wid );

   /*----------------------------------------
   ** do y axis
   */
   if( (y > 512) || (y+hgt > 512) )
   {
      // printf("adjusting Y axis. \n" );
      /* get x,wid in 2 quad */
      ts = MAX(512, y);
      te = (y+hgt-1);
      // printf("  ts=%d te=%d\n", ts, te );

      /* transpose ts, te to start/end points in 1st quad */
      s = 1023 - te;    /* start pixel */
      e = 1023 - ts;    /* end pixel */
      w = e-s+1;        /* width */
      // printf("  s=%d e=%d w=%d \n", s, e, w );

      /* Adust y, hgt to include these points */
      y = MIN( y, s);
      hgt = MAX( hgt, w);
      hgt = MIN( 512-y, hgt);
      // printf("  adjusted y=%d hgt=%d\n", y, hgt );
   }

   /*----------------------------------------
   ** make adjustment for readout boundaries.
   */

   /* y should be on 4 pixel boundary */
   s = y % 4;
   y -= s;
   hgt += s;
   // printf("  adj. bod. y=%d hgt=%d\n", y, hgt );

   /* hgt should on 4 pixel boundary, but 4 or greater */
   s    = 4 - (wid % 4);
   hgt += s;
   hgt = MIN( 512-y, hgt);
   // printf("  adj. bod. y=%d hgt=%d\n", y, hgt );

   /* copy results to caller */
   *ax = x; *ay = y; *awid = wid; *ahgt = hgt;
   // printf("  after: %d,%d  %dx%d \n", x, y, wid, hgt);
   return ERR_NONE;
}


/*------------------------------------------------------------------------------
** PointInPolygon()  - Test if point is inside Polygon using Crossings algorithm
**
** Shoot a test ray along +X axis.  The strategy, from MacMartin, is to
** compare vertex Y values to the testing point's Y and quickly discard
** edges which are entirely to one side of the test ray.
** 
** Originial code from the article "Point in Polygon Strategies"
** by Eric Haines, erich@eye.com
** in "Graphics Gems IV", Academic Press, 1994
**------------------------------------------------------------------------------
*/ 
int PointInPolygon(
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

#if (GTK_MAJOR_VERSION == 2) && (GTK_MINOR_VERSION < 12)
/*-------------------------------------------------------------------------
**  void gtk_widget_set_tooltip_text( GtkWidget *widget, const gchar * text )
**  similiar to tooltip API in gtk 2.11.3. Supported by GTK in 2.12
**-------------------------------------------------------------------------
*/
void gtk_widget_set_tooltip_text( GtkWidget *widget, const gchar * text )
{
   GtkTooltips * button_bar_tips = gtk_tooltips_new();

   gtk_tooltips_set_tip( GTK_TOOLTIPS (button_bar_tips), widget, text, NULL);
}
#endif

/****************************************************************************/
/****************************************************************************/


