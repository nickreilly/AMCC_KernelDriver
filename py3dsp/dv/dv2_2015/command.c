/*******************************************************************************
**   command.c - Command processer for application.
**
** Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php
** Copyright (C) 2012 <NASA IRTF. http://irtfweb.ifa.hawaii.edu>
********************************************************************************
*/

#define EXTERN extern
#define DEBUG 0

/*-------------------------
**  Standard include files
**-------------------------
*/
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <netinet/in.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include <mqueue.h>
#include <time.h>

#include <gtk/gtk.h>

/*------------------------------
**  Non-standard include files
**------------------------------
*/
#include "dv.h"

/*------------------------------------
**  Table of commands & ID numbers.
*/
struct PARSE_TABLE
{
  char * kw;                                /* The opcode string  */
  int (* pfi)( int index, char * par);      /* pfi is pointer to function returning an integer */
};

struct PARSE_TABLE parse_table[] =
{

   { "Active",                do_active                  },
   { "aofig.data",            do_aofig_data              },
   { "aofig.format",          do_aofig_format            },
   { "aofig.xy",              do_aofig_XY                },
   { "BoxCentroid",           do_boxcentroid             },
   { "BoxCopy",               do_boxcopy                 },
   { "BoxPeak",               do_boxpeak                 },
   { "BoxScale",              do_boxscale                },
   { "BoxZoom",               do_boxzoom                 },
   { "Buffer",                do_buffer                  },
   { "BufInfo",               do_bufinfo                 },
   { "Clear",                 do_clear                   },
   { "CM.center",             do_cmCenter                },
   { "CM.width",              do_cmWidth                 },
   { "CM.Swap.Longs",         do_cm_swap_rgb             },
   { "Colormap",              do_colormap                },
   { "Colormap.Inverse",      do_colormap_inverse        },
   { "Copy",                  do_copy                    },
   { "DisplayType",           do_displaytype             },
   { "DivByDivisor",          do_divbydivisor            },
   { "DrawBox",               do_drawbox                 },
   { "DrawLine",              do_drawline                },
   { "Echo",                  do_echo                    },
   { "Filter1",               do_filter1,                },
   { "Filter2",               do_filter2,                },
   { "FullImage",             do_fullimage               },
   { "Gbox.Cmd.Right.Click",  do_gbox_cmd_right_click    },
   { "HistArea",              do_histarea                },
   { "HistAutoRange",         do_hist_auto_range         },
   { "HistBin",               do_histbin                 },
   { "ImageAutoScale",        do_imageautoscale          },
   { "ImageCompass",          do_imagecompass_show       },
   { "ImageCompass.flipNS",   do_imagecompass_flipNS     },
   { "ImageRange",            do_imagerange              },
   { "ImageScale1P",          do_imagescale1p            },
   { "ImageShowGBox",         do_imageshowgbox           },
   { "ImageZoom",             do_imagezoom               },
   { "Inst.flavor",           do_inst_flavor             },
   { "Inst.Com",              do_inst_com                },
   { "LCutArea",              do_lcutarea                },
   { "LCutAutoScale",         do_lcutautoscale           },
   { "LCutRange",             do_lcutrange               },
   { "LCutXY",                do_lcutxy                  },
   { "Move",                  do_move                    },
   { "MovieShow",             do_movieshow               },
   { "MovieShowDelay",        do_movieshowdelay          },
   { "M.Execute",             do_m_execute               },
   { "M.FileMask",            do_m_filemask              },
   { "M.Load",                do_m_load                  },
   { "M.Path",                do_m_path                  },
   { "M.Refresh",             do_m_refresh               },
   { "M.Stop",                do_m_stop                  },
   { "NoiseArea",             do_noisearea               },
   { "NoiseAutoScale",        do_noiseautoscale          },
   { "NoiseSize",             do_noise_size              },
   { "NoiseG1Range",          do_noiseg1range            },
   { "NoiseG2Range",          do_noiseg2range            },
   { "NoiseGraphType",        do_noisegraphtype          },
   { "NoiseMod",              do_noisemod                },
   { "NoiseMode",             do_noisemode               },
   { "Offset.Angle",          do_offset_angle            },
   { "Offset.BegXY",          do_offset_begxy            },
   { "Offset.EndXY",          do_offset_endxy            },
   { "Offset.Platescale",     do_offset_platescale       },
   { "Offset.TCS",            do_offset_tcs,             },
   { "OffsetAB.TCS",          do_offset_tcsab,           },
   { "Path",                  do_path                    },
   { "Print",                 do_print                   },
   { "Printer",               do_printer                 },
   { "PrinterType",           do_printertype             },
   { "PrintToFile",           do_printtofile             },
   { "PtImageSize",           do_ptimagesize             },
   { "PtShowStats",           do_ptshowstats             },
   { "PtShowLinecut",         do_ptshowlinecut           },
   { "PtShowSpexSN",          do_ptshowspex_sn           },
   { "Push",                  do_push                    },
   { "Quit",                  do_quit                    },
   { "Read",                  do_read                    },
   { "ReadXtension",          do_read_xtension           },
   { "ReadFile",              do_readfile                },
   { "ReadMovie",             do_readmovie               },
   { "ReadSock",              do_readsock                },
   { "ReadTextArray",         do_readtextarray           },
   { "Rotate",                do_rotate                  },
   { "SARowsPerBin",          do_sarowsperbin            },
   { "SAObjBin",              do_saobjbin                },
   { "SASkyBin",              do_saskybin                },
   { "SAShift",               do_sashift                 },
   { "SAStats",               do_sastats                 },
   { "SASubtractSky",         do_sasubtractsky           },
   { "SAXScale",              do_saxscale                },
   { "SAYAutoscale",          do_sayautoscale            },
   { "SAYScale",              do_sayscale                },
   { "SBDataYRange",          do_sbdatayrange            },
   { "SBDiffYRange",          do_sbdiffyrange            },
   { "SBObjBin",              do_sbobjbin                },
   { "SBSkyBin",              do_sbskybin                },
   { "SBShow",                do_sbshow                  },
   { "SBYAutoscale",          do_sbyautoscale            },
   { "SBXScale",              do_sbxscale                },
   { "SetTimer",              do_settimer                },
   { "Save",                  do_save                    },
   { "SaveFile",              do_savefile                },
   { "Smooth",                do_smooth                  },
   { "Sqrt",                  do_sqrt                    },
   { "StatsFixedWH",          do_statsfixedwh            },
   { "StatsObjBox",           do_statsobjbox             },
   { "StatsSetSky",           do_statssetsky             },
   { "StatsXORLine",          do_statsxorline            },
   { "Test",                  do_test                    },
   { "TCSHostname",           do_tcshostname             },
   { "UseFitsAngle&Scale",    do_usefitsanglescale       },
   { "usehex",                do_usehex                  },
   { "XCutAutoScale",         do_xcutautoscale           },
   { "XCutRange",             do_xcutrange               },
   { "XCutSet",               do_xcutset                 },

   { "X",                     do_x                       },
   { "t.gd.info",             do_t_gd_info               },
};

int num_parse_item = { sizeof(parse_table) / sizeof(struct PARSE_TABLE) };

/*-------------------------------------------------------------
** cmd_execute() - Executes a local mcc command line instruction
**-------------------------------------------------------------
*/

int cmd_execute ( cc_console *cc, char * cmd_buf, int feedback )
{
   char copy_buf[120];
   char replybuf[120];
	char feedback_buf[256];
   int error,
      found,
      i;

   char * kw, *str_ptr,
        * par;

	strcpy( replybuf, "");
   /*-------------------------------------------------
   **  Search table for keyword
   */
   strxcpy( copy_buf, cmd_buf, sizeof(copy_buf));

   // exit if empty command line - next strtok_r can die otherwise
   if( (kw  = strtok_r(copy_buf, " ", &str_ptr)) == NULL )
       return( ERR_NONE );

   par = strtok_r((char*)NULL, "", &str_ptr);
   unpad( par, ' ' );

   if( kw == NULL )
      return( ERR_INV_KW);
   if( *kw == '#' )          /* line starting with # are comments */
		return( ERR_NONE );

   /*-------------------------------------------------
   **  Search table for keyword. If found call function.
   */
   error = ERR_INV_KW;
   for(i=0, found = FALSE; i<num_parse_item && found == FALSE; )
   {
     if( !stricmp( kw, parse_table[i].kw ) )
     {
        found = TRUE;
        error = (parse_table[i].pfi)( i, par);  /* Call function(i, par) */
     }
     else
		i++;
   }

   /*-------------------------------------------------
	**  if (!found ), try the math routines
	*/
   if( !found )
   {
      strxcpy( copy_buf, cmd_buf, sizeof(copy_buf));
      math_fixstring( copy_buf, sizeof(copy_buf));
      error = do_math( 1, copy_buf );
   }

   /*-------------------------------------------------
	**  Update widgets/displays 
	*/
	update_display_widgets( );

   if( feedback )
   {
	   int c;

	   // Feeback goes to both commandIO and Feedback_Label
      if( error )
		{
         sprintf( feedback_buf, "ERROR [%s] %s\n", cmd_buf, error_string(abs(error)));
			c = CM_RED;
		}
      else
		{
         sprintf( feedback_buf, "%s\n", cmd_buf);
			c = CM_BLACK;
		}
		cc_printf( cc, c, feedback_buf);
		gtk_widget_modify_fg( GTK_WIDGET(W.command_feedback), GTK_STATE_NORMAL, &CM.colors[c]);
		gtk_label_set_text( GTK_LABEL(W.command_feedback), feedback_buf);

      if( strlen(replybuf) > 0 )
         cc_printf( cc, CM_BLUE, "%s\n", replybuf);
   }

   return( error );
}



/********************************************************************************/
/*  APPLICATION COMMANDS                                                        */
/********************************************************************************/

/*--------------------------------------------------------------------------------
** do_KEYWORK - The following routines performs the processing of
**              the command KEYWORD.
**--------------------------------------------------------------------------------
*/

/*--------------------------------------------------------------------------------
**  do_active() - Set the Active display.
**     Syntax: active dpinx
**--------------------------------------------------------------------------------
*/
int do_active( int index, char * par )
{
   int  dpinx,
        old,
        rc;
    char * str_ptr;

   if( par == NULL )
      return ERR_INV_FORMAT;

   if( (rc = parseIntR_r( &dpinx, par, " ", &str_ptr, 0, Lc.num_dpy-1 )) != ERR_NONE)
      return rc;

   if( Lc.active != dpinx )
   {
      old = Lc.active;
      Lc.active = dpinx;
      call_dpytitle_redraw( old );         /* redraw old (normal background) */
      call_dpytitle_redraw( Lc.active );   /* redraw active (highlighted) */
   }

   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_aofig_data() - Tells us the organization of the data set in the fit
**     file for an aofig.
**     Syntax: AOFig.Data { X | Y } [dpinx]
**--------------------------------------------------------------------------------
*/
int do_aofig_data( int index, char * par )
{
   int rc, data, dpinx;
	char * str_ptr;

   /* Get area */
   if( par == NULL )
      return ERR_INV_FORMAT;

   if( (data = parseSelection_r( par, " ", &str_ptr, xy_selection)) < 0 )
      return data;

   /* Get dpinx */
   if( (rc = parseIntR_r( &dpinx, NULL, " ", &str_ptr, 0, Lc.num_dpy-1 )) != ERR_NONE)
      dpinx = Lc.active;

   /* update */
   if( Dpy[dpinx].aofig_data != data )
   {
      Dpy[dpinx].aofig_data = data;

      if( Dpy[dpinx].dpytype == DPYTYPE_AOFIG )
         call_dpydata_redraw( dpinx );
   }
   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_aofig_format() - Tells dv how to display the ao data: text, DM, or Sensor.
**     Syntax: AOFig.Format { text | DM | Sensor } [dpinx]
**--------------------------------------------------------------------------------
*/
int do_aofig_format( int index, char * par )
{
   int rc, format, dpinx;
	char * str_ptr;

   /* Get area */
   if( par == NULL )
      return ERR_INV_FORMAT;

   if( (format = parseSelection_r( par, " ", &str_ptr, aofig_format_selection )) < 0 )
      return format;

   /* Get dpinx */
   if( (rc = parseIntR_r( &dpinx, NULL, " ", &str_ptr, 0, Lc.num_dpy-1)) != ERR_NONE)
      dpinx = Lc.active;

   /* update */
   if( Dpy[dpinx].aofig_format != format )
   {
      Dpy[dpinx].aofig_format = format;

      if( Dpy[dpinx].dpytype == DPYTYPE_AOFIG )
         call_dpydata_redraw( dpinx );
   }
   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_aofig_XY() - Identifies the 1st data item.
**     Syntax:  AOFig.XY  x y [dpinx]
**--------------------------------------------------------------------------------
*/
int do_aofig_XY( index, par )
   int index;
   char * par;
{
   int dpinx,
       rc;
   int x, y;
	char * str_ptr;

   if( par == NULL )
      return ERR_INV_FORMAT;

   /* Get max & min */
   if( (rc = parseIntR_r( &x, par, " ", &str_ptr, 0, NUM_PIXEL-1)) != ERR_NONE)
      return rc;
   if( (rc = parseIntR_r( &y, NULL, " ", &str_ptr, 0, NUM_PIXEL-1)) != ERR_NONE)
      return rc;

   /* Get dpinx */
   if( (rc = parseIntR_r( &dpinx, NULL, " ", &str_ptr, 0, Lc.num_dpy-1)) != ERR_NONE)
      dpinx = Lc.active;

   /* update */
   if( Dpy[dpinx].aofig_x != x || Dpy[dpinx].aofig_y != y )
   {
      Dpy[dpinx].aofig_x = x;
      Dpy[dpinx].aofig_y = y;
      if( Dpy[dpinx].dpytype==DPYTYPE_AOFIG ) 
         call_dpydata_redraw( dpinx );
   }
   return ERR_NONE;
}


/*--------------------------------------------------------------------------------
**  do_boxcentroid() - Finds and Identifies the a centroid pixel in the objbox.
**     Syntax: ObjBoxCentroid dpinx
**--------------------------------------------------------------------------------
*/
int do_boxcentroid( int index, char * par )
{
   int  dpinx,
        x, y, wid, hgt,
        px, py;
   float cx, cy,
         pixel_wid;
   double sum_dx,
          sum_dy,
          sum_d,
          d;

   struct dpy_t *dp;
   struct df_buf_t *bp;
	char * str_ptr;

   /* get display index */
   if( (par == NULL ) || 
	    (parseIntR_r( &dpinx, par, " ", &str_ptr, 0, Lc.num_dpy-1) != ERR_NONE) )
      dpinx = Lc.active;
   dp = &Dpy[dpinx];
   bp = &Buffer[dp->bufinx];

   /* check status */
   if( bp->status == DF_EMPTY )
      return ERR_NO_DATA;

   /* get dimension of array and ckeck it is inside data array */
   x   = Stats[dp->bufinx].objx;
   y   = Stats[dp->bufinx].objy;
   wid = Stats[dp->bufinx].objwid;
   hgt = Stats[dp->bufinx].objhgt;
   if( ((x+wid-1) > bp->naxis1) || ((y+hgt-1) > bp->naxis2) )
      return ERR_INV_RNG;

   /*-------------------------------
   ** loop to find centroid
   */
   sum_dx = sum_dy = sum_d = 0;

   for( py = 0; py < hgt; py++ )
      for( px = 0; px < wid; px++ )
      {
         d = dfdataxy( bp, x+px, y+py);
         sum_d += d;
         sum_dx += d * (px+1);
         sum_dy += d * (py+1);
      }

   cx = x + (sum_dx / sum_d) - 1.0;
   cy = y + (sum_dy / sum_d) - 1.0;

   /* print results to feedback */
	cc_printf( W.cmd.main_console, CM_BLUE, "BoxCentroid is at (%3.1f,%3.1f)\n", cx, cy);

   /*-----------------------------------------------------------------
   ** ID pixel on image display by drawing a box around the peak pixel
   */
   pixel_wid = dp->image_zoom > 0 ? dp->image_zoom : 1.0/(1.0+abs(dp->image_zoom));

   px = ((cx - dp->image_offx) * pixel_wid) + (0.5*pixel_wid);
   py = ((cy - dp->image_offy) * pixel_wid) + (0.5*pixel_wid);

   gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_BLUE] );
   gdk_draw_line( dp->data_drawingarea->window, W.nor_gc,  px-pixel_wid, py, px+pixel_wid, py);
   gdk_draw_line( dp->data_drawingarea->window, W.nor_gc,  px, py-pixel_wid, px, py+pixel_wid);

#if USE_PANGO
   pango_layout_set_text ( W.fixed_font.layout, "C", -1);
	gdk_draw_layout (dp->data_drawingarea->window, W.nor_gc, px+pixel_wid+2, py+pixel_wid, W.fixed_font.layout);
#else
   gdk_draw_text( dp->data_drawingarea->window,  W.fixed_font, W.nor_gc,
      px+pixel_wid+2, py+pixel_wid, "C", 1);
#endif

   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_boxcopy() - Object Box Copy - copies the object box
**       subarray into another buffer. The destination objbox
**       is adjust to include all the pixels.
**     Syntax: BoxCopy src to dest
**--------------------------------------------------------------------------------
*/
int do_boxcopy( int index, char * par )
{
   int  dest, src, rc;
	char * str_ptr;

   if( par == NULL )
      return ERR_INV_FORMAT;

   /* src */
   if( (src = parseSelection_r( par, " ", &str_ptr, buffer_selection)) < 0 )
      return src;

   /* to */
   if( (rc = parseSelection_r( NULL, " ", &str_ptr, to_selection)) < 0 )
      return rc;

   /* dest */
   if( (dest = parseSelection_r( NULL, " ", &str_ptr, buffer_selection)) < 0 )
      return dest;

   // printf("boxcopy (%d,%d) %dx%d from %d to %d\n",
   // Stats[src].objx, Stats[src].objy, Stats[src].objwid, Stats[src].objhgt, src, dest );

   rc = df_copy_subarray( &Buffer[dest], &Buffer[src],
      Stats[src].objx, Stats[src].objy, Stats[src].objwid, Stats[src].objhgt);

   /* redimension objbox so that it include all pixels */
   if( rc == ERR_NONE )
   {
      Stats[dest].objx = 0;
      Stats[dest].objy = 0;
      Stats[dest].objwid = Stats[src].objwid;
      Stats[dest].objhgt = Stats[src].objhgt;
   }

   /* redraw destination frame */
   cal_box_stats( dest );
   redraw_dpydata_for_bufinx( dest );
   auto_update_hist_ref_bufinx( dest );  // update histo autoscale
   update_file_save_dialog( dest );
   return rc;
}

/*--------------------------------------------------------------------------------
**  do_boxpeak() - Finds and Identifies the peak.
**     Syntax: ObjBoxPeak [dpinx]
**--------------------------------------------------------------------------------
*/
int do_boxpeak( int index, char * par )
{
   int  dpinx,
        x, y, wid, hgt,
        px, py,
        peakx, peaky;
   double max, d;
   float pixel_wid;
   struct dpy_t *dp;
   struct df_buf_t *bp;
	char * str_ptr;

   /* get display index */
   if( parseIntR_r( &dpinx, par, " ", &str_ptr, 0, Lc.num_dpy-1) != ERR_NONE)
      dpinx = Lc.active;
   dp = &Dpy[dpinx];
   bp = &Buffer[dp->bufinx];

   /* check status */
   if( bp->status == DF_EMPTY )
      return ERR_NO_DATA;

   /* get dimension of array and ckeck it is inside data array */
   x   = Stats[dp->bufinx].objx;
   y   = Stats[dp->bufinx].objy;
   wid = Stats[dp->bufinx].objwid;
   hgt = Stats[dp->bufinx].objhgt;
   if( ((x+wid-1) > bp->naxis1) || ((y+hgt-1) > bp->naxis2) )
      return ERR_INV_RNG;

   /*-------------------------------
   ** loop to find peak
   */
   peakx = x;
   peaky = y;
   max   = dfdataxy( bp, x, y);
   for( py = y; py < (y+hgt); py++ )
      for( px = x; px < (x+wid); px++ )
      {
         d = dfdataxy( bp, px, py);
         if( d > max )
         {
            peakx = px;
            peaky = py;
            max   = d;
         }
      }

   /* print results to feedback */
	cc_printf( W.cmd.main_console, CM_BLUE, "BoxPeak is at (%d,%d) value=%3.1f\n", peakx, peaky, max);

   /*-----------------------------------------------------------------
   ** ID pixel on image display by drawing a box around the peak pixel
   */
   pixel_wid = dp->image_zoom > 0 ? dp->image_zoom : 1.0/(1.0+abs(dp->image_zoom));

   px = ((peakx - dp->image_offx) * pixel_wid) + 0.5;
   py = ((peaky - dp->image_offy) * pixel_wid) + 0.5;
   if( pixel_wid < 1 )
      pixel_wid = 1;  /* can't ID subpixels, so just use wid & hgt of 1 */

   gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_GREEN] );
   gdk_draw_rectangle( dp->data_drawingarea->window, W.nor_gc, FALSE, px, py, pixel_wid, pixel_wid);
#if USE_PANGO
   pango_layout_set_text ( W.fixed_font.layout, "P", -1);
	gdk_draw_layout (dp->data_drawingarea->window, W.nor_gc, px+pixel_wid+2, py+pixel_wid, W.fixed_font.layout);
#else
   gdk_draw_text( dp->data_drawingarea->window,  W.fixed_font, W.nor_gc, 
	   px+pixel_wid+2, py+pixel_wid, "P", 1);
#endif

   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_boxscale() - Sets autoscale to  Fixed and Scales the range to the pixels in the box.
**     Syntax: boxscale [dpinx]
**--------------------------------------------------------------------------------
*/
int do_boxscale( index, par )
   int index;
   char * par;
{
   int dpinx,
       bufinx;
   float min, max;
	char * str_ptr;

   struct stats_t *sp;

   /* get display index */
   if( parseIntR_r( &dpinx, par, " ", &str_ptr, 0, Lc.num_dpy-1) != ERR_NONE)
      dpinx = Lc.active;
   bufinx = Dpy[dpinx].bufinx;

   if( Buffer[bufinx].status == DF_EMPTY )
      return ERR_NO_DATA;
   sp = &Stats[bufinx];

   /*--------------------------------------------------------
   ** Set image range based on box statitics.
   */

   /* do simple autoscale */
   min = sp->objmean - 3*sp->objstd;
   max = sp->objmean + 3*sp->objstd;

   /* Narrow min/max based on data's Min & Max values */
   if( min < sp->objmin )
      min = sp->objmin;
   if( max > sp->objmax )
      max = sp->objmax;

   if( (max - min) < 1 )
      max = min + 1;

   Dpy[dpinx].image_min = min;
   Dpy[dpinx].image_max = max;
   Dpy[dpinx].image_autoscale = FALSE;

   /*-----------------------------------------------
   ** Redraw the canvas
   */

   if( (Dpy[dpinx].dpytype == DPYTYPE_IMAGE ) ||  (Dpy[dpinx].dpytype == DPYTYPE_HISTOGRAM ))
      call_dpydata_redraw( dpinx );

   // redraw histogram
   auto_update_hist_ref_dpinx( dpinx );

   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_boxzoom() - Zooms the active canvas on its object box.
**     Syntax: boxzoom [dpinx]
**--------------------------------------------------------------------------------
*/
int do_boxzoom( index, par )
   int index;
   char * par;
{
   int dpinx,
       bufinx,
       boxwid, boxhgt,
       fat,
       zoom,
       zoom1, zoom2,
       da_wid, da_hgt;

   float pixel_wid;
	char * str_ptr;

   /* get display index */
   if( parseIntR_r( &dpinx, par, " ", &str_ptr, 0, Lc.num_dpy-1) != ERR_NONE)
      dpinx = Lc.active;
   bufinx = Dpy[dpinx].bufinx;

   if( Buffer[bufinx].status == DF_EMPTY )
      return ERR_NO_DATA;

   /* get size of the drawing area */
   if( Dpy[dpinx].data_drawingarea->window == NULL )
      return ERR_NONE;
   gdk_drawable_get_size( Dpy[dpinx].data_drawingarea->window, &da_wid, &da_hgt);

   /*--------------------------------------------------------
   ** Find biggest zoom to fit the inside box inside the canvas
   */
   boxwid = Stats[bufinx].objwid + 2;
   boxhgt = Stats[bufinx].objhgt + 2;

   if ( da_wid > boxwid )
      zoom1 = da_wid / boxwid;
   else
      zoom1 = -(boxwid / da_wid);

   if ( da_hgt > boxhgt )
      zoom2 = da_hgt / boxhgt;
   else
      zoom2 = -(boxhgt / da_hgt);

   zoom = MIN( zoom1, zoom2 );
   zoom = (zoom > 20 ? 20 : zoom);
   zoom = (zoom < -8 ? -8 : zoom);
   Dpy[dpinx].image_zoom = zoom;

   /*-----------------------------------------------
   ** Try to center the box in the frame
   */
   pixel_wid = zoom > 0 ? zoom : 1.0/(1.0+abs(zoom));

#if 0
   if( (fat = ( da_wid - Stats[bufinx].objwid * pixel_wid)) > 0 )
      Dpy[dpinx].image_offx = Stats[bufinx].objx - (fat/2)/pixel_wid;
   else
      Dpy[dpinx].image_offx = Stats[bufinx].objx;

   if( (fat = ( da_hgt - Stats[bufinx].objhgt * pixel_wid)) > 0 )
      Dpy[dpinx].image_offy = Stats[bufinx].objy - (fat/2)/pixel_wid;
   else
      Dpy[dpinx].image_offy = Stats[bufinx].objy;
#endif
   fat = da_wid - Stats[bufinx].objwid*pixel_wid;
   Dpy[dpinx].image_offx = Stats[bufinx].objx - (fat/2)/pixel_wid;
	if( Dpy[dpinx].image_offx < 0 )
      Dpy[dpinx].image_offx = Stats[bufinx].objx;

   fat = da_hgt - Stats[bufinx].objhgt*pixel_wid;
	Dpy[dpinx].image_offy = Stats[bufinx].objy - (fat/2)/pixel_wid;
	if( Dpy[dpinx].image_offy  < 0 )
      Dpy[dpinx].image_offy = Stats[bufinx].objy;

   /* Redraw the canvas */
   if( Dpy[dpinx].dpytype == DPYTYPE_IMAGE )
      call_dpydata_redraw( dpinx );

   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_buffer() - Set the buffer in a display.
**     Syntax: buffer bufid dpinx
**--------------------------------------------------------------------------------
*/
int do_buffer( int index, char * par )
{
   int  bufinx,
        dpinx,
        rc;
	char * str_ptr;

   if( (bufinx = parseSelection_r( par, " ", &str_ptr, buffer_selection)) < 0 )
      return bufinx;
   if( (rc = parseIntR_r( &dpinx, NULL, " ", &str_ptr, 0, Lc.num_dpy-1)) != ERR_NONE)
      dpinx = Lc.active;

   if( Dpy[dpinx].bufinx != bufinx )
   {
      Dpy[dpinx].bufinx = bufinx;
      call_dpydata_redraw( dpinx );
   }

   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_bufinfo - display some information about a buffer to stdout.
**  Syntax: BufInfo bufid verbose_flag
**--------------------------------------------------------------------------------
*/
int do_bufinfo( int index, char * par )
{
   int bufinx, x, y,
       verbose,
       rc;
   float mean;
   struct df_buf_t *df;
   struct stats_t  *sp;
	char * str_ptr;

   if( (bufinx = parseSelection_r( par, " ", &str_ptr, buffer_selection)) < 0 )
      return bufinx;
   if( (rc = parseInt_r( &verbose, NULL, " ", &str_ptr )) != ERR_NONE)
      verbose = 0;

   df = &Buffer[bufinx];
   sp = &Stats[bufinx];

   if( df->status == DF_EMPTY )
   {
      printf("buffer is empty!\n");
      return ERR_NONE;
   }

   x = df->naxis1/4;
   y = df->naxis2/4;
   mean = (sp->objN>0 ? sp->objsum/sp->objN : 0);

   if( verbose )
   {
      printf("    Status %d \n", df->status );
      printf("    naxis1 %d \n", df->naxis1 );
      printf("    naxis2 %d \n", df->naxis2 );
      printf("      size %d \n", df->size );
      printf("    bitpix %d \n", df->bitpix );
      printf("         N %d \n", df->N );
      printf("       max %f \n", df->max );
      printf("       mix %f \n", df->min );
      printf("      mean %f \n", df->mean );
      printf("    stddev %f \n", df->stddev );
      printf("      as/p %f \n", df->arcsec_pixel );
      printf("   divisor %f \n", df->divisor );
      printf(" directory %s \n", df->directory );
      printf("  filename %s \n", df->filename );
      printf("     Min     Max   Mean    STD   data[%d,%d]   x  y  wid hgt  "
             "objMin  objMax objMean objSTD\n", x, y);
   }

   printf(" %6.1f %6.1f %6.1f %7.2f  %10.2f   %3d %3d  %3d %3d %6.1f %6.1f %6.1f %7.2f\n",
      df->min, df->max, df->mean, df->stddev, dfdataxy( df, x, y),
      sp->objx, sp->objy, sp->objwid, sp->objhgt, sp->objmin, sp->objmax,
      mean, sp->objstd);

   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
** do_clear - delete the data in a buffer.
**     Syntax: Clear bufid
**--------------------------------------------------------------------------------
*/
int do_clear( int index, char *par )
{
   int dest;
	char * str_ptr;

   /* dest */
   if( (dest = parseSelection_r( par, " ", &str_ptr, buffer_selection)) < 0 )
      return dest;

   /* delete data */
   df_free_fbuffer( &Buffer[dest] );

   /* refresh display */
   cal_box_stats( dest );
   redraw_dpydata_for_bufinx( dest );
   auto_update_hist_ref_bufinx( dest );  // update histo autoscale
   update_file_save_dialog( dest );
   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_cmCenter() - Adjusts the center of the colormap. The center point
**    values ranges from 0 to 1, 0.5 being the middle.
**     Syntax: cm.Center value
**--------------------------------------------------------------------------------
*/
int do_cmCenter( int index, char * par )
{
   float f;
   int rc;
	char * str_ptr;

   if( (rc = parseFloatR_r( &f, par, " ", &str_ptr, 0.0, 1.0)) )
      return rc;

   CM.center = f;
   cm_set_colormap( &CM );
   if( CM.visual->type == GDK_VISUAL_TRUE_COLOR )
      call_colormap_redraw( );
   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_cmWidth() - Adjusts the width of the colormap. The width
**    values ranges from 0 to 1, 0.5 being the normalize value.
**     Syntax: cm.Width value
**--------------------------------------------------------------------------------
*/
int do_cmWidth( int index, char * par )
{
   float f;
   int rc;
	char * str_ptr;

   if( (rc = parseFloatR_r( &f, par, " ", &str_ptr, 0.0, 1.0)) )
      return rc;

   CM.width = f;
   cm_set_colormap( &CM );
   if( CM.visual->type == GDK_VISUAL_TRUE_COLOR )
      call_colormap_redraw( );
   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_cm_swap_rgb() - when TRUE, changes the byte_order value on the CM.
**     Syntax: cm_swap_rgb { off | on }
**--------------------------------------------------------------------------------
*/
int do_cm_swap_rgb( int index, char * par )
{
   int ipar;
	char * str_ptr;

   /* get buf_id */
   if( ( ipar = parseSelection_r( par, " ", &str_ptr, offon_selection)) < 0 )
      return ipar;

   CM.swap_rgb = ipar;
   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
** do_colormap - Reads in a color map from a file.
**     Syntax: colormap filename
**--------------------------------------------------------------------------------
*/

int do_colormap( int index, char *par )
{
   FILE *fp;
   int colorid = 0;
   float value, position;
   char buf[160];
	char *cptr;
	char *str_ptr;

   /* Open file for reading */
   cat_pathname( buf, Lc.app_home, par, sizeof(buf));
   if( NULL == (fp = fopen( buf, "rt")) )   /* Open color map file */
      return ERR_FILE_READ;

   /* Get the file type indicator...should be "IRTF CMF" */
   if( NULL == fgets( buf, sizeof buf, fp))
      return ERR_FILE_READ;
   strupr( buf );
   if( NULL==strstr(buf, "IRTF CMF"))
      return ERR_FILE_READ;

   /* Read colormap description from file */
   colorid = 0;
   CM.cgraph_num_ele[CM_RED_INX] = CM.cgraph_num_ele[CM_GREEN_INX] = CM.cgraph_num_ele[CM_BLUE_INX] = 0;
   while( NULL != fgets( buf, sizeof buf, fp) )
   {
      strupr( buf );
      if( strstr(buf, "RED") )        /* Get color index to CM */
         colorid = CM_RED_INX;
      else if( strstr(buf, "GREEN") )
         colorid = CM_GREEN_INX;
      else if(strstr(buf, "BLUE") )
         colorid = CM_BLUE_INX;
      else
      {
         if( NULL == (cptr = strtok_r(buf, " ,", &str_ptr)) )
            break;
         position = atof( cptr );
         if( NULL == (cptr = strtok_r((char*)NULL, " ,", &str_ptr)) )
            break;
         value = atof( cptr );
         if( INRANGE(0.0, position, 1.0) && INRANGE(0.0, value, 1.0) && colorid < CM_NUM_ELE )
         {
            /* printf("Position is %f and Intensity is %f\n",position,value);  */
            CM.cgraph[colorid][0][CM.cgraph_num_ele[colorid]] = position;
            CM.cgraph[colorid][1][CM.cgraph_num_ele[colorid]] = value;
            CM.cgraph_num_ele[colorid]++;
         }
      }
   }

   CM.center = 0.5;
   CM.width = 0.5;
   cm_set_colormap( &CM );
   if( CM.visual->type == GDK_VISUAL_TRUE_COLOR )
      call_colormap_redraw( );

   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_colormap_inverse() - Inverses the current colormap.
**     Syntax: Colormap.Inverse
**--------------------------------------------------------------------------------
*/
int do_colormap_inverse( int index, char * par )
{
   int   c, i, n;
   float cmdef[2][CM_NUM_ELE];

   for( c=0; c<3; c++)  /* loop for R, G, B */
   {
      /* copy color map definition */
      n = CM.cgraph_num_ele[c];
      for(i=0; i<n; i++)
      {
         cmdef[0][i] = CM.cgraph[c][0][i];
         cmdef[1][i] = CM.cgraph[c][1][i];
      }

      /* write back definition in reverse order & adjust index variable */
      for(i=0; i<n; i++)
      {
         CM.cgraph[c][0][i] = fabs(cmdef[0][n-i-1] - 1 );
         CM.cgraph[c][1][i] = cmdef[1][n-i-1];
      }
   }

   Lc.cminverse = !Lc.cminverse;
   cm_set_colormap( &CM );
   if( CM.visual->type == GDK_VISUAL_TRUE_COLOR )
      call_colormap_redraw( );

   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_copy() - copy data from a src buffer to a dest buffer.
**     Syntax: COPY src TO dest
**--------------------------------------------------------------------------------
*/
int do_copy( int index, char * par )
{
   int  dest, src, rc;
	char * str_ptr;

   /* src */
   if( (src = parseSelection_r( par, " ", &str_ptr, buffer_selection)) < 0 )
      return src;

   /* to */
   if( (rc = parseSelection_r( NULL, " ", &str_ptr, to_selection)) < 0 )
      return rc;

   /* dest */
   if( (dest = parseSelection_r( NULL, " ", &str_ptr, buffer_selection)) < 0 )
      return dest;

   rc = df_buffer_math( &Buffer[dest], &Buffer[src], &Buffer[src], DF_MATH_COPY);

   /* redraw destination frame */
   cal_box_stats( dest );
   redraw_dpydata_for_bufinx( dest );
   auto_update_hist_ref_bufinx( dest );  // update histo autoscale
   update_file_save_dialog( dest );
   return rc;
}

/*--------------------------------------------------------------------------------
**  do_displaytype() - Select a display format for drawing area.
**     Syntax: displaytype {Image|Histogram|...} [dpinx]
**--------------------------------------------------------------------------------
*/
int do_displaytype( int index, char * par )
{
   int dpytype, dpinx;
	char * str_ptr;

   if( (dpytype = parseSelection_r( par, " ", &str_ptr, dpytype_selection)) < 0 )
      return dpytype;
   if( parseIntR_r( &dpinx, NULL, " ", &str_ptr, 0, Lc.num_dpy-1) != ERR_NONE)
      dpinx = Lc.active;

   /* update */
   if( Dpy[dpinx].dpytype != dpytype )
   {
      Dpy[dpinx].dpytype = dpytype;
      call_dpydata_redraw( dpinx );
   }
   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_divbydivisor() - Enables/disable the divide by divisor option. Representation
**     of the data can be divided by the DIVISOR keywork value in the FITS header.
**     Syntax:  DivByDivisor { off | on }
**--------------------------------------------------------------------------------
*/
int do_divbydivisor( index, par )
   int index;
   char * par;
{
   int ipar, bufinx;
	char * str_ptr;

   /* get buf_id */
   if( ( ipar = parseSelection_r( par, " ", &str_ptr, offon_selection )) < 0 )
      return ipar;

   /* apply parameters */
   if( Lc.divbydivisor != ipar )
   {
      Lc.divbydivisor = ipar;
      dfset_divbycoadd( ipar );  /* update the dflib to handle the divbycoadd */

      /* recalculate all stats & redisplay all canvases */
      for( bufinx=0; bufinx < NUM_BUFFER; bufinx++ )
      {
         if( Buffer[bufinx].status != DF_EMPTY )
         {
            df_stats( &Buffer[bufinx] );
            cal_box_stats( bufinx );
            redraw_dpydata_for_bufinx( bufinx );
            auto_update_hist_ref_bufinx( bufinx );  // update histo autoscale
            update_file_save_dialog( bufinx );
         }
      }
   }
   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_drawbox - Draws a Box on the image.
**     Syntax: DrawBox bufid x y wid hgt
**--------------------------------------------------------------------------------
*/
int do_drawbox( int index, char * par )
{
   int  bufinx,
        dpinx,
        x, y, wid, hgt,
        px, py, pwid, phgt,
        rc;
   float pixel_wid;
   struct dpy_t *dp;
   struct df_buf_t *bp;
	char * str_ptr;

   /*-------------------
   ** parse arguments.
   */
   if( (bufinx = parseSelection_r( par, " ", &str_ptr, buffer_selection)) < 0 )
      return bufinx;
   bp = &Buffer[bufinx];

   if( (rc = parseIntR_r( &x, NULL, " ", &str_ptr, 0, NUM_PIXEL-1)) != ERR_NONE)
      return rc;
   if( (rc = parseIntR_r( &y, NULL, " ", &str_ptr, 0, NUM_PIXEL-1)) != ERR_NONE)
      return rc;
   if( (rc = parseIntR_r( &wid, NULL, " ", &str_ptr, 0, NUM_PIXEL-x)) != ERR_NONE)
      return rc;
   if( (rc = parseIntR_r( &hgt, NULL, " ", &str_ptr, 0, NUM_PIXEL-y)) != ERR_NONE)
      return rc;

   /* check status */
   if( bp->status == DF_EMPTY )
      return ERR_NONE;

   /*----------------------------------------------------
   ** Draw on  each display showing an image from bufinx.
   */
   for( dpinx=0; dpinx < Lc.num_dpy; dpinx++ )
   {
      dp = &Dpy[dpinx];
      if( (dp->bufinx == bufinx) && (dp->dpytype == DPYTYPE_IMAGE))
      {
         pixel_wid = dp->image_zoom > 0 ? dp->image_zoom : 1.0/(1.0+abs(dp->image_zoom));

         /* convert x,y,wid,hgt to pixel location for draw command */
         px = ((x - dp->image_offx) * pixel_wid) + 0.5;
         py = ((y - dp->image_offy) * pixel_wid) + 0.5;
         pwid = (wid*pixel_wid)+0.5;
         if( pwid < 1 ) pwid = 1;
         phgt = (hgt*pixel_wid)+0.5;
         if( phgt < 1 ) phgt = 1;

         gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_GREEN] );
         gdk_draw_rectangle( dp->data_drawingarea->window, W.nor_gc, FALSE, px, py, pwid, phgt);
      }

   }

   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_drawline - Draws a line from x1,y1 to x2,y2 on the image.
**     Syntax: DrawLine bufid x1 y1 x2 y2
**--------------------------------------------------------------------------------
*/
int do_drawline( int index, char * par )
{
   int  bufinx,
        dpinx,
        x1, y1, x2, y2,
        px1, py1, px2, py2,
        rc;
   float pixel_wid;
   struct dpy_t *dp;
   struct df_buf_t *bp;
	char * str_ptr;

   /*-------------------
   ** parse arguments.
   */
   if( (bufinx = parseSelection_r( par, " ", &str_ptr, buffer_selection)) < 0 )
      return bufinx;
   bp = &Buffer[bufinx];

   if( (rc = parseIntR_r( &x1, NULL, " ", &str_ptr, 0, NUM_PIXEL-1)) != ERR_NONE)
      return rc;
   if( (rc = parseIntR_r( &y1, NULL, " ", &str_ptr, 0, NUM_PIXEL-1)) != ERR_NONE)
      return rc;
   if( (rc = parseIntR_r( &x2, NULL, " ", &str_ptr, 0, NUM_PIXEL-1)) != ERR_NONE)
      return rc;
   if( (rc = parseIntR_r( &y2, NULL, " ", &str_ptr, 0, NUM_PIXEL-1)) != ERR_NONE)
      return rc;

   /* check status */
   if( bp->status == DF_EMPTY )
      return ERR_NONE;

   /*----------------------------------------------------
   ** Draw on  each display showing an image from bufinx.
   */
   for( dpinx=0; dpinx < Lc.num_dpy; dpinx++ )
   {
      dp = &Dpy[dpinx];
      if( (dp->bufinx == bufinx) && (dp->dpytype == DPYTYPE_IMAGE))
      {
         pixel_wid = dp->image_zoom > 0 ? dp->image_zoom : 1.0/(1.0+abs(dp->image_zoom));

         /* convert x,y,wid,hgt to pixel location for draw command */
         px1 = ((x1 - dp->image_offx) * pixel_wid) + (pixel_wid/2);
         py1 = ((y1 - dp->image_offy) * pixel_wid) + (pixel_wid/2);
         px2 = ((x2 - dp->image_offx) * pixel_wid) + (pixel_wid/2);
         py2 = ((y2 - dp->image_offy) * pixel_wid) + (pixel_wid/2);

         gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_GREEN] );
         gdk_draw_line( dp->data_drawingarea->window, W.nor_gc, px1, py1, px2, py2);
      }

   }

   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_echo - Prints text on the TextOut_w.
**--------------------------------------------------------------------------------
*/
int do_echo( int index, char * par )
{
   char buf[128];

   if( par )
   {
      strxcpy( buf, par, sizeof(buf));
		cc_printf( W.cmd.main_console, CM_BLUE, "%s\n", buf );
   }
   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_filter1() - changes data in buffer so:
**   a. rescale data so [mean-std, mean+std] maps to -25 to +25
**   b. reset negative values to 0.
**  The filter could be useful when applied to data before using the centroid
**  algorithm.
**     Syntax: Filter1 bufinx
**--------------------------------------------------------------------------------
*/
int do_filter1( int index, char * par )
{
   int  dest, src, rc;
	char * str_ptr;

   /* src */
   if( (src = parseSelection_r( par, " ", &str_ptr, buffer_selection)) < 0 )
      return src;

   /* to */
   if( (rc = parseSelection_r( NULL, " ", &str_ptr, to_selection)) < 0 )
      return rc;

   /* dest */
   if( (dest = parseSelection_r( NULL, " ", &str_ptr, buffer_selection)) < 0 )
      return dest;

   rc = df_buffer_userfun( &Buffer[dest], &Buffer[src], my_filter1_fun);

   /* redraw destination frame */
   cal_box_stats( dest );
   redraw_dpydata_for_bufinx( dest );
	auto_update_hist_ref_bufinx( dest );  // update histo autoscale
   update_file_save_dialog( dest );
   return rc;
}

int my_filter1_fun(  struct df_buf_t *dest, struct df_buf_t *op1 )
{
   int inx, N;
   double low,
          high,
          data;

   low  = op1->mean - op1->stddev;
   high = op1->mean + op1->stddev;
   if( (high-low) < 1.0 ) high = low + 1;

   N = op1->naxis1 * op1->naxis2;
   for( inx=0; inx < N; inx++ )
   {
      data = dfdatainx( op1, inx );
      data = map( data, low, high, -25, 25 );
      if( data < 0 ) data = 0;

      dest->fdata[inx] = data;
   }

   return 0;
}

/*--------------------------------------------------------------------------------
**  do_filter2() - changes data in buffer so:
**    a. reset mean to 0.
**    b. divide data by std
**    c. then zero value that are <= 1.
**  The filter could be useful when applied to data before using the centroid
**  algorithm.
**     Syntax: Filter2 bufinx
**--------------------------------------------------------------------------------
*/
int do_filter2( int index, char * par )
{
   int  dest, src, rc;
	char * str_ptr;

   /* src */
   if( (src = parseSelection_r( par, " ", &str_ptr, buffer_selection)) < 0 )
      return src;

   /* to */
   if( (rc = parseSelection_r( NULL, " ", &str_ptr, to_selection)) < 0 )
      return rc;

   /* dest */
   if( (dest = parseSelection_r( NULL, " ", &str_ptr, buffer_selection)) < 0 )
      return dest;

   rc = df_buffer_userfun( &Buffer[dest], &Buffer[src], my_filter2_fun);

   /* redraw destination frame */
   cal_box_stats( dest );
   redraw_dpydata_for_bufinx( dest );
	auto_update_hist_ref_bufinx( dest );  // update histo autoscale
   update_file_save_dialog( dest );
   return rc;
}

int my_filter2_fun(  struct df_buf_t *dest, struct df_buf_t *op1 )
{
   int inx, N;
   double mean,
          std,
          data;

   mean  = op1->mean;
   std   = op1->stddev;
   N = op1->naxis1 * op1->naxis2;

   for( inx=0; inx < N; inx++ )
   {
      data = dfdatainx( op1, inx );
      data = ( data - mean )  / std;
      if( data < 0 ) data = 0;

      dest->fdata[inx] = data;
   }

   return 0;
}

/*--------------------------------------------------------------------------------
**  do_fullimage() - Zooms the dpyinx canvas so the entire image is visible.
**                   Defaults to the active canvase if dpyinx not specified.
**     Syntax: fullimage [dpinx]
**--------------------------------------------------------------------------------
*/
int do_fullimage( index, par )
   int index;
   char * par;
{
   int dpinx,
       zoom1, zoom2, zoom;
   int da_wid, da_hgt;
   struct df_buf_t *bp;            /* buffer pointer */
	char * str_ptr;

   if( parseIntR_r( &dpinx, par, " ", &str_ptr, 0, Lc.num_dpy-1) != ERR_NONE)
      dpinx = Lc.active;
   bp = &Buffer[Dpy[dpinx].bufinx];

   if( bp->status == DF_EMPTY )
      return ERR_NO_DATA;

   /* get size of the drawing area */
   if( Dpy[dpinx].data_drawingarea->window == NULL )
      return ERR_NONE;
   gdk_drawable_get_size( Dpy[dpinx].data_drawingarea->window, &da_wid, &da_hgt);

   if( da_wid >= bp->naxis1 )
      zoom1 = da_wid / bp->naxis1;
   else
      zoom1 = -((bp->naxis1-1) / da_wid);

   if( da_hgt >= bp->naxis2 )
      zoom2 = da_hgt / bp->naxis2;
   else
      zoom2 = -((bp->naxis2-1) / da_hgt );

   /*
   printf("dpy=%d  x: %d %d=%d  y:%d %d=%d\n",
      dpinx, bp->naxis1, da_wid, zoom1 , bp->naxis2, da_wid, zoom2 );
   */

   zoom = MIN( zoom1, zoom2 );
   zoom = (zoom > 20 ? 20 : zoom);
   zoom = (zoom < -20 ? -20 : zoom);
   zoom = (zoom == 0 ? 1 : zoom);

   /* update */
   if( Dpy[dpinx].image_zoom != zoom )
   {
      Dpy[dpinx].image_zoom = zoom;

      /* Redraw the canvas */
      if( Dpy[dpinx].dpytype == DPYTYPE_IMAGE )
         call_dpydata_redraw( dpinx );
   }
   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_gbox_cmd_right_click() - Setting this flag to on will make DV issue
**   a guidebox command when the user issue a right mouse click (button 1)
**   in the Image window.
**     Syntax: Gbox.Cmd.Right.Click { off | on }
**--------------------------------------------------------------------------------
*/
int do_gbox_cmd_right_click( int index, char * par )
{
   int offon;
	char * str_ptr;

   /* get instrument */
   if( (offon=parseSelection_r( par, " ", &str_ptr, offon_selection)) < 0 )
      return offon;

   Lc.gbox_cmd_right_click = offon;
   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_histarea() - Set the range of pixel to be included in the histogram.
**     Syntax: HistArea { All | Box } [dpinx]
**--------------------------------------------------------------------------------
*/
int do_histarea( int index, char * par )
{
   int rc, area, dpinx;
	char * str_ptr;

   /* Get area */
   if( par == NULL )
      return ERR_INV_FORMAT;

   if( (area = parseSelection_r( par, " ", &str_ptr, allbox_selection)) < 0 )
      return area;

   /* Get dpinx */
   if( (rc = parseIntR_r( &dpinx, NULL, " ", &str_ptr,  0, Lc.num_dpy-1)) != ERR_NONE)
      dpinx = Lc.active;

   /* update */
   if( Dpy[dpinx].hist_area != area )
   {
      Dpy[dpinx].hist_area = area;

      if( Dpy[dpinx].dpytype == DPYTYPE_HISTOGRAM )
         call_dpydata_redraw( dpinx );
   }
   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_hist_auto_range() - Auto scale the histogram based on the image scale. 
**     Syntax: HistAutoRange { off | on } [dpinx]
**--------------------------------------------------------------------------------
*/
int do_hist_auto_range( int index, char * par )
{
   int rc, offon, dpinx;
	char * str_ptr;

   /* Get offon */
   if( par == NULL )
      return ERR_INV_FORMAT;

   if( (offon = parseSelection_r( par, " ", &str_ptr, offon_selection)) < 0 )
      return offon;

   /* Get dpinx */
   if( (rc = parseIntR_r( &dpinx, NULL, " ", &str_ptr,  0, Lc.num_dpy-1)) != ERR_NONE)
      dpinx = Lc.active;

   /* update */
   if( Dpy[dpinx].hist_auto_range != offon )
   {
      Dpy[dpinx].hist_auto_range = offon;

      if( Dpy[dpinx].dpytype == DPYTYPE_HISTOGRAM )
         call_dpydata_redraw( dpinx );
   }
   return ERR_NONE;
}

/* updates the histogram that is referencing this Dyp[dpinx]->bufinx */
void auto_update_hist_ref_dpinx( int dpinx  )
{
   int i; 

	for( i=0; i<  Lc.num_dpy; i++ )
	{
      if( (Dpy[i].dpytype == DPYTYPE_HISTOGRAM) && (Dpy[i].hist_auto_range==TRUE) && 
          (Dpy[i].bufinx == Dpy[dpinx].bufinx ))
		{
         Dpy[i].image_min = Dpy[dpinx].image_min;
         Dpy[i].image_max = Dpy[dpinx].image_max;
			//printf("auto_update_hist_ref_Dpinx():  dpinx=%d \n", i); 
         call_dpydata_redraw( i );
		}
	}
}

/* updates the histogram for new data in bufinx  */
void auto_update_hist_ref_bufinx( int bufinx )
{
   int i,
       found; 
   double image_min, image_max;

   // ignore empty buffers
   if( Buffer[bufinx].status == DF_EMPTY )
	   return;

   image_min = 0;
   image_max = 10;
   found = FALSE;
   // find the 1st Dpy show the image, and use this for the auto update values.
	for( i=0; i<  Lc.num_dpy; i++ )
   {
      if( (Dpy[i].dpytype == DPYTYPE_IMAGE) && (Dpy[i].bufinx == bufinx ) )
		{
		   found = TRUE;
			image_min = Dpy[i].image_min;
			image_max = Dpy[i].image_max;
		}
   }
	// no image displayed then no auto update for histogram.
	if( !found )
	   return;

	for( i=0; i<  Lc.num_dpy; i++ )
	{
      if( (Dpy[i].dpytype == DPYTYPE_HISTOGRAM) && (Dpy[i].hist_auto_range==TRUE) && 
          (Dpy[i].bufinx == bufinx ))
		{
         Dpy[i].image_min = image_min;
         Dpy[i].image_max = image_max;
			//printf("auto_update_hist_ref_Bufinx(): dpyinx=%d \n", i); 
         call_dpydata_redraw( i );
		}
	}
}

/*--------------------------------------------------------------------------------
**  do_histbin() - Sets the number of bins for a histogram display.
**     Syntax: HistBin num [dpinx]
**--------------------------------------------------------------------------------
*/
int do_histbin( int index, char * par )
{
   int rc, dpinx, bin;
	char *str_ptr;

   /* Get bin */
   if( (rc = parseIntR_r( &bin, par, " ", &str_ptr, 1, HIST_NUM_BIN)) != ERR_NONE )
      return rc;

   /* Get dpinx */
   if( (rc = parseIntR_r( &dpinx, NULL, " ", &str_ptr, 0, Lc.num_dpy-1)) != ERR_NONE )
      dpinx = Lc.active;

   /* update */
   if( Dpy[dpinx].hist_bin != bin )
   {
      Dpy[dpinx].hist_bin = bin;
      if(  (Dpy[dpinx].dpytype == DPYTYPE_HISTOGRAM) )
         call_dpydata_redraw( dpinx );
   }
   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_imageautoscale() - Set the zoom level for a image display.
**     Syntax: ImageAutoScale { Fixed | Auto } [dpinx]
**--------------------------------------------------------------------------------
*/
int do_imageautoscale( int index, char * par )
{
   int scale,
       dpinx,
       rc;
	char * str_ptr;

   /* Get scale */
   if( (scale = parseSelection_r( par, " ", &str_ptr, imageautoscale_selection)) < 0 )
      return scale;

   /* Get dpinx */
   if( (rc = parseIntR_r( &dpinx, NULL, " ", &str_ptr, 0, Lc.num_dpy-1)) != ERR_NONE)
      dpinx = Lc.active;

   /* update */
   if( Dpy[dpinx].image_autoscale != scale )
   {
      Dpy[dpinx].image_autoscale = scale;
      if( (Dpy[dpinx].dpytype == DPYTYPE_IMAGE ) )
         call_dpydata_redraw( dpinx );

   }

   auto_update_hist_ref_dpinx( dpinx );
   return ERR_NONE;
}


/*--------------------------------------------------------------------------------
**  do_imagecompass_fipNS() - flip the NS axis on the image compass / tcs offses
**     Syntax: ImageCompass.flipNS { off | on } 
**--------------------------------------------------------------------------------
*/
int do_imagecompass_flipNS( int index, char * par )
{
   int image_compass_flipNS;
	char * str_ptr;

   /* Get value */
   if( (image_compass_flipNS = parseSelection_r( par, " ", &str_ptr, offon_selection)) < 0 )
      return image_compass_flipNS;

   Lc.image_compass_flipNS = image_compass_flipNS;
   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_imagecompass() - Set switch to draw image compass.
**     Syntax: ImageCompass.show { off | on } 
**--------------------------------------------------------------------------------
*/
int do_imagecompass_show( int index, char * par )
{
   int image_compass_show;
	char * str_ptr;

   /* Get value */
   if( (image_compass_show = parseSelection_r( par, " ", &str_ptr, offon_selection)) < 0 )
      return image_compass_show;

   Lc.image_compass_show = image_compass_show;
   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_imagerange() - Sets the image display range for the Active_canvas.
**     Syntax:  ImageRange  min max [dpinx]
**--------------------------------------------------------------------------------
*/
int do_imagerange( int index, char * par )
{
   int dpinx,
       rc;
   float min, max;
	char * str_ptr;

   /* Get max & min */
   if( (rc = parseFloat_r( &min, par, " ", &str_ptr)) != ERR_NONE)
      return rc;
   if( (rc = parseFloat_r( &max, NULL, " ", &str_ptr)) != ERR_NONE)
      return rc;

   if( (min+0.1) >= max )
      return ERR_INV_FORMAT;

   /* Get dpinx */
   if( (rc = parseIntR_r( &dpinx, NULL, " ", &str_ptr, 0, Lc.num_dpy-1)) != ERR_NONE)
      dpinx = Lc.active;

   /* update */
   Dpy[dpinx].image_autoscale = FALSE;
   if( Dpy[dpinx].image_min != min || Dpy[dpinx].image_max != max )
   {
      Dpy[dpinx].image_min = min;
      Dpy[dpinx].image_max = max;
      if( (Dpy[dpinx].dpytype==DPYTYPE_IMAGE) )
         call_dpydata_redraw( dpinx );
   }

   // redraw histogram
   auto_update_hist_ref_dpinx( dpinx );

   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_imagescale1p() - Set the fixed scale to include all the pixel values
**     except for the top or bottom 1 percent.
**     Syntax: ImageScale1P [dpinx]
**--------------------------------------------------------------------------------
*/
int do_imagescale1p( int index, char * par )
{
   int  bufinx,
        dpinx,
        rc;
   int   min, max;
	char * str_ptr;

   /* Get dpinx */
   if( (rc = parseIntR_r( &dpinx, par, " ", &str_ptr, 0, Lc.num_dpy-1)) != ERR_NONE)
      dpinx = Lc.active;

   /* check status */
   bufinx =  Dpy[dpinx].bufinx;
   if( Buffer[bufinx].status == DF_EMPTY )
      return ERR_NO_DATA;

   {
      struct timeval  beg;
      struct timeval  end;
      int ems;

      gettimeofday( &beg, NULL);
      cal_1per( &Buffer[bufinx], &min, &max );
      gettimeofday( &end, NULL);
      ems = elapse_msec( &beg, &end );
		cc_printf( W.cmd.main_console, CM_BLUE, " cal_1per(): [%ld...%ld] ems=%ld\n", min, max, ems);
   }

   /* update */
   Dpy[dpinx].image_autoscale = FALSE;
   if( Dpy[dpinx].image_min != min || Dpy[dpinx].image_max != max )
   {
      Dpy[dpinx].image_min = min;
      Dpy[dpinx].image_max = max;
      if( (Dpy[dpinx].dpytype==DPYTYPE_IMAGE) || (Dpy[dpinx].dpytype==DPYTYPE_HISTOGRAM) )
         call_dpydata_redraw( dpinx );
   }

   // redraw histogram
   auto_update_hist_ref_dpinx( dpinx );

   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  cal_1per() - function to find min & max of data set to include 99% of
**    the data. Excluding the top/bottom 1 percent value. Just an approximation
**    using a histogram algorithm.
**--------------------------------------------------------------------------------
*/
int cal_1per( struct df_buf_t *bp, int32_t * Rmin, int32_t * Rmax )
{
   int  num_bins,
        pixel_low,
        pixel_high,
        pixel_binned,
        i;
   int32_t *bins;
   float hist_max,
         hist_min,
         bin_size;

   float min, max;

   /* allocate memory for histogram bins & initialize variables */
   num_bins = 400;  /* let use 400 bins */
   if( NULL == ( bins = (int32_t *)malloc( sizeof(int32_t)*num_bins)) )
      return ERR_MEM_ALLOC;
   memset( bins, 0, sizeof(int32_t)*num_bins);
   pixel_low = 0;
   pixel_high = 0;
   pixel_binned = 0;

   /* find min & max range for histogram - use mean +- 4 std_dev. or Max/Min
   ** if Max/Min is less
   */
   hist_min = bp->mean - (bp->stddev*4);
   if( hist_min < bp->min )
      hist_min = bp->min;
   hist_max = bp->mean + (bp->stddev*4);
   if( hist_max < bp->max )
      hist_max = bp->max;

   bin_size = (hist_max - hist_min) / num_bins;

#if DEBUG
   printf("min: %3.1f,%3.1f->%3.1f\n", bp->mean-(bp->stddev*4), bp->min, hist_min);
   printf("max: %3.1f,%3.1f->%3.1f\n", bp->mean+(bp->stddev*4), bp->max, hist_max);
   printf("BinSize = %5.2f \n", bin_size);
#endif

   /* Loop through data, and bin according to values.
   */
   for( i = 0; i < bp->N; i++ )
   {
      float data;
      int   bin;

      data = dfdatainx( bp, i );
      bin  =  (data-hist_min) / bin_size;

      if( bin < 0 )
         pixel_low++;
      else if( bin >= num_bins )
         pixel_high++;
      else
      {
         pixel_binned++;
         bins[bin]++;
      }
      /* if( i < 10 ) printf(" data=%f  bin=%d \n", data, bin ); */
   }

#if DEBUG
   printf(" Low: %d %4.2f%% \n", pixel_low,  (100.0*pixel_low) / bp->N);
   printf(" Hi:  %d %4.2f%% \n", pixel_high, (100.0*pixel_high) / bp->N);
   printf(" Bin: %d %4.2f%% \n", pixel_binned, (100.0*pixel_binned) / bp->N);
   printf(" Total=%d  N=%ld \n", pixel_low+pixel_high+pixel_binned, bp->N);
#endif

   /* Find max/min */
   min = hist_min;
   max = hist_max;
   {
      int cnt, inx;

      /* find range value for min */
      cnt = pixel_low;
      for( i=0; i < num_bins; i++ )
      {
         float p;

         min = hist_min + ( bin_size * i );
         p  = (100.0 * cnt)/bp->N;

         cnt += bins[i];
         p  = (100.0 * cnt)/bp->N;
         if( p >= 1.0 )  /* if > 1 percent */
            break;
      }

      /* find range value for max */
      cnt = pixel_high;
      inx = num_bins-1;
      for( i=0; i<num_bins; i++, inx-- )
      {
         float p;

         max = hist_max - (bin_size*i);
         p  = (100.0 * cnt)/bp->N;

         cnt += bins[inx];
         p  = (100.0 * cnt)/bp->N;
         if( p >= 1.0 )  /* if > 1 percent */
            break;
      }

      if( (min + 1.0) > max )
         max = min + 1.0;
   }

   *Rmin = rint( min );
   *Rmax = rint( max );
   free( bins );
   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_imageshowgbox() - Show the guidebox information in the image display
**     Syntax: ImageShowGBox { off | on } 
**--------------------------------------------------------------------------------
*/
int do_imageshowgbox( int index, char * par )
{
   int image_show_gbox;
	char * str_ptr;

   /* Get value */
   if( (image_show_gbox = parseSelection_r( par, " ", &str_ptr, offon_selection)) < 0 )
      return image_show_gbox;

   Lc.image_show_gbox = image_show_gbox;
   return ERR_NONE;

}

/*--------------------------------------------------------------------------------
**  do_imagezoom() - Set the zoom level for a image display.
**     Syntax: ImageZoom zoom dpinx
**--------------------------------------------------------------------------------
*/
int do_imagezoom( int index, char * par )
{
   int zoom,
       dpinx,
       rc;
	char * str_ptr;

   printf("imageZoom %s \n", par);
   /* Get dpinx */
   if( (rc = parseIntR_r( &zoom, par, " ", &str_ptr, -20, 20)) != ERR_NONE)
      return rc;
   zoom = (zoom==0 ? 1 : zoom);    /* can't be zero */

   /* Get dpinx */
   if( (rc = parseIntR_r( &dpinx, NULL, " ", &str_ptr, 0, Lc.num_dpy-1)) != ERR_NONE)
      dpinx = Lc.active;

   /* update */
   if( Dpy[dpinx].image_zoom != zoom )
   {
      Dpy[dpinx].image_zoom = zoom;
      if( Dpy[dpinx].dpytype == DPYTYPE_IMAGE )
         call_dpydata_redraw( dpinx );
   }
   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_inst_com() - Issues a command to an Instrument's IC.
**     Syntax: Inst.Com command
**--------------------------------------------------------------------------------
*/
int do_inst_com( int index, char * par )
{
   int rc;
	char reply[80];

   if( par == NULL )
      return ERR_NONE;

   if( Lc.inst_flavor == INST_FLAVOR_BIGDOG ) 
		rc = bigdog_com( par, reply, sizeof(reply), "localhost" );
   else if( Lc.inst_flavor == INST_FLAVOR_GUIDEDOG ) 
		rc = guidedog_com( par, reply, sizeof(reply), "localhost" );
   else if( Lc.inst_flavor == INST_FLAVOR_SMOKEY )
		rc = smokey_com(  par, reply, sizeof(reply), "localhost" );
   else if( Lc.inst_flavor == INST_FLAVOR_MORIS )  
		rc = moris_com( par, reply, sizeof(reply), "moris" );
   else
	   rc = ERR_INV_RNG;

   return  rc;
}

/*--------------------------------------------------------------------------------
**  do_inst_flavor() - Tailor some function for specific insturments.
**     Syntax: Inst.Flavor { Spex | Smokey }
**--------------------------------------------------------------------------------
*/
int do_inst_flavor( int index, char * par )
{
   int ipar;
	char * str_ptr;

   /* get instrument */
   if( (ipar=parseSelection_r( par, " ", &str_ptr, inst_flavor_selection)) < 0 )
      return ipar;

   Lc.inst_flavor = ipar;
   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_lcutarea() - Set the range of pixel to be included in the lcut.
**     Syntax: LCutArea { All | Box } [dpinx]
**--------------------------------------------------------------------------------
*/
int do_lcutarea( int index, char * par )
{
   int rc, area, dpinx;
	char * str_ptr;

   /* Get area */
   if( par == NULL )
      return ERR_INV_FORMAT;

   if( (area = parseSelection_r( par, " ", &str_ptr, allbox_selection)) < 0 )
      return area;

   /* Get dpinx */
   if( (rc = parseIntR_r( &dpinx, NULL, " ", &str_ptr, 0, Lc.num_dpy-1)) != ERR_NONE)
      dpinx = Lc.active;

   /* update */
   if( Dpy[dpinx].lcut_area != area )
   {
      Dpy[dpinx].lcut_area = area;

      if( Dpy[dpinx].dpytype == DPYTYPE_LINECUT )
         call_dpydata_redraw( dpinx );
   }
   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_lcutautoscale() - Set the autoscale float for linecut.
**     Syntax: LCutAutoScale { Fixed | Auto } [dpinx]
**--------------------------------------------------------------------------------
*/
int do_lcutautoscale( int index, char * par )
{
   int scale,
       dpinx,
       rc;
	char * str_ptr;

   /* Get scale */
   if( (scale = parseSelection_r( par, " ", &str_ptr, imageautoscale_selection)) < 0 )
      return scale;

   /* Get dpinx */
   if( (rc = parseIntR_r( &dpinx, NULL, " ", &str_ptr, 0, Lc.num_dpy-1)) != ERR_NONE)
      dpinx = Lc.active;

   /* update */
   if( Dpy[dpinx].lcut_autoscale != scale )
   {
      Dpy[dpinx].lcut_autoscale = scale;

      if( Dpy[dpinx].dpytype == DPYTYPE_LINECUT )
         call_dpydata_redraw( dpinx );
   }
   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_lcutrange() - Sets the linecut display range
**     Syntax:  LCutRange  min max [dpinx]
**--------------------------------------------------------------------------------
*/
int do_lcutrange( index, par )
   int index;
   char * par;
{
   int dpinx,
       rc;
   int min, max;
	char * str_ptr;

   /* Get max & min */
   if( (rc = parseInt_r( &min, par, " ", &str_ptr)) != ERR_NONE)
      return rc;
   if( (rc = parseInt_r( &max, NULL, " ", &str_ptr)) != ERR_NONE)
      return rc;

   if( max <= min )
      return ERR_INV_FORMAT;

   /* Get dpinx */
   if( (rc = parseIntR_r( &dpinx, NULL, " ", &str_ptr, 0, Lc.num_dpy-1)) != ERR_NONE)
      dpinx = Lc.active;

   /* update */
    Dpy[dpinx].lcut_autoscale = FALSE;
   if( Dpy[dpinx].lcut_min != min || Dpy[dpinx].lcut_max != max )
   {
      Dpy[dpinx].lcut_min = min;
      Dpy[dpinx].lcut_max = max;
      if( Dpy[dpinx].dpytype==DPYTYPE_LINECUT )
         call_dpydata_redraw( dpinx );
   }
   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_lcutxy() - Selects the column for a line cut.
**     Syntax: LCutXY x y [dpinx]
**--------------------------------------------------------------------------------
*/
int do_lcutxy( int index, char * par )
{
   int rc, x, y, dpinx;
	char * str_ptr;

   /* Get x */
   if( (rc = parseIntR_r( &x, par, " ", &str_ptr, 0, NUM_PIXEL-1)) != ERR_NONE)
      return rc;

   /* Get y */
   if( (rc = parseIntR_r( &y, NULL, " ", &str_ptr, 0, NUM_PIXEL-1)) != ERR_NONE)
      return rc;

   /* Get dpinx */
   if( (rc = parseIntR_r( &dpinx, NULL, " ", &str_ptr, 0, Lc.num_dpy-1)) != ERR_NONE)
      dpinx = Lc.active;

   /* update */
   if((Dpy[dpinx].lcut_x != x ) || ( Dpy[dpinx].lcut_y != y ))
   {
      Dpy[dpinx].lcut_x = x;
      Dpy[dpinx].lcut_y = y;
      if( Dpy[dpinx].dpytype == DPYTYPE_LINECUT )
         call_dpydata_redraw( dpinx );
   }
   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_math() -performs a math expression.
**     Syntax: b = b {*|/|+|-} {b|num}
**        'b' can be { a | b | c | ... }
**--------------------------------------------------------------------------------
*/
int do_math( int index, char * par)
{
   int  dest, s1, s2, ops,
        is_buffer_math,
        rc;
   float constant;
   char buf[40];
	char * str_ptr;

   /* dest */
   if( (dest = parseSelection_r( par, " ", &str_ptr, buffer_selection)) < 0 )
      return dest;

   /* = */
   if( (rc=parseSelection_r( NULL, " ", &str_ptr, mathops_selection)) < 0 )
      return rc;
   if( rc != 4 ) return ERR_INV_FORMAT;

   /* s1 */
   if( (s1 = parseSelection_r( NULL, " ", &str_ptr, buffer_selection)) < 0 )
      return s1;

   /* ops */
   if( (ops=parseSelection_r( NULL, " ", &str_ptr, mathops_selection)) < 0 )
      return ops;
   if( !INRANGE(DF_MATH_ADD, ops, DF_MATH_DIV))  return ERR_INV_FORMAT;

   /* s2 or constand */
   if( (rc=parseString_r( buf, sizeof(buf), NULL, " ", &str_ptr)) != ERR_NONE )
      return rc;

   is_buffer_math = TRUE;
   if( (s2 = parseSelection_r( buf, " ", &str_ptr, buffer_selection)) < 0 )
   {
      is_buffer_math = FALSE;
      if( (rc = parseFloat_r( &constant, buf, " ", &str_ptr)) != ERR_NONE)
         return rc;
   }

   /* Call function to perform math operation */
   if( is_buffer_math )
   {
      rc = df_buffer_math( &Buffer[dest], &Buffer[s1], &Buffer[s2], ops );
   }
   else
   {
      rc = df_constant_math( &Buffer[dest], &Buffer[s1], constant, ops );
   }

   /*
   ** redraw destination frame.
   */
   cal_box_stats( dest );
   redraw_dpydata_for_bufinx( dest );
	auto_update_hist_ref_bufinx( dest );  // update histo autoscale
   update_file_save_dialog( dest );

   return rc;
}

/*--------------------------------------------------------------------------------
** do_move - moves the data from 1 buffer to another.
**     Syntax: move src TO dest  
**--------------------------------------------------------------------------------
*/
int do_move( int index, char *par )
{
   int dest, src, rc;
	char * str_ptr;

   /* src */
   if( (src = parseSelection_r( par, " ", &str_ptr, buffer_selection)) < 0 )
      return src;

   /* to */
   if( (rc = parseSelection_r( NULL, " ", &str_ptr, to_selection)) < 0 )
      return rc;

   /* dest */
   if( (dest = parseSelection_r( NULL, " ", &str_ptr, buffer_selection)) < 0 )
      return dest;

   if( src == dest )
      return ERR_NONE;

   /* delete dest data */
   df_free_fbuffer( &Buffer[dest] );

   /* move data */
	memcpy( &Buffer[dest], &Buffer[src], sizeof(Buffer[dest]));

   /* delete src data ( by setting status and NULLing pointers */
   Buffer[src].status = DF_EMPTY;     
   Buffer[src].fheader = NULL;  
   Buffer[src].fdata   = NULL;

   /* refresh display */
   cal_box_stats( dest );
   redraw_dpydata_for_bufinx( dest );
	auto_update_hist_ref_bufinx( dest );  // update histo autoscale
   update_file_save_dialog( dest );

   cal_box_stats( src );
   redraw_dpydata_for_bufinx( src );
	auto_update_hist_ref_bufinx( src );  // update histo autoscale
   update_file_save_dialog( src );
   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_movieshow() - Reads and Display all frames from a 3D Fits file.
**     Syntax: ShowMovie /path/filename bufid
**--------------------------------------------------------------------------------
*/
int do_movieshow( int index, char * par )
{
   int  bufinx,
        fd,
        i,
		  usec,
        rc;
   char pathname[180];
   char dir[128];
   char filename[40];
   char buf[40];
   char * str_ptr;

   /* get the pathname */
   if( (rc=parseString_r( pathname, sizeof(pathname), par, " ", &str_ptr)) != ERR_NONE )
      return rc;

   /* get buf_id */
   if( (bufinx=parseSelection_r( NULL, " ", &str_ptr, buffer_selection)) < 0 )
      return bufinx;

   /* extract path & dir from pathname */
   filename_from_path( filename, pathname, sizeof(filename));
   dir_from_path( dir, pathname, sizeof(dir));

   /*
   **  open file, try to read in a fits file, and close.
   */
   if( (fd = open(pathname, O_RDONLY)) < 0 )
      return  ERR_FILE_READ;

   rc = df_read_fits( fd, dir, filename, &Buffer[bufinx], FALSE, TRUE );

   /*
   ** Update Dpy, Stats with bufinx
   */
   cal_box_stats( bufinx );
   redraw_dpydata_for_bufinx( bufinx );
	auto_update_hist_ref_bufinx( bufinx );  // update histo autoscale
   update_file_save_dialog( bufinx );

   /*----------------------------------------------
   ** read and copy each move frame into the buffer.
   */
   for( i=1; i < Buffer[bufinx].nframes; i++ )
   {
      // read next 'frame' of movie
      movieshow_readimage( fd, &Buffer[bufinx], i );

      // replace filename (showing the index of the frame)
      sprintf( buf, "%s[%d]", filename, i);
      strxcpy( Buffer[bufinx].filename, buf, sizeof(Buffer[bufinx].filename));

      // refresh display
      cal_box_stats( bufinx );
      redraw_dpydata_for_bufinx( bufinx );
	   auto_update_hist_ref_bufinx( bufinx );  // update histo autoscale

      // delay 
		if( Lc.movie_show_delay > 0.0 )
		{
		   usec = Lc.movie_show_delay * 1000000;
			usleep( usec );
		}
   }

   close( fd );

   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  movieshow_readimage() - reads a frame from the 3D fits files into bp.
**--------------------------------------------------------------------------------
*/
int movieshow_readimage(
   int fd,               /* file descriptor of 3D fits file */
   struct df_buf_t * bp, /* buffer to hold data */
   int whichframe        /* which frame to read [0...n] */
)
{
   int rc,
       n,
       image_size,        /* nuber of byte in 1 image */
       numrec,            /* number of fits records to read */
       pixels_per_record, /* number of pixels in each FITS record */
       pixels_left,       /* counter */
       loop,
       i;
   float * dest;
   char *fits_buf;

   /* allocate a read buffer */
   if( NULL == (fits_buf = (char*)malloc( DF_FITS_RECORD_LEN)) )
      return ERR_MEM_ALLOC;
   rc = ERR_NONE;

   /*--------------------------------
   ** seek file pointer to start of image
   */
   image_size = bp->org_size * bp->naxis1 * bp->naxis2;
   n = bp->sizeof_header + (image_size * whichframe);

   if( lseek( fd, n, SEEK_SET) != n )
      { rc = ERR_FILE_READ; goto ldone; }

   /*-------------------------------------
   **
   */
   numrec = ceil( (float) bp->N * bp->org_size / DF_FITS_RECORD_LEN);
   pixels_per_record = DF_FITS_RECORD_LEN / bp->org_size;
   pixels_left = bp->N;
   dest  = bp->fdata;
   for(loop=0; loop < numrec; loop++)
   {
      /*  Read Next Record */
      n = read(fd, (char*)fits_buf, DF_FITS_RECORD_LEN);
      if( DF_FITS_RECORD_LEN != n )
         { rc =  ERR_FILE_READ; goto ldone; }

      if( bp->org_bitpix == DF_BITPIX_CHAR )
      {
         float pixel;
         unsigned char  *src = (unsigned char *)fits_buf;

         /* copy to bp->fdata */
         for( i=0; i < pixels_per_record && pixels_left; i++)
         {
            pixel = *src++;
            *dest++ = pixel;
            pixels_left--;
         }
      }
      else if( bp->org_bitpix == DF_BITPIX_SHORT )
      {
         float pixel;
         short  *src = (short *)fits_buf;

         /* copy to bp->fdata */
         for( i=0; i < pixels_per_record && pixels_left; i++)
         {
            pixel = (short) ntohs(*src++);
            *dest++ = pixel;
            pixels_left--;
         }
      }
      else if( bp->org_bitpix == DF_BITPIX_LONG )
      {
         float pixel;
         int32_t  *src = (int32_t *)fits_buf;

         /* copy to bp->fdata */
         for( i=0; i < pixels_per_record && pixels_left; i++)
         {
            pixel = (int32_t) ntohl(*src++);
            *dest++ = pixel;
            pixels_left--;
         }
      }
      else if( bp->org_bitpix == DF_BITPIX_FLOAT )
      {
         union u_df_lf pixel;
         uint32_t  *src = (uint32_t *)fits_buf;

         /* copy to bp->fdata */
         for( i=0; i < pixels_per_record && pixels_left; i++)
         {
            pixel.l =  ntohl( *src++);
            *dest++ = pixel.f;
            pixels_left--;
         }
      }
      else
         { rc = ERR_INV_FORMAT; goto ldone; }
   }

   /* Apply bzero / bscale to data */
   if( (bp->org_bscale != 1.0) || (bp->org_bzero != 0) )
   {
      dest  = bp->fdata;
      for( i=0; i < bp->N; i++ )
      {
         *dest = (*dest * bp->org_bscale) + bp->org_bzero;
         dest++;
      }
   }

   /* fill in stats */
   df_stats( bp );

ldone:
   free( fits_buf );
   return rc;
}

/*--------------------------------------------------------------------------------
**  do_movieshowdelay() - Puts a small pause 'show movie loop' to slow it down.
**     Syntax:  MovieShowDelay sec 
**--------------------------------------------------------------------------------
*/
int do_movieshowdelay( int index, char *par )
{
   int rc;
   float sec;
	char * str_ptr;
	char buf[25];

   if( (rc = parseFloatR_r( &sec, par, " ", &str_ptr, 0.0, 1.0)) != ERR_NONE)
      return rc;

   if( Lc.movie_show_delay != sec )
	{
		Lc.movie_show_delay = sec;
		sprintf( buf, "%0.3f", Lc.movie_show_delay );
		gtk_entry_set_text( GTK_ENTRY(W.file_open_movie_show_delay), (char*) buf );
	}
   return ERR_NONE;
}
/*--------------------------------------------------------------------------------
**  do_m_execute - executes the macro.
**--------------------------------------------------------------------------------
*/
int do_m_execute( int index, char * par )
{
   char * pathname;
   char * str_ptr = NULL;

   /* get the filename */
   if( NULL == (pathname = strtok_r( par, " ", &str_ptr)) )
      return ERR_INV_FORMAT;

   /* if currently executing, close & remove timer */
   if( Md.fp != NULL )
   {
      g_source_remove( Md.exe_timerid );  /* remove application timer */
      fclose( Md.fp );
   }

   /* open file & start application timer */
   if( NULL == (Md.fp = fopen(pathname, "r")) )
      return ERR_FILE_READ;

   Md.exe_timerid = g_timeout_add( 100L, m_exe_timer_func, NULL );

	cc_printf( W.cmd.main_console, CM_BLUE, 
	   "Starting Macro file %s (fd=%d) \n", pathname, fileno(Md.fp) );
   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_m_filemask - applies the mask to the file list display.
**--------------------------------------------------------------------------------
*/
int do_m_filemask( int index, char * par )
{
   char * mask;
   char * str_ptr = NULL;

   mask = strtok_r( par, " ", &str_ptr);
   if( mask == NULL ) mask = "*";

   strxcpy( Md.filemask, mask, sizeof(Md.filemask));
   gtk_entry_set_text( GTK_ENTRY(Md.file_mask_w), (char*) Md.filemask );

   update_file_list( FALSE, Md.path, Md.filemask, Md.file_store, GTK_TREE_VIEW(Md.file_list)); 
   m_clear_file();
   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_m_load - loads the selected file into the text widget.
**--------------------------------------------------------------------------------
*/
int do_m_load( int index, char * par )
{
   int  rc;
   char * filename;
   char textbuf[150];
   FILE * fp;
   char * str_ptr = NULL;

	GtkTextIter end;

   printf("enter do_m_load\n");
   /* get the filename */
   if( NULL == (filename = strtok_r( par, " ", &str_ptr)) )
      return ERR_INV_FORMAT;

   rc = ERR_NONE;

   /* open file */
   printf("do_m_load opening: %s\n", filename);
   if( NULL == ( fp = fopen(filename, "rt") ) )
      { rc = ERR_FILE_READ; goto ldone; }

   /* Insert file into text buffer */
	gtk_widget_freeze_child_notify( Md.text_view_w );
	gtk_text_buffer_set_text( GTK_TEXT_BUFFER(Md.text_buffer_w), "", -1);
	while ( NULL != fgets( textbuf, sizeof(textbuf), fp ) )
	{
		gtk_text_buffer_get_end_iter( Md.text_buffer_w, &end);
		gtk_text_buffer_insert( Md.text_buffer_w, &end, textbuf, -1);
	}
	gtk_widget_thaw_child_notify( Md.text_view_w);

ldone:
	if (fp ) fclose( fp );
   return rc;
}

/*--------------------------------------------------------------------------------
**  do_m_path - applies the path to the file list display.
**--------------------------------------------------------------------------------
*/
int do_m_path( int index, char * par )
{
   int rc;
   char buf[256];
   char date[10];
   char * cptr;
   char * str_ptr = NULL;

   time_t      now = time((time_t *) 0);
   struct tm * today = gmtime( &now );
   rc = ERR_NONE;

   cptr = strtok_r( par, " ", &str_ptr);
   if( cptr == NULL ) cptr = "$HOME";

   expand_pathname( buf, sizeof(buf), cptr );

   sprintf( date, "%02d%3.3s", today->tm_mday, Month_name[today->tm_mon]);
   str_replace_sub( buf, buf, sizeof(buf), "$DATE", date);

   if( exist_path( buf ) == -1 )
   {
      rc =  ERR_INV_PATH;
      goto ldone;

      if( create_path( buf ) < 0 )
      { rc =  ERR_INV_PATH; goto ldone; }
   }

ldone:
   strxcpy( Md.path, buf, sizeof(Md.path));
   gtk_entry_set_text( GTK_ENTRY(Md.path_w), (char*) Md.path );
   bfp_set_path( &W.bfp_macro, Md.path);
   update_file_list( FALSE, Md.path, Md.filemask, Md.file_store, GTK_TREE_VIEW(Md.file_list));
   m_clear_file();
   return rc;
}

/*--------------------------------------------------------------------------------
**  do_m_refresh - refreshes the file list display, clears the selection, and
**                 clears the file display text area.
**--------------------------------------------------------------------------------
*/
int do_m_refresh( int index, char * par )
{
   update_file_list( FALSE, Md.path, Md.filemask, Md.file_store, GTK_TREE_VIEW(Md.file_list));
   m_clear_file();
   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_m_stop - stop an executing macro.
**--------------------------------------------------------------------------------
*/
int do_m_stop( int index, char * par )
{
   if( Md.fp != NULL )
   {
      fclose( Md.fp );
      Md.fp = NULL;
      g_source_remove( Md.exe_timerid );   /* remove application timer */
		cc_printf( W.cmd.main_console, CM_BLUE, "Terminating current macro execution!\n" );
   }
   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_noisearea() - Set the range of pixels to be include in the Noise Display
**     Syntax: NoiseArea  All|Box
**--------------------------------------------------------------------------------
*/
int do_noisearea( int index, char * par )
{
   int area, dpinx, rc;
	char * str_ptr;

   /* Get area */
   if( par == NULL )
      return ERR_INV_FORMAT;

   if( (area = parseSelection_r( par, " ", &str_ptr, allbox_selection)) < 0 )
      return area;

   /* Get dpinx */
   if( (rc = parseIntR_r( &dpinx, NULL, " ", &str_ptr, 0, Lc.num_dpy-1)) != ERR_NONE)
      dpinx = Lc.active;

   /* update */
   if( Dpy[dpinx].noise_area != area )
   {
      Dpy[dpinx].noise_area = area;
      if( Dpy[dpinx].dpytype == DPYTYPE_NOISE )
         call_dpydata_redraw( dpinx );
   }
   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_noiseautoscale() - Set the autoscale of the noise graph
**     Syntax: NoiseAutoScale off|on
**--------------------------------------------------------------------------------
*/
int do_noiseautoscale( int index, char * par )
{
   int scale, dpinx, rc;
	char * str_ptr;

   /* Get scale */
   if( par == NULL )
      return ERR_INV_FORMAT;

   if( (scale = parseSelection_r( par, " ", &str_ptr, imageautoscale_selection)) < 0 )
      return scale;

   /* Get dpinx */
   if( (rc = parseIntR_r( &dpinx, NULL, " ", &str_ptr, 0, Lc.num_dpy-1)) != ERR_NONE)
      dpinx = Lc.active;

   /* update */
   if( Dpy[dpinx].noise_autoscale != scale )
   {
      Dpy[dpinx].noise_autoscale = scale;
      if( Dpy[dpinx].dpytype == DPYTYPE_NOISE )
         call_dpydata_redraw( dpinx );
   }
   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_noise_size() - Sets the noise channel and size. 
**     Syntax:  NoiseChSize ch size 
**--------------------------------------------------------------------------------
*/
int do_noise_size( int index, char * par )
{
   int dpinx, rc;
   int size;
	char * str_ptr;

   /* Get max & min */
   if( (rc = parseIntR_r( &size, par, " ", &str_ptr, 1, 128)) != ERR_NONE)
      return rc;
   /* Get dpinx */
   if( (rc = parseIntR_r( &dpinx, NULL, " ", &str_ptr, 0, Lc.num_dpy-1)) != ERR_NONE)
      dpinx = Lc.active;

   /* update */
   if( Dpy[dpinx].noise_size != size )
   {
      Dpy[dpinx].noise_size = size;
      if( Dpy[dpinx].dpytype == DPYTYPE_NOISE )
         call_dpydata_redraw( dpinx );
   }
   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_noiseg1range() - Sets the range of the max/min/mean noise graph.
**     Syntax:  NoiseG1Range min max
**--------------------------------------------------------------------------------
*/
int do_noiseg1range( int index, char * par )
{
   int dpinx, rc;
   int min, max;
	char * str_ptr;

   /* Get max & min */
   if( (rc = parseInt_r( &min, par, " ", &str_ptr)) != ERR_NONE)
      return rc;
   if( (rc = parseInt_r( &max, NULL, " ", &str_ptr)) != ERR_NONE)
      return rc;

   if( max <= min )
      return ERR_INV_FORMAT;

   /* Get dpinx */
   if( (rc = parseIntR_r( &dpinx, NULL, " ", &str_ptr, 0, Lc.num_dpy-1)) != ERR_NONE)
      dpinx = Lc.active;

   /* update */
   Dpy[dpinx].noise_autoscale = FALSE;
   if( Dpy[dpinx].noise_g1_min != min || Dpy[dpinx].noise_g1_max != max )
   {
      Dpy[dpinx].noise_g1_min = min;
      Dpy[dpinx].noise_g1_max = max;
      if( Dpy[dpinx].dpytype == DPYTYPE_NOISE )
         call_dpydata_redraw( dpinx );
   }
   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_noiseg2range() - Sets the range of the max/min/mean noise graph.
**     Syntax:  NoiseG2Range min max
**--------------------------------------------------------------------------------
*/
int do_noiseg2range( int index, char * par )
{
   int dpinx, rc;
   int min, max;
	char * str_ptr;

   /* Get max & min */
   if( (rc = parseInt_r( &min, par, " ", &str_ptr)) != ERR_NONE)
      return rc;
   if( (rc = parseInt_r( &max, NULL, " ", &str_ptr)) != ERR_NONE)
      return rc;

   if( max <= min )
      return ERR_INV_FORMAT;

   /* Get dpinx */
   if( (rc = parseIntR_r( &dpinx, NULL, " ", &str_ptr, 0, Lc.num_dpy-1)) != ERR_NONE)
      dpinx = Lc.active;

   /* update */
   Dpy[dpinx].noise_autoscale = FALSE;
   if( Dpy[dpinx].noise_g2_min != min || Dpy[dpinx].noise_g2_max != max )
   {
      Dpy[dpinx].noise_g2_min = min;
      Dpy[dpinx].noise_g2_max = max;
      if( Dpy[dpinx].dpytype == DPYTYPE_NOISE )
         call_dpydata_redraw( dpinx );
   }
   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_noisegraphtype() - Sets the graph type for noise display.
**     Syntax: NoiseGraphType Text|Graph
**--------------------------------------------------------------------------------
*/
int do_noisegraphtype( int index, char * par )
{
   int graphtype, dpinx, rc;
	char * str_ptr;

   /* Get graphtype */
   if( par == NULL )
      return ERR_INV_FORMAT;

   if( (graphtype = parseSelection_r( par, " ", &str_ptr, noisegraphtype_selection)) < 0 )
      return graphtype;

   /* Get dpinx */
   if( (rc = parseIntR_r( &dpinx, NULL, " ", &str_ptr, 0, Lc.num_dpy-1)) != ERR_NONE)
      dpinx = Lc.active;

   /* update */
   if( Dpy[dpinx].noise_graph_type != graphtype )
   {
      Dpy[dpinx].noise_graph_type = graphtype;
      if( Dpy[dpinx].dpytype == DPYTYPE_NOISE )
         call_dpydata_redraw( dpinx );
   }
   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_noisemod() - Sets the modular value for noise display.
**     Syntax: noisemod num
**--------------------------------------------------------------------------------
*/
int do_noisemod( int index, char * par )
{
   int dpinx, mod, rc;
	char * str_ptr;

   /* Get mod */
  if( (rc = parseIntR_r( &mod, par, " ", &str_ptr, 1, 256)) != ERR_NONE)
      return rc;

   /* Get dpinx */
   if( (rc = parseIntR_r( &dpinx, NULL, " ", &str_ptr, 0, Lc.num_dpy-1)) != ERR_NONE)
      dpinx = Lc.active;

   /* update */
   if( Dpy[dpinx].noise_mod != mod )
   {
      Dpy[dpinx].noise_mod = mod;
      if( Dpy[dpinx].dpytype == DPYTYPE_NOISE )
         call_dpydata_redraw( dpinx );
   }
   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_noisemode() - noise mode is 'mod' or 'ch'
**     Syntax: Noisemode  mod|ch
**--------------------------------------------------------------------------------
*/
int do_noisemode( int index, char * par )
{
   int mode, dpinx, rc;
	char * str_ptr;

   /* Get mode */
   if( par == NULL )
      return ERR_INV_FORMAT;

   if( (mode = parseSelection_r( par, " ", &str_ptr, noise_mode_selection)) < 0 )
      return mode;

   /* Get dpinx */
   if( (rc = parseIntR_r( &dpinx, NULL, " ", &str_ptr, 0, Lc.num_dpy-1)) != ERR_NONE)
      dpinx = Lc.active;

   /* update */
   if( Dpy[dpinx].noise_mode != mode )
   {
      Dpy[dpinx].noise_mode = mode;
      if( Dpy[dpinx].dpytype == DPYTYPE_NOISE )
         call_dpydata_redraw( dpinx );
   }
   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_offset_angle() - Sets the tcs offset angle.
**     Syntax:  Offset.angle angel
**--------------------------------------------------------------------------------
*/
int do_offset_angle( int index, char *par )
{
   int rc;
   float angle;
	char * str_ptr;

   if( (rc = parseFloatR_r( &angle, par, " ", &str_ptr, 0.0, 360.0)) != ERR_NONE)
      return rc;

   Lc.offset_angle = angle;
   cal_offset_radec( );
   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_offset_begxy() - Sets the from (x,y).
**     Syntax:  Offset.from x y
**--------------------------------------------------------------------------------
*/
int do_offset_begxy( int index, char *par )
{
   int rc;
   float x, y;
	char * str_ptr;

   if( (rc = parseFloatR_r( &x, par,  " ,", &str_ptr, 0.0, NUM_PIXEL)) != ERR_NONE)
      return rc;
   if( (rc = parseFloatR_r( &y, NULL, " ", &str_ptr, 0.0, NUM_PIXEL)) != ERR_NONE)
      return rc;

   Lc.offset_beg_x = x;
   Lc.offset_beg_y = y;
   cal_offset_radec( );
   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_offset_endxy() - Sets the from (x,y).
**     Syntax:  Offset.to x y
**--------------------------------------------------------------------------------
*/
int do_offset_endxy( int index, char *par )
{
   int rc;
   float x, y;
	char * str_ptr;

   if( (rc = parseFloatR_r( &x, par,  " ,", &str_ptr, 0.0, NUM_PIXEL)) != ERR_NONE)
      return rc;
   if( (rc = parseFloatR_r( &y, NULL, " ", &str_ptr, 0.0, NUM_PIXEL)) != ERR_NONE)
      return rc;

   Lc.offset_end_x = x;
   Lc.offset_end_y = y;
   cal_offset_radec( );
   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_offset_platescale() - Sets the tcs offset platescale.
**     Syntax:  Offset.platescale platescale
**--------------------------------------------------------------------------------
*/
int do_offset_platescale( int index, char *par )
{
   int rc;
   float platescale;
	char * str_ptr;

   if( (rc = parseFloatR_r( &platescale, par, " ", &str_ptr, 0.001, 100.0)) != ERR_NONE)
      return rc;

   Lc.offset_platescale = platescale;
   cal_offset_radec( );
   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_offset_tcs() - Sends the offset commands to the TCS.
**     Syntax:  Offset.TCS
**--------------------------------------------------------------------------------
*/
int do_offset_tcs( int index, char *par )
{
   int   rc;
   char  buf[80];

   rc = ERR_NONE;

	/*----------------------------
	**  Use tcs3 USER.INC command 
	*/
	sprintf( buf, "USER.INC %2.1f %2.1f ", Lc.offset_ra, Lc.offset_dec);

	if( tcs3_com( buf, buf, sizeof(buf), Lc.tcshostname) != ERR_NONE )
		rc = ERR_SOCKET_ERR;

	return rc;
}

/*--------------------------------------------------------------------------------
**  do_offset_tcsab() - Sends the beamswitch offset commands to the TCS.
**     Syntax:  OffsetAB.TCS
**--------------------------------------------------------------------------------
*/
int do_offset_tcsab( int index, char *par )
{
   int   rc;
   char  buf[80];

   rc = ERR_NONE;

	/*----------------------------
	**  Use tcs3 USER.INC command 
	*/
	sprintf( buf, "BEAM.SET %2.1f %2.1f ", Lc.offset_ra, Lc.offset_dec);

	if( tcs3_com( buf, buf, sizeof(buf), Lc.tcshostname) != ERR_NONE )
		rc = ERR_SOCKET_ERR;

	return rc;

}

/*--------------------------------------------------------------------------------
**  cal_offset_radec() -  calculates the RA,DEC offsets.
**--------------------------------------------------------------------------------
*/
void cal_offset_radec( void )
{
   double offset_ra, offset_dec, ns_dir;

   ns_dir = (Lc.image_compass_flipNS ? -1 : 1 );

   offset_ra  = Lc.offset_platescale * (Lc.offset_end_x-Lc.offset_beg_x);
   offset_dec = ns_dir * Lc.offset_platescale * (Lc.offset_end_y-Lc.offset_beg_y);
	rotate_pt( &offset_ra, &offset_dec, offset_ra, offset_dec, Lc.offset_angle);

   Lc.offset_ra  = offset_ra;
   Lc.offset_dec = offset_dec;
}

/*--------------------------------------------------------------------------------
**  do_path() - set the application's default data path.
**     Syntax: Path directory
**--------------------------------------------------------------------------------
*/
int do_path( int index, char * par )
{
   int l;
   char buf[256];
   char date[10];

   time_t      now = time((time_t *) 0);
   struct tm * today = gmtime( &now );

   if( par == NULL ) par = "$HOME";
   expand_pathname( buf, sizeof(buf), par );

   sprintf( date, "%02d%02d%02d", today->tm_year, today->tm_mon+1, today->tm_mday);
   str_replace_sub( buf, buf, sizeof(buf), "$DATE", date);

   if( exist_path( buf ) == -1 )
   {
       if( create_path( buf ) < 0 )
          return ERR_INV_PATH;
   }

   /* append '/' if not already present */
   l = strlen( buf );
   if( buf[l-1] != '/' )
      strcat( buf, "/");

   /* compare the present path & new input, if it is the same then        */
   /* return. We call do_path on every read/save to keep DV in sync       */
   /* with camera software. This check will avoid re-making the file list */
   /* unnecessarily */
   if( strcmp( Lc.path, buf ) == 0)
	return ERR_NONE;

   strxcpy( Lc.path, buf, sizeof(Lc.path));
   strxcpy( Lc.dialog_path, buf, sizeof(Lc.dialog_path));

   bfp_set_path( &W.bfp_data_path, Lc.path); // set browse to path dialog 

   // set file open dialog to path
   gtk_file_chooser_set_current_folder( GTK_FILE_CHOOSER( W.file_open_dialog ), Lc.path ); 

#if TODO
   /* set file access dialogs */
   fad_set_path( &Fad_open, Lc.path );
   fad_set_path( &Fad_save, Lc.path );
#endif

   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_print() - Prints the current display in the drawingArea.
**     Syntax: Print dpinx
**--------------------------------------------------------------------------------
*/
int do_print( int index, char * par )
{
   int  dpinx,
        rc;
   FILE *fp;
   struct dpy_t *dp;
   char buf[60];
	char * str_ptr;

   /* get display index */
   if( (rc = parseIntR_r( &dpinx, par, " ", &str_ptr, 0, Lc.num_dpy-1)) != ERR_NONE)
      dpinx = Lc.active;
   dp = &Dpy[dpinx];

   /* check status */
   if( Buffer[dp->bufinx].status == DF_EMPTY )
      return ERR_NO_DATA;

   /* Open Temper file */
   if( NULL == ( fp = fopen( "/tmp/dv_print.ps", "wt")) )
      return ERR_FILE_WRITE;

   /* Setup postscript file and make letter head */
   if( (rc = print_psheader( fp )) != ERR_NONE)
      goto err;

   /* Print fits header information */
   if( (rc = print_fitsheader( fp, dp )) != ERR_NONE)
      goto err;

   /* Print graphic */
   switch( dp->dpytype )
   {
      case DPYTYPE_IMAGE:
         if( Lc.printertype )
            rc = print_color_image(fp, dp );
         else
            rc = print_gray_image(fp, dp );
         break;

      case DPYTYPE_HISTOGRAM:
         rc = print_histogram(fp, dp );
         break;

      case DPYTYPE_LINECUT:
         rc = print_linecut(fp, dp );
         break;

      case DPYTYPE_NOISE:
         rc = print_noise(fp, dp );
         break;

      case DPYTYPE_XLINECUT:
         rc = print_xcut(fp, dp );
         break;

      case DPYTYPE_AOFIG:
         rc = print_aofig_gray(fp, dp );
         break;

      case DPYTYPE_SA:
         rc = print_sa(fp, dp );
         break;

      case DPYTYPE_SB:
         rc = print_sb(fp, dp );
         break;

      default:
         rc = print_gdummy(fp, dp );
         break;
   }
   if( rc != ERR_NONE)
      goto err;

   fprintf( fp, "showpage\n%%Trailer\n");
   fclose( fp );

   /* Send postscript file to printer */
   if( Lc.printtofile )
   {
		cc_printf( W.cmd.main_console, CM_GRAY, 
			" Created /tmp/dv_print.ps. Did not sent to printer.\n");
   }
   else
   {
      sprintf( buf, "lp -c -s -d%s /tmp/dv_print.ps \n", Lc.printer);
		cc_printf( W.cmd.main_console, CM_GRAY, " Printer command: ");
		cc_printf( W.cmd.main_console, CM_GRAY, buf );
      system( buf );
      chmod( "/tmp/dv_print.ps", S_IRWXU | S_IRWXG | S_IRWXO );
   }
   return rc;

err:
   fclose( fp );
   return rc;
}

/*--------------------------------------------------------------------------------
**  do_printer - Output postscript to this printer
**     Syntax: Pinter name
**--------------------------------------------------------------------------------
*/
int do_printer( int index, char * par )
{
   int rc;
   char buf[40];
	char * str_ptr;

   /* get the pathname */
   if( (rc=parseString_r( buf, sizeof(buf), par, " ", &str_ptr)) != ERR_NONE )
      return rc;

   strxcpy( Lc.printer, buf, sizeof(Lc.printer));
   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_printertype() - Identifies the type of printer.
**     Syntax: PrinterType { BW | Color }
**--------------------------------------------------------------------------------
*/
int do_printertype( int index, char * par )
{
   int ipar;
	char * str_ptr;

   /* get printertype */
   if( (ipar=parseSelection_r( par, " ", &str_ptr, printer_selection)) < 0 )
      return ipar;

   Lc.printertype = ipar;
   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_printtofile() - If TRUE the postscript print file is
**     created, but not sent to printer.
**     Syntax: PrintToFile { Off | On }
**--------------------------------------------------------------------------------
*/
int do_printtofile( int index, char * par )
{
   int ipar;
	char * str_ptr;

   if( (ipar=parseSelection_r( par, " ", &str_ptr, offon_selection)) < 0 )
      return ipar;

   Lc.printtofile = ipar;
   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_ptimagesize() - Sets the image size for the pointer display .
**     Syntax: noisemod num [dpinx]
**--------------------------------------------------------------------------------
*/
int do_ptimagesize( int index, char * par )
{
   int dpinx, size, rc;
	char * str_ptr;

   /* Get size */
   if( (rc = parseInt_r( &size, par, " ", &str_ptr)) != ERR_NONE)
      return rc;

   /* must be odd & force range */
   if( iseven( size )) size++;
   if( size < 11 ) size = 11;
   if( size > 111 ) size = 111; // 111 is an limit so pointer works in a 256x256 window

   /* Get dpinx */
   if( (rc = parseIntR_r( &dpinx, NULL, " ", &str_ptr, 0, Lc.num_dpy-1)) != ERR_NONE)
      dpinx = Lc.active;

   /* update */
   Dpy[dpinx].pt_image_size = size;
   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_ptshowstats() - Toggles displaying of statistics on the pointer DisplayType.
**     Syntax: PtShowStats { off | on } [dpinx]
**--------------------------------------------------------------------------------
*/
int do_ptshowstats( int index, char * par )
{
   int rc, offon, dpinx;
	char * str_ptr;

   /* Get offon */
   if( par == NULL )
      return ERR_INV_FORMAT;

   if( (offon = parseSelection_r( par, " ", &str_ptr, offon_selection)) < 0 )
      return offon;

   /* Get dpinx */
   if( (rc = parseIntR_r( &dpinx, NULL, " ", &str_ptr, 0, Lc.num_dpy-1)) != ERR_NONE)
      dpinx = Lc.active;

   /* update */
   Dpy[dpinx].pt_show_stats = offon;

   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_ptshowlinecut() - Toggles the linecut graph on the pointer DisplayType.
**     Syntax: PtShowlinecut { off | on } [dpinx]
**--------------------------------------------------------------------------------
*/
int do_ptshowlinecut( int index, char * par )
{
   int rc, offon, dpinx;
	char * str_ptr;

   /* Get offon */
   if( par == NULL )
      return ERR_INV_FORMAT;

   if( (offon = parseSelection_r( par, " ", &str_ptr, offon_selection)) < 0 )
      return offon;

   /* Get dpinx */
   if( (rc = parseIntR_r( &dpinx, NULL, " ", &str_ptr, 0, Lc.num_dpy-1)) != ERR_NONE)
      dpinx = Lc.active;

   /* update */
   Dpy[dpinx].pt_show_linecut = offon;

   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_ptshowspex_sn() - Toggles the linecut graph on the pointer DisplayType.
**     Syntax: PtShowSpexSN  { off | on } [dpinx]
**--------------------------------------------------------------------------------
*/
int do_ptshowspex_sn( int index, char * par )
{
   int rc, offon, dpinx;
	char * str_ptr;

   /* Get offon */
   if( par == NULL )
      return ERR_INV_FORMAT;

   if( (offon = parseSelection_r( par, " ", &str_ptr, offon_selection)) < 0 )
      return offon;

   /* Get dpinx */
   if( (rc = parseIntR_r( &dpinx, NULL, " ", &str_ptr, 0, Lc.num_dpy-1)) != ERR_NONE)
      dpinx = Lc.active;

   /* update */
   Dpy[dpinx].pt_show_spex_sn = offon;

   return ERR_NONE;
}
/*--------------------------------------------------------------------------------
**  do_push - pushes command on Lc.cmd_stack. Intended
**            for socket users.
**--------------------------------------------------------------------------------
*/
int do_push( int index, char * par )
{
   int l;
   char *cptr;

   if( par == NULL )
      return ERR_NONE;

   if( Lc.stack_inx >= NUM_CMD_STACK-1 )
      return ERR_INV_RNG;

   if( (l = strlen(par)) > 100 )
      l = 100;
   else
      l++;

   if( (cptr = malloc( l )) == NULL )
      return ERR_MEM_ALLOC;;

   strxcpy( cptr, par, l);

   Lc.cmd_stack[Lc.stack_inx] = cptr;
   Lc.stack_inx++;
   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_quit() - exits the applications
**    Syntax: Quit
**--------------------------------------------------------------------------------
*/
int do_quit( int index, char * par )
{
   int i;

/*   printf("do_quit()\n");*/

   /* deallocated any colormap related resources */
   cm_free();

   /* deallocated any FITS lib resources  */
   for( i = 0; i<NUM_BUFFER; i++ )
      if( Buffer[i].status != DF_EMPTY ) df_free_fbuffer( &Buffer[i] );

   /* remove Networking resources */
#if USE_GIO
      g_io_channel_shutdown( Lc.sock_gio, FALSE, NULL );
      g_io_channel_unref( Lc.sock_gio );
#else
   if( Lc.sock_tag )
      gdk_input_remove( Lc.sock_tag );
#endif
   if( Lc.sock_fd > 0 )
      close( Lc.sock_fd );

   /* OK, now quit GUI. */
   g_source_remove( Lc.app_timerid );
   gtk_main_quit();
   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_read - read a data file from (pathname) into (buf_id).
**  Syntax: Read fullpathname/file buf_id
**--------------------------------------------------------------------------------
*/
int do_read( int index, char * par )
{
   int  bufinx,
        fd,
        rc;
   char pathname[256];
   char dir[180];
   char filename[60];
	char * str_ptr;

   /* get the pathname */
   if( (rc=parseString_r( pathname, sizeof(pathname), par, " ", &str_ptr)) != ERR_NONE )
      return rc;

   /* get buf_id */
   if( (bufinx=parseSelection_r( NULL, " ", &str_ptr, buffer_selection)) < 0 )
      return bufinx;

   /* extract path & dir from pathname */
   expand_pathname( pathname, sizeof(pathname), pathname );
   filename_from_path( filename, pathname, sizeof(filename));
   dir_from_path( dir, pathname, sizeof(dir));

#if 0
   printf("DO READ\n");
   printf("   Pathname: [%s]\n", pathname);
   printf("        dir: [%s]\n", dir);
   printf("   filename: [%s]\n", filename);
   printf("     bufinx: %d\n", bufinx);
#endif

   /*
   **  open file, try to read in a fits file, and close.
   */
   if( (fd = open(pathname, O_RDONLY)) < 0 )
      return  ERR_FILE_READ;

   rc = df_read_fits( fd, dir, filename, &Buffer[bufinx], FALSE, FALSE );
   close( fd );

   /*
   ** Update Dpy, Stats with bufinx
   */
   cal_box_stats( bufinx );
   redraw_dpydata_for_bufinx( bufinx );
   auto_update_hist_ref_bufinx( bufinx );  // update histo autoscale
   update_file_save_dialog( bufinx );

   /* update default path with last used directory */
   if( rc == ERR_NONE )
      do_path( 0, dir );

   return rc;
}

/*--------------------------------------------------------------------------------
**  do_read_xtension - read an data XTENSION from a FITS file.
**  Syntax: ReadXtension fullpathname/file buf_id  xtension_inx
**--------------------------------------------------------------------------------
*/ 
int do_read_xtension( int index, char * par )
{
   int  bufinx,
        xten_inx,
        x,
        fd,
        rc;
   char pathname[180];
   char dir[128];
   char filename[40];
	char * str_ptr;

   /* get the pathname */
   if( (rc=parseString_r( pathname, sizeof(pathname), par, " ", &str_ptr)) != ERR_NONE )
      return rc;
   
   /* get buf_id */
   if( (bufinx=parseSelection_r( NULL, " ", &str_ptr, buffer_selection)) < 0 )
      return bufinx;

   /* get xtension index  */
   if( (rc = parseIntR_r( &xten_inx, NULL, " ", &str_ptr, 0, 31 )) < 0 )
      return rc;

   /* extract path & dir from pathname */
   filename_from_path( filename, pathname, sizeof(filename));
   dir_from_path( dir, pathname, sizeof(dir));
   
#if 0
   printf("Do ReadXtension\n");
   printf("   Pathname: [%s]\n", pathname);
   printf("        dir: [%s]\n", dir);
   printf("   filename: [%s]\n", filename);
   printf("     bufinx: %d\n", bufinx);
   printf("   xten_inx: %d\n", xten_inx);
#endif

   /*
   **  open file, try to read in a fits file, and close.
   */
   if( (fd = open(pathname, O_RDONLY)) < 0 )
      return  ERR_FILE_READ;

   /* 
   ** read the data until you get to the correct XTENSION.
   */
   for( x = 0; x <= xten_inx; x++ )
   {
      printf("reading xten %d ...", x);
      rc = df_read_fits( fd, dir, filename, &Buffer[bufinx], FALSE, FALSE );
      printf("rc = %d \n", rc);
      if( rc != ERR_NONE)
      {
         close( fd );
         return rc;
      }
   }

   close( fd );

   /*
   ** Update Dpy, Stats with bufinx
   */
   cal_box_stats( bufinx );
   redraw_dpydata_for_bufinx( bufinx );
   auto_update_hist_ref_bufinx( bufinx );  // update histo autoscale
   update_file_save_dialog( bufinx );

   /* update default path with last used directory */
   if( rc == ERR_NONE )
      do_path( 0, dir );

   return rc;
}

/*--------------------------------------------------------------------------------
**  do_readfile - reads a file in directory Lc.path
**  Syntax: Readfile filename buffer
**--------------------------------------------------------------------------------
*/
int do_readfile( int index, char * par )
{
   int  bufinx,
        fd, rc;
   char pathname[180];
   char filename[40];
	char * str_ptr;

   /* get the filename */
   if( (rc=parseString_r( filename, sizeof(filename), par, " ", &str_ptr)) != ERR_NONE )
      return rc;

   /* get buf_id */
   if( (bufinx=parseSelection_r( NULL, " ", &str_ptr, buffer_selection)) < 0 )
      return bufinx;

   /* Cat Lc.path & filename */
   cat_pathname( pathname, Lc.path, filename, sizeof(pathname));

#if DEBUG
   printf("DO READFILE\n");
   printf("   Pathname: [%s]\n", pathname);
   printf("   filename: [%s]\n", filename);
   printf("     bufinx: %d\n", bufinx);
#endif

   /*
   **  open file, try to read in a fits file, and close.
   */
   if( (fd = open(pathname, O_RDONLY)) < 0 )
      return  ERR_FILE_READ;

   rc = df_read_fits( fd, Lc.path, filename, &Buffer[bufinx], FALSE, FALSE );
   close( fd );

   /*
   ** Update Dpy, Stats with bufinx
   */
   cal_box_stats( bufinx );
   redraw_dpydata_for_bufinx( bufinx );
   auto_update_hist_ref_bufinx( bufinx );  // update histo autoscale
   update_file_save_dialog( bufinx );

   //do_tcs3cmd( 0, NULL );        // added to help process tcs3 engineering data.
   return rc;
}

/*--------------------------------------------------------------------------------
**  do_readmovie - reads a frame from a 3d Fits from (pathname) into (buf_id).
**  Syntax: ReadMovie fullpathname/file buf_id
**--------------------------------------------------------------------------------
*/
int do_readmovie( int index, char * par )
{
   int  bufinx,
        fd,
        rc;
   char pathname[180];
   char dir[128];
   char filename[40];
	char * str_ptr;

   /* get the pathname */
   if( (rc=parseString_r( pathname, sizeof(pathname), par, " ", &str_ptr)) != ERR_NONE )
      return rc;

   /* get buf_id */
   if( (bufinx=parseSelection_r( NULL, " ", &str_ptr, buffer_selection)) < 0 )
      return bufinx;

   /* extract path & dir from pathname */
   filename_from_path( filename, pathname, sizeof(filename));
   dir_from_path( dir, pathname, sizeof(dir));

   /*
   **  open file, try to read in a fits file, and close.
   */
   if( (fd = open(pathname, O_RDONLY)) < 0 )
      return  ERR_FILE_READ;

   rc = df_read_fits( fd, dir, filename, &Buffer[bufinx], FALSE, TRUE );
   close( fd );

   /*
   ** Update Dpy, Stats with bufinx
   */
   cal_box_stats( bufinx );
   redraw_dpydata_for_bufinx( bufinx );
   auto_update_hist_ref_bufinx( bufinx );  // update histo autoscale
   update_file_save_dialog( bufinx );

   return rc;
}


/*--------------------------------------------------------------------------------
**  do_readsock - reads aa FITS image from the socket. Name is optional.
**  Syntax: ReadSock bufid name
**--------------------------------------------------------------------------------
*/
int do_readsock( int index, char * par )
{
   int  bufinx,
        rc;
	char filename[DF_FITS_LEN_FILENAME];
	char * str_ptr;

   /* get buf_id */
   if( (bufinx=parseSelection_r( par, " ", &str_ptr, buffer_selection)) < 0 )
      return bufinx;

   /* filename is optional */
	if( (rc=parseString_r( filename, sizeof(filename), NULL, " ", &str_ptr)) != ERR_NONE )
	{
	   strxcpy( filename, "unsaved.fts", sizeof(filename));
	}

   rc = df_read_fits( Lc.connect_fd, Lc.path, filename, &Buffer[bufinx], TRUE, FALSE);

   /*
   ** Update Dpy, Stats with bufinx
   */
   cal_box_stats( bufinx );
   redraw_dpydata_for_bufinx( bufinx );
   auto_update_hist_ref_bufinx( bufinx );  // update histo autoscale
   update_file_save_dialog( bufinx );
   return rc;
}

/*--------------------------------------------------------------------------------
**  do_readtextarray - reads a text file from (pathname) into (buf_id).
**  Syntax: ReadTextArray fullpathname/file buf_id naxis1 naxis2
**--------------------------------------------------------------------------------
*/
int do_readtextarray( int index, char * par )
{
   int  bufinx,
		  naxis1, naxis2,
        rc;
   char pathname[180];
   char dir[128];
   char filename[40];
	FILE * fp;
	char * str_ptr;

   /* get the pathname */
   if( (rc=parseString_r( pathname, sizeof(pathname), par, " ", &str_ptr)) != ERR_NONE )
      return rc;

   /* get buf_id */
   if( (bufinx=parseSelection_r( NULL, " ", &str_ptr, buffer_selection)) < 0 )
      return bufinx;

   /* get naxis1 x naxis2 */
	if( (rc = parseIntR_r( &naxis1, NULL, " ", &str_ptr, 0, 1024)) != ERR_NONE)
		return rc;
	if( (rc = parseIntR_r( &naxis2, NULL, " ", &str_ptr, 0, 1024)) != ERR_NONE)
		return rc;


   /* extract path & dir from pathname */
   filename_from_path( filename, pathname, sizeof(filename));
   dir_from_path( dir, pathname, sizeof(dir));

#if DEBUG
   printf("ReadTextArray\n");
   printf("   Pathname: [%s]\n", pathname);
   printf("        dir: [%s]\n", dir);
   printf("   filename: [%s] bufinx=%d naxis1=%d naxis2=%d\n", filename, bufinx, naxis1, naxis2);
#endif

   /*
   **  open file, try to read in a fits file, and close.
   */
   if( (fp = fopen(pathname, "r")) == NULL )
      return  ERR_FILE_READ;

   rc = read_text_array( fp, dir, filename, &Buffer[bufinx], naxis1, naxis2 );
   fclose( fp );

   /*
   ** Update Dpy, Stats with bufinx
   */
   cal_box_stats( bufinx );
   redraw_dpydata_for_bufinx( bufinx );
   auto_update_hist_ref_bufinx( bufinx );  // update histo autoscale
   update_file_save_dialog( bufinx );

   /* update default path with last used directory */
   if( rc == ERR_NONE )
      do_path( 0, dir );

   return rc;
}

int read_text_array_token( FILE *fp, char *buf, int max_len )
{
   int i, c, pre_token;
   char ch;

   i = 0;
   pre_token = 1;  // True
   while( i < (max_len-1) )
   {
      if( fgets( &ch, 2, fp ) == NULL )
         return -1;

      c = ch;
      if( isspace( c ) )
      {
         //printf("s");
         if( pre_token )
         {
            /* do nothing on white space before token */
         }
         else
         {
            /* we are done */
            goto ldone;
         }
      }
      else
      {
         buf[i] = c;
         i++;
         pre_token = 0;  // False
      }
   }

ldone:
   buf[i] = 0;
   return (pre_token ? -1 : 0 );
}

int read_text_array( 
	FILE* fp,                     /* file pointer to read */
	char *dir,                    /* path filename for header info */
	char *filename, 
	struct df_buf_t *bufp,        /* read into this buffer */
	int naxis1,                   /* dim of text file */
	int naxis2
)
{
	int nele, i, rc;
   float *f;
	char * cptr;
   char buf[256];

	rc = ERR_NONE;
   nele = naxis1*naxis2;

   /*-------------------------------------------------
	** initialize buffer & allocate header, data memory.
	*/
   df_free_fbuffer( bufp );

   if( NULL == ( bufp->fdata = (float*) malloc( sizeof(float)*nele )) )
			{ rc = ERR_MEM_ALLOC; goto ldone; }
   if( NULL == ( bufp->fheader = (struct df_fheader_t*) calloc( 1, sizeof(struct df_fheader_t))) )
			{ rc = ERR_MEM_ALLOC; goto ldone; }
   bufp->fheader->next = NULL;
   bufp->naxis1 = naxis1;
   bufp->naxis2 = naxis2;
   bufp->size   = sizeof(float);
	bufp->bitpix = DF_BITPIX_FLOAT;
	bufp->N      = nele;
	bufp->divisor= 1;
	bufp->Nheader= 0;

   bufp->org_size = sizeof(float);
   bufp->org_bitpix = DF_BITPIX_FLOAT;
   bufp->org_bscale = 1;

	strcpy( bufp->directory, dir);
	strcpy( bufp->filename, "unsaved.fts");

   cptr = bufp->fheader->buf;
	cptr = df_build_card( cptr, "SIMPLE",  "T",  "DATA IS IN FITS FORMAT");
	cptr = df_build_card( cptr, "BITPIX",  "-32", "32 BITS FLOAT");
	cptr = df_build_card( cptr, "NAXIS1",  "1", "1st MOST VARYING AXIS");
	cptr = df_build_card( cptr, "NAXIS2",  "2", "2st MOST VARYING AXIS");
	cptr = df_build_card( cptr, "END", NULL, NULL );
	bufp->Nheader= 5;

   f = bufp->fdata;
   for( i=0; i< nele; i++ )
   {
      /* read next token */
      if( (rc = read_text_array_token( fp, buf, sizeof(buf)) != 0 ))
         goto ldone;

      /* remove delimiter, clean up white spaces */
      str_rws( buf, sizeof(buf));
      unpad( buf, ' ');
      if( strlen(buf) < 1 )
		{
			rc = ERR_FILE_READ;
         goto ldone;
		}

      /* convert to float  */
      f[i] = atof( buf );
   }

ldone:
   if( rc )
	{
		df_free_fbuffer( bufp );
	}
   else
	{
		bufp->status = DF_UNSAVED;
		df_stats( bufp );
		sprintf( buf, "Data from %s", filename);
		df_math_fixheader( bufp, buf );
	}

	return rc;
}


/*--------------------------------------------------------------------------------
**  do_rotate() - rotate the image.
**     Syntax: rotate bufid { M90 | P90 | 180 }
**--------------------------------------------------------------------------------
*/
int do_rotate( int index, char * par )
{
   int dest, ops, rc;
	char * str_ptr;

   /* dest */
   if( (dest = parseSelection_r( par, " ", &str_ptr, buffer_selection)) < 0 )
      return dest;

   /* operation - how much to rotate  */
   if( (ops = parseSelection_r( NULL, " ", &str_ptr, rotate_selection)) < 0 )
      return ops;

   /* do it */
   rc = df_buffer_rotate( &Buffer[dest], &Buffer[dest], ops );

   /* refresh display */
   cal_box_stats( dest );
   redraw_dpydata_for_bufinx( dest );
   auto_update_hist_ref_bufinx( dest );  // update histo autoscale
   update_file_save_dialog( dest );
   return rc;
}


/*---------------------------------------------------------------------------
**  do_saobjbin() - Sets the range of rows to be used for the object bin
**     in the Spectra A display. Must be between 0 & NUM_PIXEL. 
**     Syntax:  SAObjBin min max [dpinx]
**---------------------------------------------------------------------------
*/
int do_saobjbin( index, par )
   int index;
   char * par;
{
   int dpinx, 
	    min, max,
       min_binsize,
		 rc;
	char * str_ptr;

	/* Get max & min */
	if( (rc = parseInt_r( &min, par, " ", &str_ptr)) != ERR_NONE)
		return rc;
	if( (rc = parseInt_r( &max, NULL, " ", &str_ptr)) != ERR_NONE)
		return rc;

   if( !INRANGE(0, min, max) || !INRANGE( min, max, NUM_PIXEL-1) ) 
      return ERR_INV_RNG;

	/* Get dpinx */
	if( (rc = parseIntR_r( &dpinx, NULL, " ", &str_ptr, 0, Lc.num_dpy-1)) != ERR_NONE)
			dpinx = Lc.active;

   if( Dpy[dpinx].sa_objbin_min != min ||
       Dpy[dpinx].sa_objbin_max != max )
   {
      Dpy[dpinx].sa_objbin_min = min;
      Dpy[dpinx].sa_objbin_max = max;

      /* Adjust number of row to bin if its below the minimum */
      min_binsize = (((float)Dpy[dpinx].sa_objbin_max -
                            Dpy[dpinx].sa_objbin_min+1) / 20.0) + 0.5;
      if( min_binsize < 1 ) 
			min_binsize = 1;
      if( Dpy[dpinx].sa_rows_per_bin < min_binsize )
         Dpy[dpinx].sa_rows_per_bin = min_binsize;

      if( Dpy[dpinx].dpytype == DPYTYPE_SA )
         call_dpydata_redraw(dpinx);           /* redraw canvas */
   }
   return ERR_NONE;
}

/*---------------------------------------------------------------------------
**  do_sarowsperbin() - Sets the number of rows per bin for Spectra A graph.
**     Syntax: saRowPerBin num [dpinx]
**---------------------------------------------------------------------------
*/
int do_sarowsperbin( index, par )
   int index;
   char * par;
{
   int  rows_per_bin,
	     min;
   int  dpinx, rc;
	char * str_ptr;

	/* Get rows_per_bin */
	if( (rc = parseInt_r( &rows_per_bin, par, " ", &str_ptr)) != ERR_NONE)
		return rc;

	/* Get dpinx */
	if( (rc = parseIntR_r( &dpinx, NULL, " ", &str_ptr, 0, Lc.num_dpy-1)) != ERR_NONE)
			dpinx = Lc.active;

   /* Determine minimum value ( Maximum of 20 spectra lines on graph ) */
   min = (((float)Dpy[dpinx].sa_objbin_max
                  - Dpy[dpinx].sa_objbin_min+1) / 20.0) + 0.5;
   if( min < 1 ) min = 1;

   if( rows_per_bin < min )    /* adjust of lpar is too low */
      rows_per_bin = min;

   if( !INRANGE(1, rows_per_bin, 512))
      return ERR_INV_RNG;

   if( Dpy[dpinx].sa_rows_per_bin != rows_per_bin )
   {
      Dpy[dpinx].sa_rows_per_bin = rows_per_bin;
      if( Dpy[dpinx].dpytype == DPYTYPE_SA )
         call_dpydata_redraw( dpinx );           /* redraw canvas */
   }
   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_sashift() - Specifies the amount of shift in the x axis to be applied
**                 when converting pixel columns to wavelenght.
**  Syntax: sashift num  [dpinx]
**--------------------------------------------------------------------------------
*/
int do_sashift( index, par )
   int index;
   char * par;
{
   int  dpinx,
	     rc;
   int sa_shift;
	char * str_ptr;

	/* Get shift */
	if( (rc = parseInt_r( &sa_shift, par, " ", &str_ptr)) != ERR_NONE)
		return rc;

   if( !INRANGE(-50, sa_shift, 50))
      return ERR_INV_RNG;

	/* Get dpinx */
	if( (rc = parseIntR_r( &dpinx, NULL, " ", &str_ptr, 0, Lc.num_dpy-1)) != ERR_NONE)
			dpinx = Lc.active;

   if( Dpy[dpinx].sa_shift != sa_shift )
   {
      Dpy[dpinx].sa_shift = sa_shift;

      if( Dpy[dpinx].dpytype == DPYTYPE_SA )
         call_dpydata_redraw(dpinx);           /* redraw canvas */
   }
   return ERR_NONE;
}

/*---------------------------------------------------------------------------
**  do_saskybin() - Sets the range of rows to be used for the sky bin
**     in the Spectra A display.
**     Syntax:  SASkyBin min max [dpinx]
**---------------------------------------------------------------------------
*/
int do_saskybin( index, par )
   int index;
   char * par;
{
   int rc;
   int min, max;
	int dpinx;
	char * str_ptr;

	/* Get max & min */
	if( (rc = parseInt_r( &min, par, " ", &str_ptr)) != ERR_NONE)
		return rc;
	if( (rc = parseInt_r( &max, NULL, " ", &str_ptr)) != ERR_NONE)
		return rc;

   if( !INRANGE(0, min, max) || !INRANGE( min, max, NUM_PIXEL-1) )
      return ERR_INV_RNG;

	/* Get dpinx */
	if( (rc = parseIntR_r( &dpinx, NULL, " ", &str_ptr, 0, Lc.num_dpy-1)) != ERR_NONE)
			dpinx = Lc.active;

   if( Dpy[dpinx].sa_skybin_min != min ||
       Dpy[dpinx].sa_skybin_max != max )
   {
      Dpy[dpinx].sa_skybin_min = min;
      Dpy[dpinx].sa_skybin_max = max;

      if( Dpy[dpinx].dpytype == DPYTYPE_SA )
         call_dpydata_redraw(dpinx);           /* redraw canvas */
   }
   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_sastats() - Indicated whether to show graph of stats
**  Syntax: sastats { off | on } [dpinx]
**--------------------------------------------------------------------------------
*/
int do_sastats( index, par )
   int index;
   char * par;
{
   int rc;
   int ipar;
	int dpinx;
	char * str_ptr;

   if( (ipar = parseSelection_r( par, " ", &str_ptr, offon_selection)) < 0 )
      return ipar;

	/* Get dpinx */
	if( (rc = parseIntR_r( &dpinx, NULL, " ", &str_ptr, 0, Lc.num_dpy-1)) != ERR_NONE)
			dpinx = Lc.active;

   if( ipar != Dpy[dpinx].sa_stats )
   {
      Dpy[dpinx].sa_stats = ipar;
      if( Dpy[dpinx].dpytype == DPYTYPE_SA )
         call_dpydata_redraw(dpinx);           /* redraw canvas */
   }
   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_sasubtractsky() - Indicated whether to subtract the sky values for a
**      spectra A graph.
**  Syntax: saSubtractSky { off | on } [dpinx]
**--------------------------------------------------------------------------------
*/
int do_sasubtractsky( index, par )
   int index;
   char * par;
{
   int ipar;
	int dpinx;
	int rc;
	char * str_ptr;

   if( (ipar = parseSelection_r( par, " ", &str_ptr, offon_selection)) < 0 )
      return ipar;

	/* Get dpinx */
	if( (rc = parseIntR_r( &dpinx, NULL, " ", &str_ptr, 0, Lc.num_dpy-1)) != ERR_NONE)
			dpinx = Lc.active;

   if( ipar != Dpy[dpinx].sa_subtractsky )
   {
      Dpy[dpinx].sa_subtractsky = ipar;
      if( Dpy[dpinx].dpytype == DPYTYPE_SA )
         call_dpydata_redraw(dpinx);           /* redraw canvas */
   }
   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_saxscale() - Sets the range of the X axis of a  Spectra A graph.
**     Syntax:  saXScale min max [dpinx]
**--------------------------------------------------------------------------------
*/
int do_saxscale( index, par )
   int index;
   char * par;
{
   int rc;
   int min, max;
   int dpinx; 
	char * str_ptr;

	/* Get max & min */
	if( (rc = parseInt_r( &min, par, " ", &str_ptr)) != ERR_NONE)
		return rc;
	if( (rc = parseInt_r( &max, NULL, " ", &str_ptr)) != ERR_NONE)
		return rc;

   if( !INRANGE(0, min, max-1) || !INRANGE( min+1, max, NUM_PIXEL-1) )
      return ERR_INV_RNG;

	/* Get dpinx */
	if( (rc = parseIntR_r( &dpinx, NULL, " ", &str_ptr, 0, Lc.num_dpy-1)) != ERR_NONE)
			dpinx = Lc.active;

   if( Dpy[dpinx].sa_xmin != min ||
       Dpy[dpinx].sa_xmax != max )
   {
      Dpy[dpinx].sa_xmin = min;
      Dpy[dpinx].sa_xmax = max;

      if( Dpy[dpinx].dpytype == DPYTYPE_SA )
         call_dpydata_redraw(dpinx);           /* redraw canvas */
   }
   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_sayautoscale() - Sets the Y auto scale flag for the sa graph.
**     Syntax:  saYAutoScale { Fixed | Local | Global } [dpinx]
**--------------------------------------------------------------------------------
*/
int do_sayautoscale( index, par )
   int index;
   char * par;
{
   int ipar;
	int dpinx;
	int rc;
	char * str_ptr;

   /* autoscale flag */
   if( (ipar = parseSelection_r( par, " ", &str_ptr, sa_yautoscale_selection)) < 0 )
      return ipar;

	/* Get dpinx */
	if( (rc = parseIntR_r( &dpinx, NULL, " ", &str_ptr, 0, Lc.num_dpy-1)) != ERR_NONE)
			dpinx = Lc.active;

   if( ipar != Dpy[dpinx].sa_yautoscale )
   {
      Dpy[dpinx].sa_yautoscale = ipar;
      if( Dpy[dpinx].dpytype == DPYTYPE_SA )
         call_dpydata_redraw(dpinx);           /* redraw canvas */
   }
   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_sayscale() - Sets the range of the Y axis of a  Spectra A graph.
**                  And set yautoscale to fixed.
**     Syntax:  saYScale min max [dpinx]
**--------------------------------------------------------------------------------
*/
int do_sayscale( index, par )
   int index;
   char * par;
{
   int rc;
   int min, max;
   int dpinx;
	char * str_ptr;

	/* Get max & min */
	if( (rc = parseInt_r( &min, par, " ", &str_ptr)) != ERR_NONE)
		return rc;
	if( (rc = parseInt_r( &max, NULL, " ", &str_ptr)) != ERR_NONE)
		return rc;

   if( !INRANGE(DF_MIN_SIGNED_INT32, min, max-1) || !INRANGE( min+1, max, DF_MAX_SIGNED_INT32) )
      return ERR_INV_RNG;

	/* Get dpinx */
	if( (rc = parseIntR_r( &dpinx, NULL, " ", &str_ptr, 0, Lc.num_dpy-1)) != ERR_NONE)
			dpinx = Lc.active;

	Dpy[dpinx].sa_yautoscale = 0;

   if( Dpy[dpinx].sa_ymin != min || Dpy[dpinx].sa_ymax != max )
   {
      Dpy[dpinx].sa_ymin = min;
      Dpy[dpinx].sa_ymax = max;

      if( Dpy[dpinx].dpytype == DPYTYPE_SA )
         call_dpydata_redraw(dpinx);           /* redraw canvas */
   }
   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_sbdatayrange() - Sets the range of the Y axis of a  Spectra B graph of
**     the obj and sky bin data..
**     Syntax:  SBDataYRange min max [dpinx]
**--------------------------------------------------------------------------------
*/
int do_sbdatayrange( index, par )
   int index;
   char * par;
{
   int rc;
   int min, max;
	int  dpinx;
	char * str_ptr;

	/* Get max & min */
	if( (rc = parseInt_r( &min, par, " ", &str_ptr)) != ERR_NONE)
		return rc;
	if( (rc = parseInt_r( &max, NULL, " ", &str_ptr)) != ERR_NONE)
		return rc;

   if( !INRANGE(DF_MIN_SIGNED_INT32, min, max-1) || !INRANGE( min+1, max, DF_MAX_SIGNED_INT32) )
      return ERR_INV_RNG;

	/* Get dpinx */
	if( (rc = parseIntR_r( &dpinx, NULL, " ", &str_ptr, 0, Lc.num_dpy-1)) != ERR_NONE)
			dpinx = Lc.active;

	Dpy[dpinx].sb_yautoscale = 0;

   if( Dpy[dpinx].sb_data_ymin != min || Dpy[dpinx].sb_data_ymax != max )
   {
      Dpy[dpinx].sb_data_ymin = min;
      Dpy[dpinx].sb_data_ymax = max;

      if( Dpy[dpinx].dpytype == DPYTYPE_SB )
         call_dpydata_redraw(dpinx);           /* redraw canvas */
   }
   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_sbdiffyrange() - Sets the range of the Y axis of a  Spectra B graph of
**     the difference between the obj and sky bins.
**     Syntax:  SBDiffYRange min max [dpinx]
**--------------------------------------------------------------------------------
*/
int do_sbdiffyrange( index, par )
   int index;
   char * par;
{
   int rc;
   int min, max;
	int  dpinx;
	char *str_ptr;

	/* Get max & min */
	if( (rc = parseInt_r( &min, par, " ", &str_ptr)) != ERR_NONE)
		return rc;
	if( (rc = parseInt_r( &max, NULL, " ", &str_ptr)) != ERR_NONE)
		return rc;

   if( !INRANGE(DF_MIN_SIGNED_INT32, min, max-1) || !INRANGE( min+1, max, DF_MAX_SIGNED_INT32) )
      return ERR_INV_RNG;

	/* Get dpinx */
	if( (rc = parseIntR_r( &dpinx, NULL, " ", &str_ptr, 0, Lc.num_dpy-1)) != ERR_NONE)
			dpinx = Lc.active;

	Dpy[dpinx].sb_yautoscale = 0;

   if( Dpy[dpinx].sb_diff_ymin != min ||
       Dpy[dpinx].sb_diff_ymax != max )
   {
      Dpy[dpinx].sb_diff_ymin = min;
      Dpy[dpinx].sb_diff_ymax = max;

      if( Dpy[dpinx].dpytype == DPYTYPE_SB )
         call_dpydata_redraw(dpinx);           /* redraw canvas */
   }
   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_sbobjbin() - Sets the bin range of the object data for a spectra B graph.
**     Syntax:  SBObjBin min max [dpinx]
**--------------------------------------------------------------------------------
*/
int do_sbobjbin( index, par )
   int index;
   char * par;
{
   int rc;
   int dpinx;
   int  min, max;
	char * str_ptr;

	/* Get max & min */
	if( (rc = parseInt_r( &min, par, " ", &str_ptr)) != ERR_NONE)
		return rc;
	if( (rc = parseInt_r( &max, NULL, " ", &str_ptr)) != ERR_NONE)
		return rc;

   if( !INRANGE(0, min, max-1) || !INRANGE( min+1, max, 255) )
      return ERR_INV_RNG;

	/* Get dpinx */
	if( (rc = parseIntR_r( &dpinx, NULL, " ", &str_ptr, 0, Lc.num_dpy-1)) != ERR_NONE)
			dpinx = Lc.active;

   if( Dpy[dpinx].sb_objbin_min != min ||
       Dpy[dpinx].sb_objbin_max != max )
   {
      Dpy[dpinx].sb_objbin_min = min;
      Dpy[dpinx].sb_objbin_max = max;

      if( Dpy[dpinx].dpytype == DPYTYPE_SB )
         call_dpydata_redraw(dpinx);           /* redraw canvas */
   }
   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_sbshow() - Specified which graphs to include with Spectra B.
**     Syntax:  SBShow {D|O|S} [dpinx]
**--------------------------------------------------------------------------------
*/
int do_sbshow( index, par )
   int index;
   char * par;
{
   int rc;
   int mask, i, l;
	char buf[40];
	int dpinx;
	char * str_ptr;

	if( (rc=parseString_r( buf, sizeof(buf), par, " ", &str_ptr)) != ERR_NONE )
		return ERR_INV_RNG;

	/* Get dpinx */
	if( (rc = parseIntR_r( &dpinx, NULL, " ", &str_ptr, 0, Lc.num_dpy-1)) != ERR_NONE)
			dpinx = Lc.active;

   mask = 0x00;
	strupr( par );
	l = strlen(par);
	for( i=0; i<l; i++)
	{
		if( *(par+i) == 'D' ) mask |= 0x01;
		if( *(par+i) == 'O' ) mask |= 0x02;
		if( *(par+i) == 'S' ) mask |= 0x04;
	}

   if( Dpy[dpinx].sb_showgraph != mask )
   {
      Dpy[dpinx].sb_showgraph = mask;
      if( Dpy[dpinx].dpytype == DPYTYPE_SB )
         call_dpydata_redraw(dpinx);           /* redraw canvas */
   }
   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_sbskybin() - Sets the bin range of the sky data for a spectra B graph.
**     Syntax:  SBSkyBin min max [dpinx]
**--------------------------------------------------------------------------------
*/
int do_sbskybin( index, par )
   int index;
   char * par;
{
   int rc;
   int  min, max;
   int  dpinx;
	char * str_ptr;

	/* Get max & min */
	if( (rc = parseInt_r( &min, par, " ", &str_ptr)) != ERR_NONE)
		return rc;
	if( (rc = parseInt_r( &max, NULL, " ", &str_ptr)) != ERR_NONE)
		return rc;

   if( !INRANGE(0, min, max-1) || !INRANGE( min+1, max, 255) )
      return ERR_INV_RNG;

	/* Get dpinx */
	if( (rc = parseIntR_r( &dpinx, NULL, " ", &str_ptr, 0, Lc.num_dpy-1)) != ERR_NONE)
			dpinx = Lc.active;


   if( Dpy[dpinx].sb_skybin_min != min ||
       Dpy[dpinx].sb_skybin_max != max )
   {
      Dpy[dpinx].sb_skybin_min = min;
      Dpy[dpinx].sb_skybin_max = max;

      if( Dpy[dpinx].dpytype == DPYTYPE_SB )
         call_dpydata_redraw(dpinx);           /* redraw canvas */
   }
   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_sbxscale() - Sets the scale (range) of the X axis of the Spectra B graphs.
**     Syntax:  SBXScale min max [dpinx]
**--------------------------------------------------------------------------------
*/
int do_sbxscale( index, par )
   int index;
   char * par;
{
   int rc;
	int dpinx;
   int  min, max;
	char * str_ptr;

	/* Get max & min */
	if( (rc = parseInt_r( &min, par, " ", &str_ptr)) != ERR_NONE)
		return rc;
	if( (rc = parseInt_r( &max, NULL, " ", &str_ptr)) != ERR_NONE)
		return rc;

   if( !INRANGE(0, min, max-1) || !INRANGE( min+1, max, 255) )
      return ERR_INV_RNG;

	/* Get dpinx */
	if( (rc = parseIntR_r( &dpinx, NULL, " ", &str_ptr, 0, Lc.num_dpy-1)) != ERR_NONE)
			dpinx = Lc.active;

   if( Dpy[dpinx].sb_xmin != min ||
       Dpy[dpinx].sb_xmax != max )
   {
      Dpy[dpinx].sb_xmin = min;
      Dpy[dpinx].sb_xmax = max;

      if( Dpy[dpinx].dpytype == DPYTYPE_SB )
         call_dpydata_redraw(dpinx);           /* redraw canvas */
   }
   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_sbyautoscale() - Sets the Y auto scale flag for the sb graph.
**     Syntax:  sbYAutoScale { Off | On } [dpinx]
**--------------------------------------------------------------------------------
*/
int do_sbyautoscale( index, par )
   int index;
   char * par;
{
   int rc;
	int dpinx;
   int ipar;
	char * str_ptr;

   if( (ipar = parseSelection_r( par, " ", &str_ptr, offon_selection)) < 0 )
      return ipar;

	/* Get dpinx */
	if( (rc = parseIntR_r( &dpinx, NULL, " ", &str_ptr, 0, Lc.num_dpy-1)) != ERR_NONE)
			dpinx = Lc.active;

   if( ipar != Dpy[dpinx].sb_yautoscale )
   {
      Dpy[dpinx].sb_yautoscale = ipar;
      if( Dpy[dpinx].dpytype == DPYTYPE_SB )
         call_dpydata_redraw(dpinx);           /* redraw canvas */
   }
   return ERR_NONE;
}

/*---------------------------------------------------------------
**  do_settimer() - set the application timer period. 
**     The application timer controls the data update rate.
**     The range is from 100 to 2000 milliseconds.
**  Syntax: setTimer ms.
**---------------------------------------------------------------
*/
int do_settimer( int index, char * par )
{
   int rc, ms;
   char * str_ptr;

	rc = parseIntR_r( &ms, par, " ", &str_ptr, 100, 2000);
	if( rc != ERR_NONE )
		return rc;

   Lc.timer_ms = ms;
   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_save - save a data buffer from (buf_id) into (pathname).
**  Syntax: Save fullpathname/file buf_id
**--------------------------------------------------------------------------------
*/
int do_save( int index, char * par )
{
   int  bufinx,
        fd,
        rc;
   char pathname[180];
   char dir[128];
   char filename[40];
   char * str_ptr;

   printf("saving [%s] \n", par );

   /* get the pathname */
   if( (rc=parseString_r( pathname, sizeof(pathname), par, " ", &str_ptr)) != ERR_NONE )
      return rc;

   /* get buf_id */
   if( (bufinx=parseSelection_r( NULL, " ", &str_ptr, buffer_selection)) < 0 )
      return bufinx;

   /* extract path & dir from pathname */
   filename_from_path( filename, pathname, sizeof(filename));
   dir_from_path( dir, pathname, sizeof(dir));

#if DEBUG
   printf("DO SAVE\n");
   printf("   Pathname: [%s]\n", pathname);
   printf("        dir: [%s]\n", dir);
   printf("   filename: [%s]\n", filename);
   printf("     bufinx: %d\n", bufinx);
#endif

   /*
   **  open/create file, try to write in a fits file, and close.
   */
   if( (fd = open(pathname, (O_CREAT|O_WRONLY|O_TRUNC), (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH))) < 0 )
      return  ERR_FILE_CREATE;

   rc = df_write_fits( fd, dir, filename, &Buffer[bufinx] );
   close( fd );

   /*
   ** Update Dpy with bufinx
   */
   redraw_dpytitle_for_bufinx( bufinx );

#if TODO
   /* force file selection box to refresh path */
   fad_set_path( &Fad_open, Lc.path );
   fad_set_path( &Fad_save, Lc.path );
#endif

   /* update default path with last used directory */
   if( rc == ERR_NONE )
      do_path( 0, dir );


   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_savefile - save a file in directory Lc.path
**  Syntax: Savefile filename buffer
**--------------------------------------------------------------------------------
*/
int do_savefile( int index, char * par )
{
   int  bufinx,
        rc;
   char pathname[180];
   char filename[40];
   char * str_ptr;

   /* get the filename */
   if( (rc=parseString_r( filename, sizeof(filename), par, " ", &str_ptr)) != ERR_NONE )
      return rc;

   /* get buf_id */
   if( (bufinx=parseSelection_r( NULL, " ", &str_ptr, buffer_selection)) < 0 )
      return bufinx;

   /* Cat Lc.path & filename */
   cat_pathname( pathname, Lc.path, filename, sizeof(pathname));

#if DEBUG
   printf("DO SAVEFILE\n" );
   printf("   Pathname: [%s]\n", pathname);
   printf("   filename: [%s]\n", filename);
   printf("     bufinx: %d\n", bufinx);
#endif
   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_smooth() - dv function to smooth the image.
**     Syntax: smooth src to dest
**--------------------------------------------------------------------------------
*/
int do_smooth( int index, char * par )
{
   int  dest, src, rc;
   char * str_ptr;

   /* src */
   if( (src = parseSelection_r( par, " ", &str_ptr, buffer_selection)) < 0 )
      return src;

   /* to */
   if( (rc = parseSelection_r( NULL, " ", &str_ptr, to_selection)) < 0 )
      return rc;

   /* dest */
   if( (dest = parseSelection_r( NULL, " ", &str_ptr, buffer_selection)) < 0 )
      return dest;

   rc = df_buffer_userfun( &Buffer[dest], &Buffer[src], my_smooth_fun);

   /* redraw destination frame */
   cal_box_stats( dest );
   redraw_dpydata_for_bufinx( dest );
   auto_update_hist_ref_bufinx( dest );  // update histo autoscale
   update_file_save_dialog( dest );
   return rc;
}

int my_smooth_fun(  struct df_buf_t *dest, struct df_buf_t *op1 )
{
   double sum;
   int x, y,
       xi, yi,
       inx,
       n;

   for( y=0; y < op1->naxis2; y++ )
   {
      for( x=0; x < op1->naxis1; x++ )
      {
         sum = 0;
         n = 0;

         for( yi=y-1; yi<=y+1; yi++ )
         {
            for( xi=x-1; xi<=x+1; xi++ )
            {
               if( INRANGE(0, yi, dest->naxis2-1) && INRANGE(0, xi, dest->naxis1-1))
               {
                  sum += dfdataxy( op1, xi, yi);
                  n++;
               }
            }
         }
         inx = y * op1->naxis1 + x;
         dest->fdata[inx] = sum / n;
      }
   }
   return 0;
}

/*--------------------------------------------------------------------------------
**  do_sqrt() - Takes a square root of an image.
**     Syntax: sqrt src to dest
**--------------------------------------------------------------------------------
*/
int do_sqrt( int index, char * par )
{
   int  dest, src, rc;
   char * str_ptr;

   /* src */
   if( (src = parseSelection_r( par, " ", &str_ptr, buffer_selection)) < 0 )
      return src;

   /* to */
   if( (rc = parseSelection_r( NULL, " ", &str_ptr, to_selection)) < 0 )
      return rc;

   /* dest */
   if( (dest = parseSelection_r( NULL, " ", &str_ptr, buffer_selection)) < 0 )
      return dest;

   rc = df_buffer_math( &Buffer[dest], &Buffer[src], &Buffer[src], DF_MATH_SQRT);

   /* redraw destination frame */
   cal_box_stats( dest );
   redraw_dpydata_for_bufinx( dest );
   auto_update_hist_ref_bufinx( dest );  // update histo autoscale
   update_file_save_dialog( dest );
   return rc;
}

/*--------------------------------------------------------------------------------
**  do_statsfixedwh() - Controls the ability to set/fix the wid, hgt of the stats box.
**  Syntax: StatsFixedWH bufid { off | on } [wid hgt]
**--------------------------------------------------------------------------------
*/
int do_statsfixedwh( index, par )
   int index;
   char * par;
{
   int bufinx,
       offon,
       wid, hgt,
       x, y,
       rc;
   char * str_ptr;

   /* Get bufid */
   if( (bufinx = parseSelection_r( par, " ", &str_ptr, buffer_selection)) < 0 )
      return bufinx;

   /* Get offon */
   if( (offon = parseSelection_r( NULL, " ", &str_ptr, offon_selection)) < 0 )
      return offon;

   /* Get wid - optional defaults to current width if not specified. */
   if( (rc = parseIntR_r( &wid, NULL, " ", &str_ptr, 0, NUM_PIXEL)) != ERR_NONE)
      wid = Stats[bufinx].objwid;

   /* Get hgt - optional defaults to current height if not specified. */
   if( (rc = parseIntR_r( &hgt, NULL, " ", &str_ptr, 0, NUM_PIXEL)) != ERR_NONE)
      hgt = Stats[bufinx].objhgt;

   /* if no data in buffer, just return */
   if( Buffer[bufinx].status == DF_EMPTY )
   {
      Stats[bufinx].fixedWH = offon;
      Stats[bufinx].objwid = wid;
      Stats[bufinx].objhgt = hgt;
      return ERR_NONE;
   }

   /* Fixup x, y, wid, hgt to fit inside image */
   if( wid > Buffer[bufinx].naxis1 ) wid = Buffer[bufinx].naxis1;
   if( hgt > Buffer[bufinx].naxis2 ) hgt = Buffer[bufinx].naxis2;

   x = Stats[bufinx].objx;
   if( x > (Buffer[bufinx].naxis1 - wid)) x = Buffer[bufinx].naxis1 - wid;

   y = Stats[bufinx].objy;
   if( y > (Buffer[bufinx].naxis2 - hgt)) y = Buffer[bufinx].naxis2 - hgt;

   /* Erase old box and draw new one */
   draw_xor_box_on_buffer( bufinx, Stats[bufinx].objx,   Stats[bufinx].objy,
                                 Stats[bufinx].objwid, Stats[bufinx].objhgt);
   draw_xor_box_on_buffer( bufinx, x, y, wid, hgt);

   /* set new parameters */
   Stats[bufinx].fixedWH = offon;
   Stats[bufinx].objx   = x;
   Stats[bufinx].objy   = y;
   Stats[bufinx].objwid = wid;
   Stats[bufinx].objhgt = hgt;

   /* Update calculations and update screen */
   cal_box_stats( bufinx );
   redraw_dpytype_stats_for_bufinx(bufinx);

   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_statsobjbox() - set the object position and box size.
**  Syntax: StatsObjBox x, y, wid, hgt bufid
**--------------------------------------------------------------------------------
*/
int do_statsobjbox( int index, char * par )
{
   int x, y, wid, hgt, bufinx, dpinx,
       rc;
   char * str_ptr;

   /* parse parameters: x, y wid, hgt, & bufinx */
   if( (rc = parseIntR_r( &x, par, " ", &str_ptr, 0, NUM_PIXEL-1)) != ERR_NONE)
      return rc;
   if( (rc = parseIntR_r( &y, NULL, " ", &str_ptr, 0, NUM_PIXEL-1)) != ERR_NONE)
      return rc;
   if( (rc = parseIntR_r( &wid, NULL, " ", &str_ptr, 0, NUM_PIXEL-x)) != ERR_NONE)
      return rc;
   if( (rc = parseIntR_r( &hgt, NULL, " ", &str_ptr, 0, NUM_PIXEL-y)) != ERR_NONE)
      return rc;
   if( (bufinx = parseSelection_r( NULL, " ", &str_ptr, buffer_selection )) < 0 )
      return bufinx;

   /* Erase old box and draw new one */
   draw_xor_box_on_buffer( bufinx, Stats[bufinx].objx,   Stats[bufinx].objy,
                                 Stats[bufinx].objwid, Stats[bufinx].objhgt);
   draw_xor_box_on_buffer( bufinx, x, y, wid, hgt);

   /* Store new values in Stats array */
   Stats[bufinx].objx = x;
   Stats[bufinx].objy = y;
   Stats[bufinx].objwid = wid;
   Stats[bufinx].objhgt = hgt;

   /* Update calculations and update screen */
   cal_box_stats( bufinx );  /* cal stats only if buffer is referenced */
   redraw_dpytype_stats_for_bufinx(bufinx);

   // update display with 'box' options.
	for( dpinx=0; dpinx < Lc.num_dpy; dpinx++ )
	{
	   // update 'line cut'  display for 'area==box' 
      if( (Dpy[dpinx].bufinx == bufinx)  && 
	      (Dpy[dpinx].dpytype==DPYTYPE_LINECUT)  && (Dpy[dpinx].lcut_area == 1 /*box*/) )
	          call_dpydata_redraw( dpinx );
		// update 'histogram' display for 'area==box' 
      if( (Dpy[dpinx].bufinx == bufinx)  && 
	      (Dpy[dpinx].dpytype==DPYTYPE_HISTOGRAM)  && (Dpy[dpinx].hist_area == 1 /*box*/) )
	          call_dpydata_redraw( dpinx );
	}
				

   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_statssetsky() - set the sky position and box size to equal the object's.
**  Syntax: StatsSetSky bufinx
**--------------------------------------------------------------------------------
*/
int do_statssetsky( index, par )
   int index;
   char * par;
{
   int bufinx;
   char * str_ptr;

   /* Get bufid */
   if( (bufinx = parseSelection_r( par, " ", &str_ptr, buffer_selection)) < 0 )
      return bufinx;

   Stats[bufinx].skyx = Stats[bufinx].objx;
   Stats[bufinx].skyy = Stats[bufinx].objy;
   Stats[bufinx].skywid = Stats[bufinx].objwid;
   Stats[bufinx].skyhgt = Stats[bufinx].objhgt;

   /* Update calculations and update screen */
   cal_box_stats( bufinx );
   redraw_dpytype_stats_for_bufinx(bufinx);

   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_statsxorline() - sets the position of the xor line.
**  Syntax: Statsxorline xbeg, ybeg, xend, yend  bufid
**--------------------------------------------------------------------------------
*/
int do_statsxorline( int index, char * par )
{
   int xbeg, ybeg, xend, yend, bufinx,
       rc;
   char * str_ptr;

   /* parse parameters:  xbeg, ybeg, xend, yend, & bufinx  */
   if( (rc = parseIntR_r( &xbeg, par, " ", &str_ptr, 0, NUM_PIXEL-1)) != ERR_NONE)
      return rc;
   if( (rc = parseIntR_r( &ybeg, NULL, " ", &str_ptr, 0, NUM_PIXEL-1)) != ERR_NONE)
      return rc;
   if( (rc = parseIntR_r( &xend, NULL, " ", &str_ptr, 0, NUM_PIXEL-1)) != ERR_NONE)
      return rc;
   if( (rc = parseIntR_r( &yend, NULL, " ", &str_ptr, 0, NUM_PIXEL-1)) != ERR_NONE)
      return rc;
   if( (bufinx = parseSelection_r( NULL, " ", &str_ptr, buffer_selection)) < 0 )
      return bufinx;

   /* Erase old box and draw new one */
   draw_xor_line_on_buffer( bufinx, Stats[bufinx].ln_xbeg, Stats[bufinx].ln_ybeg,
                                    Stats[bufinx].ln_xend, Stats[bufinx].ln_yend);
   draw_xor_line_on_buffer( bufinx, xbeg, ybeg, xend, yend);

   /* Store new values in Stats array */
   Stats[bufinx].ln_xbeg = xbeg;
   Stats[bufinx].ln_ybeg = ybeg;
   Stats[bufinx].ln_xend = xend;
   Stats[bufinx].ln_yend = yend;

   return ERR_NONE;
}

/*-----------------------------------
**  do_test
**-----------------------------------
*/
int do_test( int index, char * par )
{
   printf("do_test()\n");

   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_tcshostname() - Name the tcs interface computer
**--------------------------------------------------------------------------------
*/
int do_tcshostname( int index, char *par )
{
   if( par == NULL) 
	{
	   par = "tcs3_host";
	}

   strxcpy( Lc.tcshostname, par, sizeof(Lc.tcshostname));

   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_usefitsanglescale() - If TRUE the TCS offsets are calculated
**     using the Position Angle variable in the FITS header.
**     Syntax: UseFITSPositionAngle { Off | On }
**--------------------------------------------------------------------------------
*/
int do_usefitsanglescale( int index, char * par )
{
   int ipar;
   char * str_ptr;

   if( (ipar=parseSelection_r( par, " ", &str_ptr, offon_selection)) < 0 )
      return ipar;

   Lc.usefitsanglescale = ipar;
   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_UseHex() - Displays pixel values in hex or decmial.
**     Syntax:  UseHex { off | on }
**--------------------------------------------------------------------------------
*/
int do_usehex( int index, char * par )
{
   int ipar;
   char * str_ptr;

   if( (ipar = parseSelection_r( par, " ", &str_ptr, offon_selection)) < 0 )
      return ipar;

   Lc.usehex = ipar;
   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_xcutautoscale() - Set the autoscale of the XLineCut graph
**     Syntax: XCutAutoScale off|on
**--------------------------------------------------------------------------------
*/
int do_xcutautoscale( int index, char * par )
{
   int scale,
       dpinx,
       rc;
   char * str_ptr;

   /* Get scale */
   if( (scale = parseSelection_r( par, " ", &str_ptr, imageautoscale_selection)) < 0 )
      return scale;

   /* Get dpinx */
   if( (rc = parseIntR_r( &dpinx, NULL, " ", &str_ptr, 0, Lc.num_dpy-1)) != ERR_NONE)
      dpinx = Lc.active;

   /* update */
   if( Dpy[dpinx].xcut_autoscale != scale )
   {
      Dpy[dpinx].xcut_autoscale = scale;

      if( Dpy[dpinx].dpytype == DPYTYPE_XLINECUT )
         call_dpydata_redraw( dpinx );
   }
   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_xcutrange() - Sets the range of the XLineCut graph.
**     Syntax:  XCutRange min max
**--------------------------------------------------------------------------------
*/
int do_xcutrange( int index, char * par )
{
   int dpinx,
       rc;
   int min, max;
   char * str_ptr;

   /* Get max & min */
   if( (rc = parseInt_r( &min, par, " ", &str_ptr)) != ERR_NONE)
      return rc;
   if( (rc = parseInt_r( &max, NULL, " ", &str_ptr)) != ERR_NONE)
      return rc;

   if( max <= min )
      return ERR_INV_FORMAT;

   /* Get dpinx */
   if( (rc = parseIntR_r( &dpinx, NULL, " ", &str_ptr, 0, Lc.num_dpy-1)) != ERR_NONE)
      dpinx = Lc.active;

   /* update */
   Dpy[dpinx].xcut_autoscale = FALSE;
   if( Dpy[dpinx].xcut_min != min || Dpy[dpinx].xcut_max != max )
   {
      Dpy[dpinx].xcut_min = min;
      Dpy[dpinx].xcut_max = max;
      if( Dpy[dpinx].dpytype == DPYTYPE_XLINECUT )
         call_dpydata_redraw( dpinx );
   }
   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_xcutset() - Set the coordinates of the XLineCut
**     Syntax: XCutSet xbeg ybeg xend yend
**--------------------------------------------------------------------------------
*/
int do_xcutset( int index, char * par )
{
   long lpar;
   int  xbeg, ybeg, xend, yend, dpinx,
        rc;
   char *cptr;
   char *str_ptr;

   /* Get x, y, wid, hgt */
   if( (NULL == (cptr=strtok_r(par, " ,", &str_ptr))) || (-1 == my_atol(cptr, &lpar)) )
      return ERR_INV_FORMAT;
   xbeg = lpar;
   if( (NULL == (cptr=strtok_r(NULL, " ,", &str_ptr))) || (-1 == my_atol(cptr, &lpar)) )
      return ERR_INV_FORMAT;
   ybeg = lpar;
   if( (NULL == (cptr=strtok_r(NULL, " ,", &str_ptr))) || (-1 == my_atol(cptr, &lpar)) )
      return ERR_INV_FORMAT;
   xend = lpar;
   if( (NULL == (cptr=strtok_r(NULL, " ,", &str_ptr))) || (-1 == my_atol(cptr, &lpar)) )
      return ERR_INV_FORMAT;
   yend = lpar;
   if( !INRANGE(0, xbeg, NUM_PIXEL-1) || !INRANGE(0, ybeg, NUM_PIXEL-1) ||
       !INRANGE(0, xend, NUM_PIXEL-1) || !INRANGE(0, yend, NUM_PIXEL-1) )
      return ERR_INV_RNG;

   /* Get dpinx */
   if( (rc = parseIntR_r( &dpinx, NULL, " ", &str_ptr, 0, Lc.num_dpy-1)) != ERR_NONE)
      dpinx = Lc.active;

   if( Dpy[dpinx].xcut_xbeg!=xbeg || Dpy[dpinx].xcut_ybeg!=ybeg ||
       Dpy[dpinx].xcut_xend!=xend || Dpy[dpinx].xcut_yend!=yend )
   {
       Dpy[dpinx].xcut_xbeg = xbeg;
       Dpy[dpinx].xcut_ybeg = ybeg;
       Dpy[dpinx].xcut_xend = xend;
       Dpy[dpinx].xcut_yend = yend;
       if( Dpy[dpinx].dpytype == DPYTYPE_XLINECUT )
          call_dpydata_redraw( dpinx );
   }
   return ERR_NONE;
}

/********************************************************************************/
/*  Helper Functions                                                            */
/********************************************************************************/

/*--------------------------------------------------------------------------------
**  int qsort_comp_string( i, j ) - Macro Dialog.
**--------------------------------------------------------------------------------
*/
int qsort_comp_string( char **i, char **j)
{
   int r;
   r = strcmp(*i, *j);
   return r;
}

/*--------------------------------------------------------------------------------
**  math_fixstring() - fixes up the math string by padding each element
**  with a space. So the user doesn't have to.
**--------------------------------------------------------------------------------
*/
int math_fixstring(
   char * scr,
   int    scr_size)    /* Max len of scr */
{
   int d, s, count;
   char dest[40];

   d = s = count = 0;
   while( (d < sizeof(dest)-5) && (scr[s] != 0))
   {
      if( (scr[s]=='='|| scr[s]=='+' || scr[s]=='-' || scr[s]=='*' || scr[s]=='/')
           && count < 2 )
      {
         dest[d++] = ' ';
         dest[d++] = scr[s++];
         dest[d++] = ' ';
         count++;
      }
      else
         dest[d++] = scr[s++];
   }
   dest[d] = 0;
   strxcpy( scr, dest, scr_size);
   return ERR_NONE;
}

/****************************************************************************/
/****************************************************************************/

/*--------------------------------------------------------------------------------
**  do_tcs3cmd() - help tcs3 reduce image data. 
**     Syntax:  tcs3cmd
**--------------------------------------------------------------------------------
*/
int do_tcs3cmd( int index, char * par )
{
   int  dpinx,
        x, y, wid, hgt,
        px, py;
   float cx, cy,
         pixel_wid;
   double sum_dx,
          sum_dy,
          sum_d,
          d;
   char * str_ptr;

   struct dpy_t *dp;
   struct df_buf_t *bp;

   /* get display index */
   if( parseIntR_r( &dpinx, par, " ", &str_ptr, 0, Lc.num_dpy-1) != ERR_NONE)
      dpinx = Lc.active;
   dp = &Dpy[dpinx];
   bp = &Buffer[dp->bufinx];

   /* check status */
   if( bp->status == DF_EMPTY )
      return ERR_NO_DATA;

   /* get dimension of array and ckeck it is inside data array */
   x   = Stats[dp->bufinx].objx;
   y   = Stats[dp->bufinx].objy;
   wid = Stats[dp->bufinx].objwid;
   hgt = Stats[dp->bufinx].objhgt;
   if( ((x+wid-1) > bp->naxis1) || ((y+hgt-1) > bp->naxis2) )
      return ERR_INV_RNG;

   /*-------------------------------
   ** loop to find centroid
   */
   sum_dx = sum_dy = sum_d = 0;

   for( py = 0; py < hgt; py++ )
      for( px = 0; px < wid; px++ )
      {
         d = dfdataxy( bp, x+px, y+py);
         sum_d += d;
         sum_dx += d * (px+1);
         sum_dy += d * (py+1);
      }

   cx = x + (sum_dx / sum_d) - 1.0;
   cy = y + (sum_dy / sum_d) - 1.0;

   /* print results to feedback */
	{
	   FILE *fp;
		fp = fopen("tcs3cmd.dat", "a");
		fprintf(fp, "%s  %3.1f %3.1f\n", bp->filename, cx, cy);
		fclose(fp);
	}

   /*-----------------------------------------------------------------
   ** ID pixel on image display by drawing a box around the peak pixel
   */
   pixel_wid = dp->image_zoom > 0 ? dp->image_zoom : 1.0/(1.0+abs(dp->image_zoom));

   px = ((cx - dp->image_offx) * pixel_wid) + (0.5*pixel_wid);
   py = ((cy - dp->image_offy) * pixel_wid) + (0.5*pixel_wid);

   gdk_gc_set_foreground( W.nor_gc, &CM.colors[CM_BLUE] );
   gdk_draw_line( dp->data_drawingarea->window, W.nor_gc,  px-pixel_wid, py, px+pixel_wid, py);
   gdk_draw_line( dp->data_drawingarea->window, W.nor_gc,  px, py-pixel_wid, px, py+pixel_wid);

#if USE_PANGO
   pango_layout_set_text ( W.fixed_font.layout, "C", -1);
	gdk_draw_layout (dp->data_drawingarea->window, W.nor_gc, x, y, W.fixed_font.layout);
#else
   gdk_draw_text( dp->data_drawingarea->window,  W.fixed_font, W.nor_gc,
      px+pixel_wid+2, py+pixel_wid, "C", 1);
#endif

   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_x() - a test command
**     Syntax:  x
**--------------------------------------------------------------------------------
*/
int do_x( int index, char * par )
{
   double a, a1;
   for( a = 0 - (360*2); a< 360*3; a+= 90 )
	{
	   a1 = normalize_0_360( a );
	   printf(" %5.1f   %5.1f \n", a, a1);
	}
   return ERR_NONE;
}

/*--------------------------------------------------------------------------------
**  do_t_gd_info() - show some guidedog data
**     Syntax:  t.gd.info
**--------------------------------------------------------------------------------
*/
int do_t_gd_info( int index, char * par )
{
   int i;

   printf("GUIDEBOX A: \n");
	for( i=0; i<GD_NUM_SLITS; i++ )
	{
	   printf(" { %5.1f, %5.1f, %.0f, %.0f  },   // %s \n", 
		Gd_slit_auto_guidebox_a[i].x,
		Gd_slit_auto_guidebox_a[i].y, 
		Gd_slit_auto_guidebox_a[i].wid,
		Gd_slit_auto_guidebox_a[i].hgt,
		Gd_slit_selection[i]);
	}
   printf("GUIDEBOX B: \n");
	for( i=0; i<GD_NUM_SLITS; i++ )
	{
	   printf(" { %5.1f, %5.1f, %.0f, %.0f  } //  %s \n", 
		Gd_slit_auto_guidebox_b[i].x,
		Gd_slit_auto_guidebox_b[i].y,
		Gd_slit_auto_guidebox_b[i].wid,
		Gd_slit_auto_guidebox_b[i].hgt,
		Gd_slit_selection[i]);
	}
   return ERR_NONE;
}


