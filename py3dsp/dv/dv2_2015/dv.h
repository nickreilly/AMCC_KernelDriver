/***************************************************************************
**  dv.h - application header file for dv
**  It is divided into the following sections:
**     1. other include files.
**     2. defines
**     3. struct definitions 
**     4. Global Variables.    
**     5. function prototypes  
***************************************************************************
  Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php
  Copyright (C) 2012 <NASA IRTF. http://irtfweb.ifa.hawaii.edu>

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

/*************************************************************************/
/*                      1. Other Include Files                           */
/* These are group by function (could be portable/usefile to other apps) */
/*************************************************************************/

#include "./libir2/ir2.h"    // DV's own copy of libir2 (general purpose C function)

#include "cm.h"              // colormap defines, structure, and prototypes
#include "cmdcon.h"          // code to provide command console widget.
#include "sclist.h"          // code to provde a single colmunm list widget.

#include "xpm.h"             // defines some pixmaps

#define USE_GIO 1            /* use g_io rather that gtk_input_() */

/*************************************************************************/
/*                      2. DV defines                                    */
/*************************************************************************/

#define APP_VERSION   "2015.09"
#define APP_NAME      "DV2"
#define APP_HOME_VAR  "DV_HOME"

#if USE_PANGO
//#define NOR_FONT_NAME "DejaVu Sans Mono 7"     // 6x12
//#define NOR_FONT_NAME "Liberation Mono 8"     // 6x13
#define NOR_FONT_NAME "monospace 8"             // 6x13
#define BIG_FONT_NAME "monospace 10"
#else
#define NOR_FONT_NAME "-*-fixed-*-*-*-*-12-*-*-*-*-*-*-*"    //   7x14
#define BIG_FONT_NAME "-*-fixed-*-*-*-*-20-*-*-*-*-*-*-*"    //  19x21 
#endif

#define DPYINX_DATA_KEY           "dpyinx_data_key"           // use for g_object_set_data()
#define MENU_FROM_SELECTION_KEY   "menu_from_selection_key"   // use for g_object_set_data()

#define U_CHAR                0    // specifies units for drawtext() and friends.
#define U_PIXEL               1

#define MAX_DPY              25       /* max number of display area for data */
#define NUM_BUFFER            8       /* number of data buffer */
#define NUM_FUN_BUT           6       /* number of function macro buttons */

#define NUM_PIXEL          4096       /* maximum size of image (4096x4096) */

#define NUM_DPYTYPE          11
#define DPYTYPE_IMAGE         0       /* values for dpyinx */
#define DPYTYPE_HEADER        1
#define DPYTYPE_HISTOGRAM     2
#define DPYTYPE_LINECUT       3
#define DPYTYPE_XLINECUT      4
#define DPYTYPE_NOISE         5
#define DPYTYPE_POINTER       6
#define DPYTYPE_STATS         7
#define DPYTYPE_AOFIG         8
#define DPYTYPE_SA            9
#define DPYTYPE_SB           10

#define NUM_CMD_STACK        10
#define HIST_NUM_BIN        100       /* maximum number of histogram bins */
#define NUM_MATH_EQU          3       /* number of math equation entries  */

#define COORD_CSHELL_ANGLE    270.0
#define COORD_CSHELL_PSCALE   0.20
#define COORD_NSFCAM_ANGLE    90.0    // now for nsfcam2, nsfcam1 was 0.0
#define COORD_NSFCAM_PSCALE   0.04    // now for nsfcam2, nsfcam1 was 0.30
#define COORD_SPEX_ANGLE      0.0
#define COORD_SPEX_PSCALE     0.12    // guidedog's platescale

#define DV_SOCK_TIMEOUT_MS   1000

#define INST_FLAVOR_BIGDOG   0
#define INST_FLAVOR_GUIDEDOG 1
#define INST_FLAVOR_SMOKEY   2
#define INST_FLAVOR_MORIS    3
#define NUM_INST_FLAVOR      4

#define AOFIG_NUM_ELEMENTS   36    /* number of elements (ie:sensors) in the ao */

#define NUM_AOFIG_FORMAT      3
#define AOFIG_FORMAT_TEXT     0
#define AOFIG_FORMAT_DM       1
#define AOFIG_FORMAT_SENSOR   2

/*************************************************************************/
/*                      3. DV Structures                                 */
/*************************************************************************/

struct array_t
{
   int x;
	int y;
	int wid;
	int hgt;
};

struct farray_t
{
   float x;
	float y;
	float wid;
	float hgt;
};

/*----------------------------------------------------------------------------
** dpy_t - struct for managing display
*/
struct dpy_t
{
   GtkWidget * title_drawingarea;     /* Drawing area for Text, ie: names */
   GtkWidget * data_drawingarea;      /* Drawing data of data displays */
   GtkAdjustment * vadj;              /* vertical adjustments */
   GtkAdjustment * hadj;              /* horizontial adjustments */

   int      bufinx;                   /* display data in this buffer index */
   int      dpytype;                  /* type of display: image, header, graph.. */

                                      /* HEADER Display Type parameters */
   int      header_row;               /* Starting row & column position */
   int      header_col;               /* 0 indicates first position */

                                      /* IMAGE Display Type parameters */
   int      image_autoscale;          /* Fixed or Auto scaling */
   int      image_zoom;               /* zoom factor */
   float    image_min;                /* min & max range */
   float    image_max;
   int      image_offx;               /* offset variables */
   int      image_offy;

                                      /* HISTOGRAM Display Type parameters */
   int      hist_bin;                 /* Number of bins for histogram */
   int      hist_area;                /* 0 (full array) or 1 (box)    */
	int      hist_auto_range;          /* updates when the image update */

                                      /* LINECUT Display Type parameters */
   int      lcut_area;                /* 0 (full array) or 1 (box)    */
   int      lcut_autoscale;           /* Fixed or Auto scaling */
   float    lcut_min;                 /* min & max range */
   float    lcut_max;
   int      lcut_x;                   /* X Axis of line cut */
   int      lcut_y;                   /* Y Axis of line cut */

                                    /* Parameters for type SPECTRA A        */
   int sa_objbin_min;               /* The columns making up the object.    */
   int sa_objbin_max;               /* ( 0 to naxis1).                      */
   int sa_rows_per_bin;              /* Number of spectra line to show.      */
   int sa_skybin_min;               /* The columns making up the sky.       */
   int sa_skybin_max;               /* ( 0 to naxis1).                      */
   int sa_subtractsky;              /* Do you wish to subtract the sky      */
   int sa_xmin;                     /* The range of the X axis of graph     */
   int sa_xmax;                     /*   ( 0 to naxis2 )                    */
   int32_t sa_ymin;                    /* The range of the Y axis of graph     */
   int32_t sa_ymax;                    /* (MAX_SIGNED_INT32 to MIN_SIGNED_INT32) */
   int  sa_yautoscale;              /* Type of autoscaling: none, indiv. gobol */
   int  sa_shift;                   /* Amount of pixel shift                */
   int  sa_stats;                   /* Show Graph(off) or stats(on)         */

   int sb_showgraph;                /* Which graph to display O,S,D = 4,2,1 */
   int sb_objbin_min;               /* The range of object to bin for obj.  */
   int sb_objbin_max;               /* ( 0 to naxis2 )                      */
   int sb_skybin_min;               /* The range of object to bin for sky.  */
   int sb_skybin_max;               /* ( 0 to naxis2 )                      */
   int sb_yautoscale;               /* Autoscale the Y axis of the graph    */
   int32_t sb_diff_ymin;               /* The Y axis range for the diff graph. */
   int32_t sb_diff_ymax;               /* (MAX_SIGNED_INT32 to MIN_SIGNED_INT32) */
   int32_t sb_data_ymin;               /* The Y axis range for the data graphs.*/
   int32_t sb_data_ymax;               /* (MAX_SIGNED_INT32 to MIN_SIGNED_INT32) */
   int sb_xmin;                     /* The range of the X axis of graph     */
   int sb_xmax;                     /*   ( 0 to naxis1 )                    */

   int      noise_row;
   int      noise_col;
   int      noise_mode;               /* 0 (mode) or 1 (ch/size)     */
   int      noise_area;               /* 0 (full array) or 1 (box)    */
   int      noise_mod;                /* noise modular variable       */
   int      noise_ch;                 /* noise channel       */
   int      noise_size;               /* noise channel size  */
   int      noise_graph_type;         /* 0 (text) or 1 (graphics)     */
   int      noise_autoscale;          /* autoscale the graphs         */
   float    noise_g1_min;
   float    noise_g1_max;
   float    noise_g2_min;
   float    noise_g2_max;
   
   int      xcut_output_data;         /* Flag to tell xcut to output data to file */
   int      xcut_fit_data;            /* Flag to tell xcut to do equation fit */
   int      xcut_autoscale;           /* Fixed or Auto scaling        */
   int      xcut_xbeg;                /* Beginning end point          */
   int      xcut_ybeg;
   int      xcut_xend;                /* Ending end point             */
   int      xcut_yend;
   float    xcut_min;                 /* min & max range              */
   float    xcut_max;

   int      pt_image_size;            /* size of image: 11 to 31 */
   int      pt_show_stats;            /* show stats */
   int      pt_show_linecut;          /* show line cut */
   int      pt_show_spex_sn;          /* show spex signal to noise estimate */
   
   int      aofig_data;               /* data set in row or col */
   int      aofig_x;                  /* [x, y] of 1st data item */
   int      aofig_y;
   int      aofig_format;             /* Show data as: text, DM, or Sensor */
}; 
   
struct stats_t {
   int   objx;                        /* object box x position               */
   int   objy;                        /* object box y position               */
   int   objwid;                      /* object box width                    */
   int   objhgt;                      /* object box height                   */
   float objmin;                      /* object minimum pixel value          */
   float objmax;                      /* object maximum pixel value          */
   float objsum;                      /* object sum of values                */
   float objmean;                     /* object mean of values               */
   float objstd;                      /* object stddev                       */
   int   objN;                        /* obj number of data points           */
   int   skyx;                        /* sky box x position                  */
   int   skyy;                        /* sky box y position                  */
   int   skywid;                      /* sky box width                       */
   int   skyhgt;                      /* sky box height                      */
   float skymin;                      /* sky minimum pixel value             */
   float skymax;                      /* sky maximum pixel value             */
   float skysum;                      /* sky sum of values                   */
   float skymean;                     /* sky mean of values                  */
   float skystd;                      /* sky variance                        */
   int   skyN;                        /* sky number of data points           */
   float redmin;                      /* reduce minimum pixel value          */
   float redmax;                      /* reduce maximum pixel value          */
   float redstd;                      /* reduce variance                     */
   float redsum;                      /* reduce sum of values                */
   int   redN;                        /* reduce num of data points           */

   int   fixedWH;                     /* T/F flag to fixed the wid&hig of objbox */

   int   ln_xbeg;                     /* This is not needed for stats but    */
   int   ln_ybeg;                     /* I will keep the starting and        */
   int   ln_xend;                     /* ending point of the XOR line in     */
   int   ln_yend;                     /* the same struct as the object box   */
};

/*----------------------------------------------------------------------------
** lc_t - application local variables
*/

struct lc_t
{  
   char * program_name;          /* points to arg[0] - command name */
   int  debug;                   /* debug flag */
   int  app_timerid;             /* ID to the application timer */
   int  timer_ms;                /* controls period of gtk timer */
   char *app_home;               /* Value of App_home_env_var   */

   // command line options
   int  num_dpy;                 /* number of display area for data  */
   int  dpy_r;                   /* nrow & columns for main dpys */
   int  dpy_c;
   int  dpy_default_wid;         /* default W x H  for main dpys */
	int  dpy_default_hgt;
	int  use_config_window;       // config widget on base or window? 
	int  mga_enable;              // Shows the "MORIS GuideBox Adj" button on the Offset tab.

   // configuration 
   char path[256];                    /* application's default path  */
   char dialog_path[256];             /* dialog box's default path  */
   int  file_read_bufinx;             /* selection for the File_read_buffer option menu */
   int  active;                       /* the active display */
   int  divbydivisor;                 /* divide data by divisor */
   char printer[80];                  /* name of printer */
   int  printertype;                  /* type of printer */
   int  printtofile;                  /* file to prevent sending postscript file to lp */
   char tcshostname[40];              /* Hostname of TCS */
   int  usehex;                       /* display pixel values as hex number in image display */
   int  cminverse;                    /* is the colormap inversed */
   int  dvport;                       /* port number used (for user display only) */
   int  image_compass_show;           /* draw compass on images */
   int  image_compass_flipNS;         /* flip NS on the image compass */
   int  image_show_gbox;              /* show guidebox information on images */
   int  inst_flavor;                  /* tailor some functions for this instrument */
   int  gbox_cmd_right_click;         /* this flag make DV issue a gbox cmd on image right click */


   int  usefitsanglescale;            /* Use FIT position angle & scale for tcs offset calculation */
   float offset_beg_x;                /* Beg (x,y) */
   float offset_beg_y;
   float offset_end_x;                /* End (x,y) */
   float offset_end_y;
   float offset_angle;                /* rotation angle */
   float offset_platescale;           /* plate scale in arcsec/pixel */
   float offset_ra;                   /* offset as a (ra,dec) */
   float offset_dec;
	float movie_show_delay;            /* delay in seconds, for "movie show" loop */

   int  sock_fd;                      /* file description for accepting connections */
#if USE_GIO
   GIOChannel * sock_gio;            /* gio channel for sock_fd */
#else
   int  sock_tag;             /* return tag from gtk_input_add(), need tag to remove resource */
#endif
   int  connect_fd;                   /* The accepted socket connection */


   char *cmd_stack[NUM_CMD_STACK];    /* a command stack */
   int  stack_inx;                    /* index to next free stack element */
};

struct noise_t
{
   float max;
   float min;
   int32_t N;
   double mean;
   double std;
};

struct xcut_buf_t {                    /* Used by draw_xcut() */
   int  x;
   int  y;
   float value;
};

struct hist_info {                     /* Histogram Information */
   int pixel_low;                     /* number of pixel are below bin[0]         */
   int pixel_high;                    /* number of pixel above bin[num_of_bins-1] */
   int pixel_graph;                   /* number of pixel in the graph */
   int pixel_total;                   /* total pixels */
   int max_bin_value;                 /* max value of the bins[] */

   int  num_of_bins;                   /* number of bins */
   int bins[HIST_NUM_BIN];            /* array of bins */
};

struct lcut_info {                     /* Linecut Information */
   int xmin;                           /* beginning and ending pixel for x axis */
   int xmax;
   int ymin;                           /* beginning and ending pixel for y axis */
   int ymax;

   int xaxis;                          /* where the line cut occures */
   int yaxis;

   float range_min;                    /* value scale for graph */
   float range_max;
};

/* need polygon tables for drawing figure of DM / Sensors */
typedef struct
{
   int n;
   GdkPoint pt[5];
} polygon_t;

/* --------- remaining structures are related to GTK ---------- */

#if USE_PANGO
struct font_info_t            // font, pango layout
{
   PangoFontDescription * font;    // font reference
   PangoLayout          * layout;  // Pango reference
   int                  wid;
   int                  hgt;
   int                  ascent;
   int                  descent;
};
#endif

struct ds_t                  // structure used for drawing text/graphics in drawing area
{
   GdkDrawable *window;
   GdkGC       *gc;
   GdkColor *ifg;           /* foregroud color for imagetext */
   GdkColor *ibg;           /* background color for imagetext */

   int     da_wid;   /* Width & Height of Drawing Area */
   int     da_hgt;
#if USE_PANGO
   struct font_info_t *font;    /* Font & its size */
#else
   GdkFont *font;
#endif
   int     char_wid;
   int     char_hgt;

};

// macro_t - widgets and variable to support the macro function
struct macro_t            /* Widgets and variables for macro dialog & execution support */
{
   GtkWidget     * path_w;           /* entry line widget to hold directory path */
   GtkWidget     * file_mask_w;      /* entry line widget to hold filename mask */
   GtkWidget     * file_list;        /* list of files */
   GtkListStore  * file_store;
   GtkWidget     * file_text_w;      /* non-editable text widget to display file contents */
	GtkTextBuffer * text_buffer_w;    /* buffer for file contents */
	GtkWidget     * text_view_w;      /* text view for file contents */
   
   char path[80];                      /* current macro path */
   char filemask[30];                  /* filemask of filelist */
   char *filename;                     /* name of currently selected file */

   gint exe_timerid;                   /* Execution timer ID */
   FILE * fp;                          /* file pointer of macro file to execute */
}; 

struct browseForPath_t
{
   GtkWidget   * dialog_window;   /* reference to dialog window */
   GtkWidget   * path_cbox_w;     /* combo box showing path & parents */
   int           path_cbox_hid;   /* hid for path_cbox_w */
	struct sclist_view_t *sclist;  /* List view widget */
   GtkWidget   * entry;           /* Shows which directory is selected */
   GtkWidget   * ok_button;       /* The ok button */

   struct dirlist_t dirlist;    /* direct string & list of parents */
};

struct moris_guide_adj_t          // moris_guiding_adjustment dialog, aka mga
{
   GtkWidget   * dialog_window;   /* reference to dialog window */
   GtkWidget   * bufinx;          /* buffer to use */
   GtkWidget   * center_method;   /* how to determine center  */
   GtkWidget   * cal_label;       /* label to provide feedback for Caculate Offset button  */
   GtkWidget   * apply_label;     /* label to provide feedback for Apply Offsets   button  */

	// some data
	int data_ok;                   // Apply button cb will on on good data.
	double beg_xy[2];              // guidedog FROM xy
	double end_xy[2];              // guidedog TO xy
	double gd_arcsec_pixel;        // guidedog platescale
	double gd_rot_deg;             // guidedog platescale
	double moris_arcsec_pixel;     // Moris platescale
	double moris_rot_deg;          // moris rotation value

	double tcs_offset[2];           // TCS offset to apply
	double moris_adj[2];            // Moris GuideBox Adjustments to apply
};

struct w_t
{
#if USE_PANGO
   struct font_info_t  fixed_font;    /* font for drawing area repaint functions */
#else
   GdkFont * fixed_font;
#endif
   int      fixed_font_wid;        // only need for NON-PANGO
   int      fixed_font_hgt;

   GdkGC * nor_gc;              /* Normal GC - an application wide GC */
   GdkGC * xor_gc;              /* XOR GC    - an application wide GC for rubberbanding */

   GdkPixmap *icon_pixmap;

   GtkStyle  * style_Default,    /* applicaton's default style */
             * style_BlackBG, 
             * style_YellowGray, 
             * style_GreenGray, 
             * style_BlueGray, 
             * style_Red, 
             * style_Green; 

	GtkWidget *command_feedback;    // label on base window for feedback of commands.

   // MAIN BAR 
	GtkWidget *colormap;            // DA to display colormap 
	GtkWidget *colormap_cbox;       // cbox to select colormap 
	int        colormap_cbox_hid;  

   // Display Option Widgets 
	GtkWidget *dpyactive_w[MAX_DPY];  // which display is currently active
	GtkWidget *dpytype_w;             // What is display in the Dpy[] canvase.
	int        dpytype_hid;                                                     
	GtkWidget *dpybuf_w[NUM_BUFFER];  // Which buffer to link to this Dpy[] 
	GtkWidget *dpy_notebook;          // Hold widget for differenc DPY: image, histogram, ...

	GtkObject *image_zoom; 
	int        image_zoom_hid;        // hid for image_zoom widget.
	GtkWidget *image_range_min; 
	GtkWidget *image_range_max; 
	GtkWidget *image_autoscale[2];   //  {fixed|auto} widgets for radio buttons
  
	GtkWidget *hist_area[2];         //  {all|box} widgets for radio buttons
	GtkWidget *hist_bins;            //  number of bins
	GtkWidget *hist_auto_range;      //  auto scale range via image.
	GtkWidget *hist_range_min;
	GtkWidget *hist_range_max;

	GtkWidget *lcut_x;
	GtkWidget *lcut_y;
	GtkWidget *lcut_area[2];         // {all|box} widgets for radio buttons
	GtkWidget *lcut_autoscale[2];    // {fixed|auto} widgets for radio buttons
	GtkWidget *lcut_range_min;
	GtkWidget *lcut_range_max;

	GtkWidget *xcut_beg;
	GtkWidget *xcut_end;
	GtkWidget *xcut_range_min;
	GtkWidget *xcut_range_max;
	GtkWidget *xcut_autoscale[2];    // {fixed|auto} widgets for radio buttons

	GtkWidget *noise_g1_range_min;
	GtkWidget *noise_g1_range_max;
	GtkWidget *noise_g2_range_min;
	GtkWidget *noise_g2_range_max;
	GtkWidget *noise_autoscale[2];    // {fixed|auto} widgets for radio buttons
	GtkWidget *noise_mode[2];         // {mod|chSize} widgets for radio buttons
	GtkWidget *noise_area[2];         // {all|box} widgets for radio buttons
	GtkWidget *noise_graph_type[2];   // {text|graph} widgets for radio buttons
	GtkWidget *noise_mod;             // number of nodes to be drawn
	GtkWidget *noise_size;         // number of channels

	GtkWidget *stats_fixedwh;         // Wid/Hgt button
	GtkWidget *stats_wh_entry;        // Wid/Hgt Entry.

	GtkWidget *pt_image_size;    
	GtkWidget *pt_show_stats;    
	GtkWidget *pt_show_linecut;  
	GtkWidget *pt_show_spex_sn;          

	GtkWidget *aofig_format;          // combobox
	      int  aofig_format_hid;
	GtkWidget *aofig_data;            // combobox 
	      int  aofig_data_hid;                                                     
	GtkWidget *aofig_x;
	GtkWidget *aofig_y; 

	GtkWidget *sa_objbin_label;
	GtkWidget *sa_objbin;
	GtkWidget *sa_skybin_label;
	GtkWidget *sa_skybin;
	GtkWidget *sa_rows_per_bin;
	GtkWidget *sa_shift;
	GtkWidget *sa_subtractsky;
	GtkWidget *sa_stats;
	GtkWidget *sa_yautoscale;
	      int  sa_yautoscale_hid;
	GtkWidget *sa_yscale;
	GtkWidget *sa_xscale;

	GtkWidget *sb_objbin;                  /* entry */
	GtkWidget *sb_objbin_label;            /* label */
	GtkWidget *sb_skybin;                  /* entry */
	GtkWidget *sb_skybin_label;            /* label */
	GtkWidget *sb_show_diff;               /* checkbutton */
	GtkWidget *sb_show_obj;                /* checkbutton */
	GtkWidget *sb_show_sky;                /* checkbutton */
	GtkWidget *sb_yautoscale;              /* checkbutton */
	GtkWidget *sb_diffyrange;              /* entry */
	GtkWidget *sb_datayrange;              /* entry */
	GtkWidget *sb_xscale;                  /* entry */

   // Math Widgets 
	GtkWidget *math_equ_w[NUM_MATH_EQU];  // entry to hold math equation 
	GtkWidget *math_copy_from_bufinx;     // copy $from to $to
	GtkWidget *math_copy_to_bufinx;
	GtkWidget *math_clear_bufinx;         // clear $buf
	GtkWidget *math_rotate_bufinx;        // rotate $buf $ops
	GtkWidget *math_rotate_ops;     

   // Setup Widgets 
	GtkWidget *dvport_w;                   // label to show Dv's port number
	GtkWidget *div_by_divisor;             // check button
	GtkWidget *tcs_system;                 // combobox + _hid
	      int  tcs_system_hid;
	GtkWidget *tcs_hostname;               // tcs hostname
	GtkWidget *printer_name;               // printer hostname
	GtkWidget *printer_type;               // combobox + _hid
	      int  printer_type_hid; 
	GtkWidget *print_to_file;              // print to file
	GtkWidget *path;                       // DV's default path
	GtkWidget *cm_swap_rgb;                // ColorMap Swap RGB  
	GtkWidget *inst_flavor;                // combobox + _hid
	      int  inst_flavor_hid; 
	GtkWidget *image_compass; 
	GtkWidget *image_compass_flip_NS; 
	GtkWidget *image_show_gbox;
	GtkWidget *gbox_cmd_right_click;

   // Offset Widgets
   GtkWidget * use_fits_angle_scale;
   GtkWidget * offset_angle;
   GtkWidget * offset_platscale;
   GtkWidget * offset_beg_xy;
   GtkWidget * offset_end_xy;
   GtkWidget * offset_radec; 

   // Command Console
   struct cmd_t              // command widget 
   {
      cc_console * main_console;   // command console
   } cmd;

   // Floating Display Dialog Windows.
	GtkWidget * dpy_dialog_window;         /* dialog window for Dpy[]  */

	GtkWidget * configure_dialog_window;   /* configuration dialog window */

	GtkWidget * file_open_dialog;             // file open dialog window 
	GtkWidget * file_open_xtension;           // spin button for xtension value.
	GtkWidget * file_open_dialog_buffer;      // cbox provide destination buffer for data.
	GtkWidget * file_open_movie_show_delay;   // entry for movie show delay 

	GtkWidget * file_save_dialog;          // file save dialog window 
	GtkWidget * file_save_dialog_buffer;   // cbox provide destination buffer for data.
	int         file_save_dialog_buffer_hid;  // HID for file_open_dialog_buffer

	struct browseForPath_t bfp_macro;      // macro's browser for path 
	struct browseForPath_t bfp_data_path;  // browser for path for data path (in setup) 

	struct moris_guide_adj_t mga;          // Moris Guider Adjustment Dialog widgets/data
};


/*************************************************************************/
/*                      4. Application Globals                           */
/*************************************************************************/

EXTERN struct cm_t CM;                     /* keep track of color related info */
EXTERN struct w_t W;                       /* application GTK widgets */
EXTERN struct macro_t Md;                  /* macro widgets & data */
EXTERN struct lc_t Lc;                     /* application's local variables */
EXTERN struct dpy_t Dpy[MAX_DPY];          /* Display configuration */
EXTERN struct df_buf_t Buffer[NUM_BUFFER]; /* Data buffers */
EXTERN struct stats_t  Stats[NUM_BUFFER];  /* stats for each Buffer[] */


/* define the static colors used the this application */
EXTERN struct cm_colordef_t CM_static_colors_def[CM_NUM_STATIC_COLORS]
#if MAIN
= {
/*     RED  GREEN    BLUE                */
	{     0,     0,      0},   /* Black   */
	{  1.00,     0,      0},   /* Red     */
	{     0,  1.00,      0},   /* Green   */
	{     0,     0,   1.00},   /* Blue    */
	{  0.63,  0.63,   0.63},   /* Gray    */
	{     0,  1.00,   1.00},   /* Cyan    */
	{  1.00,     0,   1.00},   /* Megenta */
	{  1.00,  1.00,      0},   /* Yellow  */
	{  1.00,  1.00,   1.00},   /* White   */
}
#endif
   ;

/*------------------------------------------------------------------
**  Tables and selection list
**------------------------------------------------------------------
*/

EXTERN char * offon_selection[3]
#ifdef MAIN
   = { "off", "on", NULL }
#endif
   ;

EXTERN char * allbox_selection[3]
#ifdef MAIN 
   = {  "All", "Box", NULL}
#endif
   ;

EXTERN char * noise_mode_selection[3]
#ifdef MAIN 
   = {  "mod", "ChSize", NULL}
#endif
   ;

EXTERN char * xy_selection[3]
#ifdef MAIN 
   = {  "X", "Y", NULL}
#endif
   ;

EXTERN char * aofig_format_selection[NUM_AOFIG_FORMAT+1]
#ifdef MAIN 
   = {  "Text", "DM", "Sensor", NULL}
#endif
   ;

EXTERN  char * buffer_selection[NUM_BUFFER+1]
#if MAIN
   = { "A", "B", "C", "D", "E", "F", "G", "H", NULL  }
#endif
   ;

EXTERN char * buffer_status_names[3]   
#ifdef MAIN
   = {  "EMPTY", "UNSAVED", "SAVED" }  
#endif
   ;

EXTERN  char * CM_color_names[CM_NUM_STATIC_COLORS]
#if MAIN
   = { "Black", "red", "green", "blue", "gray", "cyan", "magenta", "yellow", "white" }
#endif
   ;

EXTERN  char * colormap_selection[9]
#if MAIN
   = { "a.cm", "b.cm", "bb.cm", "c.cm", "gray.cm", "grayRed.cm", "i8.cm", "ao.cm", NULL  }
#endif 
   ;

EXTERN  char * dpytype_selection[NUM_DPYTYPE+1]
#if MAIN
   = { "Image", "Header", "Histogram", "LineCut",  "XLineCut", "Noise", "Pointer",
       "Stats", "AOFig",  "SpectraA",  "SpectraB", NULL  }
#endif
   ;

EXTERN  char * function_button_selection[NUM_FUN_BUT+1]
#if MAIN
   = { "0", "1", "2", "3", "4", "5", NULL  }
#endif
   ;

EXTERN char * imageautoscale_selection[3]
#ifdef MAIN
   = {  "Fixed", "Auto", NULL}
#endif
   ;

EXTERN char * sa_yautoscale_selection[4]
#ifdef MAIN
   = { "Fixed", "Local", "Global" , NULL}
#endif
   ;

EXTERN char * Month_name[12]
#ifdef MAIN
   = { "jan", "feb", "mar", "apr", "may", "jun", 
       "jul", "aug", "sep", "oct", "nov", "dec"  }
#endif
   ;

EXTERN char * noisegraphtype_selection[3]
#ifdef MAIN
   = {  "Text", "Graph", NULL}
#endif
   ;

EXTERN char * printer_selection[3]
#ifdef MAIN
   = {  "BW_Postscript", "Color_Postscript", NULL }
#endif
   ;

EXTERN  char * rotate_selection[4]
#if MAIN
   = { "-90", "+90", "180", NULL  }
#endif
   ;

EXTERN  char * to_selection[2]  /* a convience list for parsing commands */
#if MAIN
   = { "to", NULL  }
#endif
   ;

EXTERN  char * mathops_selection[6]  /* a convience list for parsing math commands */
#if MAIN
   = { "+", "-", "*", "/", "=", NULL  }
#endif
   ;

EXTERN char * inst_flavor_selection[NUM_INST_FLAVOR+1]
#ifdef MAIN
   = {  "bigdog", "guidedog", "smokey", "moris", NULL}
#endif
   ;

EXTERN char * Button_names[6]       // for ID's subarray, GuideA & GuideB       
#ifdef MAIN
   = {  "0", "1", "2", "Ga", "Gb", NULL}
#endif
   ;


/*-------------------
** AO polygons
**-------------------
*/

EXTERN polygon_t AO_Sensor_Table[AOFIG_NUM_ELEMENTS]
#if MAIN
=  {
      { 5, { {500,460}, {500,350}, {575,370}, {630,425}, {535,480} } }, // 0 - Inner Ring
      { 5, { {535,480}, {630,425}, {650,500}, {630,575}, {535,520} } }, // 1
      { 5, { {535,520}, {630,575}, {575,629}, {500,650}, {500,540} } }, // 2
      { 5, { {500,540}, {500,650}, {425,629}, {370,574}, {465,520} } }, // 3
      { 5, { {465,520}, {370,574}, {349,500}, {370,425}, {465,480} } }, // 4
      { 5, { {465,480}, {370,425}, {426,369}, {500,350}, {500,460} } }, // 5
      { 4, { {500,350}, {500,250}, {624,287}, {575,370}, {  0,  0} } }, // 6 - Middle Ring
      { 4, { {575,370}, {624,287}, {717,377}, {630,425}, {  0,  0} } }, // 7
      { 4, { {630,425}, {717,377}, {746,500}, {650,500}, {  0,  0} } }, // 8
      { 4, { {650,500}, {746,500}, {715,625}, {630,575}, {  0,  0} } }, // 9
      { 4, { {630,575}, {715,625}, {622,713}, {575,629}, {  0,  0} } }, // 10
      { 4, { {575,629}, {622,713}, {500,750}, {500,650}, {  0,  0} } }, // 11
      { 4, { {500,650}, {500,750}, {376,713}, {425,629}, {  0,  0} } }, // 12
      { 4, { {425,629}, {376,713}, {283,624}, {370,574}, {  0,  0} } }, // 13
      { 4, { {370,574}, {283,624}, {254,500}, {349,500}, {  0,  0} } }, // 14
      { 4, { {349,500}, {254,500}, {284,374}, {370,425}, {  0,  0} } }, // 15
      { 4, { {370,425}, {284,374}, {378,287}, {426,369}, {  0,  0} } }, // 16
      { 4, { {426,369}, {378,287}, {500,250}, {500,350}, {  0,  0} } }, // 17
      { 4, { {500,250}, {500,000}, {674,031}, {587,265}, {  0,  0} } }, // 18 - Outter Ring
      { 4, { {587,265}, {674,031}, {823,119}, {662,310}, {  0,  0} } }, // 19
      { 4, { {662,310}, {823,119}, {934,252}, {717,377}, {  0,  0} } }, // 20
      { 4, { {717,377}, {934,252}, {992,416}, {746,458}, {  0,  0} } }, // 21
      { 4, { {746,458}, {992,416}, {992,590}, {746,544}, {  0,  0} } }, // 22
      { 4, { {746,544}, {992,590}, {931,752}, {715,625}, {  0,  0} } }, // 23
      { 4, { {715,625}, {931,752}, {819,885}, {660,692}, {  0,  0} } }, // 24
      { 4, { {660,692}, {819,885}, {669,970}, {584,735}, {  0,  0} } }, // 25
      { 4, { {584,735}, {669,970}, {500,999}, {500,750}, {  0,  0} } }, // 26
      { 4, { {500,750}, {500,999}, {326,967}, {413,735}, {  0,  0} } }, // 27
      { 4, { {413,735}, {326,967}, {177,880}, {339,690}, {  0,  0} } }, // 28
      { 4, { {339,690}, {177,880}, { 66,748}, {283,624}, {  0,  0} } }, // 29
      { 4, { {283,624}, { 66,748}, {  8,584}, {253,542}, {  0,  0} } }, // 30
      { 4, { {253,542}, {  8,584}, {  8,410}, {254,456}, {  0,  0} } }, // 31
      { 4, { {254,456}, {  8,410}, { 69,248}, {284,374}, {  0,  0} } }, // 32
      { 4, { {284,374}, { 69,248}, {181,116}, {340,301}, {  0,  0} } }, // 33
      { 4, { {340,301}, {181,116}, {332, 29}, {416,265}, {  0,  0} } }, // 34
      { 4, { {416,265}, {332, 29}, {500,000}, {500,250}, {  0,  0} } }, // 35
   }
#endif
   ;

EXTERN polygon_t AO_DM_Table[AOFIG_NUM_ELEMENTS]
#if MAIN
=  {
      { 5, { {506,480}, {506,400}, {550,414}, {584,446}, {515,487} } }, // 0 - Inner Ring
      { 5, { {519,496}, {589,455}, {600,500}, {590,544}, {519,505} } }, // 1
      { 5, { {515,513}, {584,554}, {549,587}, {505,600}, {504,520} } }, // 2
      { 5, { {495,519}, {496,600}, {450,586}, {416,555}, {486,514} } }, // 3
      { 5, { {481,507}, {410,544}, {400,500}, {411,454}, {481,494} } }, // 4
      { 5, { {486,485}, {417,444}, {450,413}, {496,400}, {496,480} } }, // 5
      { 4, { {506,305}, {594,329}, {551,403}, {506,390}, {  0,  0} } }, // 6 - Middle Ring
      { 4, { {603,334}, {666,399}, {593,441}, {560,408}, {  0,  0} } }, // 7
      { 4, { {598,451}, {672,408}, {695,495}, {610,495}, {  0,  0} } }, // 8
      { 4, { {610,505}, {695,505}, {672,592}, {598,550}, {  0,  0} } }, // 9
      { 4, { {593,559}, {664,602}, {602,666}, {559,593}, {  0,  0} } }, // 10
      { 4, { {550,598}, {593,671}, {505,695}, {504,610}, {  0,  0} } }, // 11
      { 4, { {495,610}, {495,695}, {406,671}, {450,598}, {  0,  0} } }, // 12
      { 4, { {440,593}, {398,666}, {338,603}, {408,559}, {  0,  0} } }, // 13
      { 4, { {402,550}, {329,593}, {305,505}, {390,506}, {  0,  0} } }, // 14
      { 4, { {390,495}, {305,495}, {329,406}, {402,450}, {  0,  0} } }, // 15
      { 4, { {408,440}, {335,398}, {399,333}, {442,407}, {  0,  0} } }, // 16
      { 4, { {450,402}, {409,328}, {496,305}, {496,390}, {  0,  0} } }, // 17
      { 4, { {507,195}, {507, 30}, {659, 58}, {600,213}, {  0,  0} } }, // 18 - Outter Ring
      { 4, { {611,216}, {667, 61}, {800,139}, {692,264}, {  0,  0} } }, // 19
      { 4, { {702,274}, {808,145}, {906,264}, {762,344}, {  0,  0} } }, // 20
      { 4, { {767,353}, {910,271}, {962,416}, {800,442}, {  0,  0} } }, // 21
      { 4, { {802,453}, {964,425}, {963,579}, {802,548}, {  0,  0} } }, // 22
      { 4, { {799,559}, {962,588}, {909,731}, {767,648}, {  0,  0} } }, // 23
      { 4, { {761,658}, {904,740}, {805,857}, {700,730}, {  0,  0} } }, // 24
      { 4, { {692,737}, {798,863}, {664,941}, {610,784}, {  0,  0} } }, // 25
      { 4, { {599,789}, {655,944}, {503,970}, {503,805}, {  0,  0} } }, // 26
      { 4, { {496,805}, {496,970}, {343,943}, {400,789}, {  0,  0} } }, // 27
      { 4, { {390,785}, {333,939}, {200,862}, {308,737}, {  0,  0} } }, // 28
      { 4, { {300,730}, {194,856}, { 96,739}, {238,656}, {  0,  0} } }, // 29
      { 4, { {233,648}, { 91,731}, { 38,585}, {200,558}, {  0,  0} } }, // 30
      { 4, { {198,547}, { 36,576}, { 37,421}, {198,453}, {  0,  0} } }, // 31
      { 4, { {200,441}, { 39,412}, { 92,267}, {234,352}, {  0,  0} } }, // 32
      { 4, { {239,342}, { 97,259}, {195,143}, {310,268}, {  0,  0} } }, // 33
      { 4, { {309,262}, {204,136}, {338, 59}, {393,214}, {  0,  0} } }, // 34
      { 4, { {404,211}, {348, 55}, {497, 30}, {497,195}, {  0,  0} } }, // 35
   }
#endif
   ;

/*-------------------
** Spex Info  
**-------------------
*/
#define GD_NUM_SLITS 12

EXTERN char * Gd_slit_selection[GD_NUM_SLITS+1]
#if MAIN
= {"Open",   "Mirror", "0.3x15", "0.5x15", "0.8x15",
	"1.6x15", "3.0x15", "0.3x60", "0.5x60", "0.8x60",
	"1.6x60", "3.0x60",  NULL };
#endif
   ;

EXTERN float Gd_slit_wid_selection[GD_NUM_SLITS]
#if MAIN
= { 0.3,  0.3, 0.3, 0.5, 0.8, 1.6, 3.0, 0.3, 0.5, 0.8, 1.6, 3.0, }
#endif
   ;

// x, y, wid, hgt of the guider guideboxes
EXTERN struct farray_t Gd_slit_auto_guidebox_a[GD_NUM_SLITS]
#if MAIN
={
   { 249.0, 209.0, 30, 30  }, //  Open 
   { 249.0, 209.0, 30, 30  }, //  Mirror 
   { 249.3, 209.0, 30, 30  }, //  0.3x15 
   { 245.3, 210.0, 30, 30  }, //  0.5x15 
   { 246.3, 214.0, 30, 30  }, //  0.8x15 
   { 246.8, 210.0, 32, 32  }, //  1.6x15 
   { 245.9, 212.0, 45, 45  }, //  3.0x15 
   { 247.0, 116.0, 30, 30  }, //  0.3x60 
   { 244.3, 116.0, 30, 30  }, //  0.5x60 
   { 242.6, 116.0, 30, 30  }, //  0.8x60 
   { 243.9, 116.0, 32, 32  }, //  1.6x60 
   { 244.7, 116.0, 45, 45  }  //  3.0x60 
}
#endif
   ;

// x, y, wid, hgt of the guider guideboxes
EXTERN struct farray_t Gd_slit_auto_guidebox_b[GD_NUM_SLITS]
#if MAIN
= {
   { 249.0, 272.0, 30, 30  }, //  Open 
   { 249.0, 272.0, 30, 30  }, //  Mirror 
   { 250.0, 272.0, 30, 30  }, //  0.3x15 
   { 246.3, 273.0, 30, 30  }, //  0.5x15 
   { 246.9, 276.0, 30, 30  }, //  0.8x15 
   { 247.6, 273.0, 32, 32  }, //  1.6x15 
   { 246.7, 275.0, 45, 45  }, //  3.0x15 
   { 248.4, 372.0, 30, 30  }, //  0.3x60 
   { 247.9, 372.0, 30, 30  }, //  0.5x60 
   { 245.1, 372.0, 30, 30  }, //  0.8x60 
   { 246.5, 372.0, 32, 32  }, //  1.6x60 
   { 245.5, 372.0, 45, 45  }, //  3.0x60 
}
#endif
   ;



/*************************************************************************/
/*                      5. Function Prototypes                           */
/*************************************************************************/

/* xui.c */
int main( int argc, char *argv[] );
void initialize_globals( char * program_name );
void usage( void );
void cal_box_stats_subf( struct df_buf_t *bp, int type, int objx, int objy,
   int objwid, int objhgt, int skyx, int skyy, double *rN,
   double *rmin, double *rmax, double *rsum, double *rmean, double *rstd);
void cal_box_stats( int bufinx );
void rotate_pt( double *Rx, double *Ry, double x, double y, double angle );
double normalize_0_360( double angle );
int cpu_msb( void );
int tcs3_com( char *command, char *reply, int reply_size, char *tcshostname );
int bigdog_com( char *command, char *reply, int reply_size, char *tcshostname );
int guidedog_com( char *command, char *reply, int reply_size, char *tcshostname );
int moris_com( char *command, char *reply, int reply_size, char *tcshostname );
int smokey_com( char *command, char *reply, int reply_size, char *tcshostname );
int spex_fix_subarray_dim( int *ax, int *ay, int *awid, int *ahgt);

int PointInPolygon( double pgon[][2], int numverts, double point[2]);

#if (GTK_MAJOR_VERSION == 2) && (GTK_MINOR_VERSION < 12)
void gtk_widget_set_tooltip_text( GtkWidget *widget, const gchar * text );
#endif

/*----------------------------------------------------------------------------
** init.c
*/
int create_base( char * font_name );

void create_mainbar_widgets( GtkWidget ** c );
void create_mainbar_button_menu( GtkWidget ** c );

void create_main_dpydata( GtkWidget ** c );
void create_dialog_dpydata ( GtkWidget ** c );
void create_one_dpydata( GtkWidget ** c, int wid, int hgt, int dpinx, int resize_it );

void create_configure_widgets( GtkWidget ** c, GtkWidget *base_window );
void create_cmdio_widgets(GtkWidget ** c, GtkWidget *base_window );
void create_display_widgets( GtkWidget ** c );
void create_math_widgets( GtkWidget ** c );
void create_setup_widgets( GtkWidget ** c );
void create_offset_widgets( GtkWidget ** c );

void create_display_parameters_notebook( GtkWidget ** c ) ;
void create_display_image_widgets( GtkWidget ** c );
void create_display_header_widgets( GtkWidget ** c );
void create_display_histogram_widgets( GtkWidget ** c );
void create_display_linecut_widgets( GtkWidget ** c );
void create_display_xlinecut_widgets( GtkWidget ** c );
void create_display_noise_widgets( GtkWidget ** c );
void create_display_pointer_widgets( GtkWidget ** c );
void create_display_stats_widgets( GtkWidget ** c );
void create_display_aofig_widgets( GtkWidget ** c );
void create_display_spectra_a_widgets( GtkWidget ** c );
void create_display_spectra_b_widgets( GtkWidget ** c );

void create_macro_widgets( GtkWidget ** c ) ;
void create_about_widgets ( GtkWidget ** c );

GtkWidget * MyCreateCheckButtons( char * selection[], GtkWidget *check[], 
	int is_hbox, GCallback checkbutton_cb );
GtkWidget *  MyCreateRadioButtons( char * selection[], GtkWidget *radio[],
	int is_hbox, int draw_indicator, GCallback radiobutton_cb);
GtkWidget* MyComboBoxCreate( char * selection_list[]);
GtkWidget *  MyCreateMenuFromSelection( char * selection[], int tearoff,               
	GCallback menuitem_cb, gpointer user_data );
void MyStyleSetItemColor( GdkColor color, char item, GtkStyle * style);
#if USE_PANGO
int get_font( struct font_info_t * myfont, GtkWidget * gtk_widget, char * font_name);
#endif

/*----------------------------------------------------------------------------
** cb.c
*/
int base_delete_event( GtkWidget *widget, GdkEvent *event, gpointer data );
void base_destroy_cb( GtkWidget *widget, gpointer data );
void widget_hide_cb( GtkWidget *widget, gpointer data );
void button_cb( GtkWidget *widget, gpointer data );
void button2_cb( GtkWidget *widget, gpointer data );
void adj_cb( GtkAdjustment *adj, gpointer data );
void entry_cb( GtkEntry *w, gpointer data );
gint entry_fo_cb( GtkEntry *widget, GdkEventFocus * event, gpointer data );
void toggle_button_cb( GtkToggleButton *widget, gpointer data );
void checkbutton_offon_cb( GtkToggleButton *widget, gpointer data );
int dialog_delete_event( GtkWidget *widget, GdkEvent  *event, gpointer data );

/*--------- Main Menu bar Callbacks ------------*/
void menubar_open_cb( GtkWidget *widget, gpointer data );
void menubar_save_cb( GtkWidget *widget, gpointer data );
void menubar_quit_cb( GtkWidget *widget, gpointer data );
void menubar_configure_cb( GtkWidget *widget, gpointer data );
void colormap_cbox_cb( GtkWidget *widget, gpointer data );

/*--------- Display Options Callbacks ---------*/
void dpyactive_cb( GtkToggleButton *widget, gpointer data );
void dpytype_cb( GtkWidget * widget, gpointer data );
void buffer_cb( GtkToggleButton *widget, gpointer data );

void image_autoscale_cb( GtkWidget *widget, gpointer data );
void image_zoom_cb( GtkAdjustment *adj, gpointer data );
void image_range_cb( GtkEntry *w, gpointer data );
gint image_range_fo_cb( GtkEntry *widget, GdkEventFocus * event, gpointer data );
void inst_subarray_cb( GtkWidget *widget, gpointer data );

void hist_area_cb( GtkToggleButton *widget, gpointer data );
void hist_range_cb( GtkEntry *w, gpointer data );
gint hist_range_fo_cb( GtkEntry *widget, GdkEventFocus * event, gpointer data );

void lcut_area_cb( GtkToggleButton *widget, gpointer data );
void lcut_autoscale_cb( GtkToggleButton *widget, gpointer data );
void lcut_range_cb( GtkEntry *w, gpointer data );
gint lcut_range_fo_cb( GtkEntry *widget, GdkEventFocus * event, gpointer data );
void lcut_xy_cb( GtkEntry *w, gpointer data );
gint lcut_xy_fo_cb( GtkEntry *widget, GdkEventFocus * event, gpointer data );
void lcut_xarrow( GtkWidget *w, gpointer data );
void lcut_yarrow( GtkWidget *w, gpointer data );

void noise_g1range_cb( GtkEntry *w, gpointer data );
gint noise_g1range_fo_cb( GtkEntry *widget, GdkEventFocus * event, gpointer data );
void noise_g2range_cb( GtkEntry *w, gpointer data );
void noise_autoscale_cb( GtkToggleButton *widget, gpointer data );
void noise_graphtype_cb( GtkToggleButton *widget, gpointer data );
void noise_area_cb( GtkToggleButton *widget, gpointer data );
void noise_mode_cb( GtkToggleButton *widget, gpointer data );

void xcut_autoscale_cb( GtkToggleButton *widget, gpointer data );
void xcut_range_cb( GtkEntry *w, gpointer data );
gint xcut_range_fo_cb( GtkEntry *widget, GdkEventFocus * event, gpointer data );
void xcut_set_cb( GtkEntry *w, gpointer data );
gint xcut_set_fo_cb( GtkEntry *widget, GdkEventFocus * event, gpointer data );
void xcut_setline_cb( GtkWidget *widget, gpointer data );
void xcut_output_data_cb( GtkWidget *widget, gpointer data );
void xcut_fit_data_cb( GtkWidget *widget, gpointer data );

void stats_setsky_cb( GtkWidget *widget, gpointer data );
void stats_fixedwh_cb( GtkWidget *widget, gpointer data );
void stats_wh_entry_cb( GtkWidget *widget, gpointer data );
gint stats_wh_entry_fo_cb( GtkWidget *widget, GdkEventFocus * event, gpointer data );

void aofig_format_cb ( GtkWidget *widget, gpointer data );
void aofig_data_cb ( GtkWidget *widget, gpointer data );
void aofig_XY_cb (GtkAdjustment *adj, gpointer data );

void sa_setobjbin_cb( GtkWidget *widget, gpointer data );
void sa_setskybin_cb( GtkWidget *widget, gpointer data );
void sa_yautoscale_cb ( GtkMenuItem * menuitem, gpointer data );
void sa_setxscale_cb( GtkWidget *widget, gpointer data );
void sb_setobjbin_cb( GtkWidget *widget, gpointer data );
void sb_setskybin_cb( GtkWidget *widget, gpointer data );
void sb_show_cb( GtkToggleButton *widget, gpointer data );
void sb_setxscale_cb( GtkWidget *widget, gpointer data );

/*--------- Macro Dialog Callbacks ---------*/
void m_execute_cb( GtkWidget *widget, gpointer data );
void m_filelist_cb( GtkWidget *widget, gint row, gint col, gpointer data );
void m_filemask_cb( GtkWidget *widget, gpointer data );
gint m_filemask_fo_cb( GtkWidget *widget, GdkEventFocus * event, gpointer data );
void m_set_path_cb( GtkWidget *widget, gpointer data );
void m_path_cb( GtkWidget *widget, gpointer data );
gint m_path_fo_cb( GtkWidget *widget, GdkEventFocus * event, gpointer data );
void m_refresh_cb( GtkWidget *widget, gpointer data );
void m_stop_cb( GtkWidget *widget, gpointer data );
void m_clear_file( void );
gboolean m_exe_timer_func( gpointer data );

/*--------- Math Widgets Callbacks ---------*/
void math_equ_cb( GtkWidget *widget, gpointer data );
void math_copy_button_cb( GtkWidget *widget, gpointer data );
void math_clear_button_cb( GtkWidget *widget, gpointer data );
void math_rotate_button_cb( GtkWidget *widget, gpointer data );

/*--------- Offset Widgets Callbacks ---------*/
void mga_show_button_cb( GtkWidget *widget, gpointer data );

/*--------- Setup Widgets Callbacks ---------*/
void printer_type_cb ( GtkWidget *widget, gpointer data );
void inst_flavor_cb ( GtkWidget *widget, gpointer data ); 
void browse_data_path_cb( GtkWidget *widget, gpointer data );
void offset_defaults_cb( GtkWidget *widget, gpointer data );

/*--------- Fresh Widgets ---------*/

void update_display_widgets( void );
void update_displayOptions_widgets( void );
void update_dpytype_image_widgets( struct dpy_t *dp );
void update_dpytype_histogram_widgets( struct dpy_t *dp );
void update_dpytype_linecut_widgets( struct dpy_t *dp );
void update_dpytype_noise_widgets( struct dpy_t *dp );
void update_dpytype_pointer_widgets( struct dpy_t *dp );
void update_dpytype_xlinecut_widgets( struct dpy_t *dp );
void update_dpytype_stats_widgets( struct dpy_t *dp );
void update_dpytype_aofig_widgets( struct dpy_t *dp );
void update_dpytype_spectra_a_widgets( struct dpy_t *dp );
void update_dpytype_spectra_b_widgets( struct dpy_t *dp );
void update_setup_widgets ( void );
void update_offset_widgets( void );

/*--------- Dialog Callbacks ---------*/

void file_open_activate_cb( GtkWidget *widget, gpointer data );
void file_open_response_cb( GtkWidget *widget, gint response_id, gpointer data );
void file_open_read_xtension( GtkWidget *widget, gpointer data );
void file_open_movieshow( GtkWidget *widget, gpointer data );

void file_save_response_cb( GtkWidget *widget, gint response_id, gpointer data );
void file_save_bufinx_cb( GtkWidget * widget, gpointer data );
void update_file_save_dialog( int bufinx );

void bfp_macro_ok_cb( GtkWidget *widget, gpointer data );
void bfp_data_path_ok_cb( GtkWidget *widget, gpointer data );

/*--------- Timer/IO ---------*/

gboolean timer_function( gpointer data );
#if USE_GIO
gboolean socket_accept_connection( GIOChannel * source, GIOCondition condition, gpointer data);
#else
void socket_accept_connection( gpointer data, int fd, GdkInputCondition ic );
#endif

/*--------- Helper Functions ---------*/
void updateNotebookWidget( GtkWidget * widget, int widget_hid, int index );
void updateComboboxWidget( GtkWidget * widget, int widget_hid, int index );
void updateAdjustmentWidget( GtkObject * widget, int widget_hid, gdouble value );
int update_file_list( int isDir, char *cdir, char *fmask, GtkListStore *list_store, 
  GtkTreeView *list_view);

/*------------------------------------------------------
** command.c 
*/
int cmd_execute ( cc_console *cc, char * cmd_buf, int feedback );

int do_active( int index, char * par );
int do_aofig_data( int index, char * par );
int do_aofig_format( int index, char * par );
int do_aofig_XY( int index, char * par );
int do_boxcentroid( int index, char * par );
int do_boxcopy( int index, char * par );
int do_boxpeak( int index, char * par );
int do_boxscale( int index, char * par );
int do_boxzoom( int index, char * par );
int do_buffer( int index, char * par );
int do_bufinfo( int index, char * par );
int do_clear( int index, char * par );
int do_cmCenter( int index, char * par );
int do_cmWidth( int index, char * par );
int do_cm_swap_rgb( int index, char * par );
int do_colormap( int index, char * par );
int do_colormap_inverse( int index, char * par );
int do_copy( int index, char * par );
int do_displaytype( int index, char * par );
int do_divbydivisor( int index, char * par );
int do_drawbox( int index, char * par );
int do_drawline( int index, char * par );
int do_echo( int index, char * par );
int do_filter1( int index, char * par );
int my_filter1_fun(  struct df_buf_t *dest, struct df_buf_t *op1 );
int do_filter2( int index, char * par );
int my_filter2_fun(  struct df_buf_t *dest, struct df_buf_t *op1 );
int do_fullimage( int index, char * par );
int do_histarea( int index, char * par );
int do_hist_auto_range( int index, char * par );
void auto_update_hist_ref_dpinx( int dpinx  );
void auto_update_hist_ref_bufinx( int bufinx );
int do_histbin( int index, char * par );
int do_imageautoscale( int index, char * par );
int do_imagecompass_show( int index, char * par );
int do_imagecompass_flipNS( int index, char * par );
int do_imagerange( int index, char * par );
int do_imagescale1p( int index, char * par );
int cal_1per( struct df_buf_t *bp, int32_t * Rmin, int32_t * Rmax );
int do_imageshowgbox( int index, char * par );
int do_imagezoom( int index, char * par );
int do_inst_com( int index, char * par );
int do_inst_flavor( int index, char * par );
int do_gbox_cmd_right_click( int index, char * par );
int do_lcutarea( int index, char * par );
int do_lcutautoscale( int index, char * par );
int do_lcutrange( int index, char * par );
int do_lcutxy( int index, char * par );
int do_m_execute( int index, char * par );
int do_m_filemask( int index, char * par );
int do_m_load( int index, char * par ); 
int do_m_path( int index, char * par );
int do_m_refresh( int index, char * par );
int do_m_stop( int index, char * par );
int do_math( int index, char * par );
int do_move( int index, char * par );
int do_movieshow( int index, char * par );
int movieshow_readimage( int fd, struct df_buf_t * bp, int whichframe);
int do_movieshowdelay( int index, char * par );
int do_noisearea( int index, char * par );
int do_noisemod( int index, char * par );
int do_noisemode( int index, char * par );
int do_noise_size( int index, char * par );
int do_noisegraphtype( int index, char * par );
int do_noiseautoscale( int index, char * par );
int do_noiseg1range( int index, char * par );
int do_noiseg2range( int index, char * par );
int do_offset_angle( int index, char * par );
int do_offset_begxy( int index, char * par );
int do_offset_endxy( int index, char * par );
int do_offset_platescale( int index, char * par );
int do_offset_tcs( int index, char * par );
int do_offset_tcsab( int index, char * par );
void cal_offset_radec( void );
int do_path( int index, char * par ); 
int do_print( int index, char * par ); 
int do_printer( int index, char * par );
int do_printertype( int index, char * par );
int do_printtofile( int index, char * par );
int do_ptimagesize( int index, char * par );
int do_ptshowstats( int index, char * par );
int do_ptshowlinecut( int index, char * par );
int do_ptshowspex_sn( int index, char * par );
int do_push( int index, char * par );
int do_quit( int index, char * par ); 
int do_read( int index, char * par ); 
int do_read_xtension( int index, char * par ); 
int do_readfile( int index, char * par );
int do_readmovie( int index, char * par ); 
int readmovie( int index, char * par );
int do_readtextarray( int index, char * par );
int read_text_array_token( FILE *fp, char *buf, int max_len );
int read_text_array( FILE*fp, char *dir, char *filename, struct df_buf_t *bufp,
                     int naxis1, int naxis2);
int do_readsock( int index, char * par );
int do_rotate( int index, char * par );
int do_settimer( int index, char * par );
int do_sarowsperbin( int index, char *par );
int do_saobjbin( int index, char *par );
int do_saskybin( int index, char *par );
int do_sasubtractsky( int index, char *par );
int do_saxscale( int index, char *par );
int do_sayautoscale( int index, char *par );
int do_sayscale( int index, char *par ); 
int do_sashift( int index, char *par );
int do_sastats( int index, char *par );
int do_sbdatayrange( int index, char *par );
int do_sbdiffyrange( int index, char *par );
int do_sbobjbin( int index, char *par );
int do_sbshow( int index, char *par ); 
int do_sbyautoscale( int index, char *par );
int do_sbskybin( int index, char *par );
int do_sbxscale( int index, char *par );
int do_save( int index, char * par );
int do_savefile( int index, char * par );
int do_smooth( int index, char * par );
int my_smooth_fun(  struct df_buf_t *dest, struct df_buf_t *op1 );
int do_sqrt( int index, char * par );
int do_statsobjbox( int index, char * par );
int do_statssetsky( int index, char * par ); 
int do_statsxorline( int index, char * par );
int do_statsfixedwh( int index, char * par );
int do_test( int index, char * par );
int do_tcshostname( int index, char *par);
int do_tcssystem( int index, char *par);
int do_usehex( int index, char * par );
int do_usefitsanglescale( int index, char * par );
int do_xcutautoscale( int index, char * par );
int do_xcutset( int index, char * par );
int do_xcutrange( int index, char * par );

// test commands
int do_tcs3cmd( int index, char * par );
int do_x( int index, char * par );
int do_t_gd_info( int index, char * par );


/*--------- Helper Functions -------------------*/
int qsort_comp_string( char **i, char **j );
int math_fixstring( char *scr, int scr_size );


/*----------------------------------------------------------------------------
** DRAW.C
*/
/*--------- Colormap Callbacks -----------------*/
void call_colormap_redraw( void ); 
int  colormap_expose_event( GtkWidget *widget, GdkEventExpose *event );
int  colormap_configure_event( GtkWidget *widget, GdkEventConfigure *event );
void colormap_redraw( GtkWidget *da );

   /*--------- Dpytitle DA Callbacks ----------*/
int dpytitle_expose_event( GtkWidget *widget, GdkEventExpose *event );
int dpytitle_configure_event( GtkWidget *widget, GdkEventConfigure *event );
void call_dpytitle_redraw( int dpinx );
void redraw_dpytitle_for_bufinx( int bufinx );
void dpytitle_redraw( GtkWidget *da );

int dpytitle_button_press_event (GtkWidget *da, GdkEventButton *event, gpointer data );

   /*--------- Dpydata DA Callbacks -----------*/
int dpydata_expose_event( GtkWidget *widget, GdkEventExpose *event );
int dpydata_configure_event( GtkWidget *widget, GdkEventConfigure *event );
void call_dpydata_redraw( int dpinx );
void redraw_dpydata_for_bufinx( int bufinx );
void redraw_dpytype_stats_for_bufinx( int bufinx );
void dpydata_redraw( GtkWidget *da );

int dpydata_button_press_event( GtkWidget *widget, GdkEventButton *event, gpointer data );
int dpydata_motion_notify_event( GtkWidget *widget, GdkEventMotion *event, gpointer data );

int dpydata_motion_notify_event_for_ImageDpyType ( GtkWidget *da, GdkEventMotion *event,
   int dpinx, struct dpy_t *dp, struct df_buf_t *bp );
int dpydata_motion_notify_event_for_AOFigDpyType ( GtkWidget *da, GdkEventMotion *event,
   int dpinx, struct dpy_t *dp, struct df_buf_t *bp );

int dpydata_key_press_event( GtkWidget *da, GdkEventKey *event, gpointer data );
int dpydata_focus_in_event( GtkWidget *da, GdkEventFocus *event, gpointer data );
int dpydata_focus_out_event( GtkWidget *da, GdkEventFocus *event, gpointer data );

void draw_X( GtkWidget *da, struct dpy_t *dp, struct df_buf_t *bp );
void draw_no_data( GtkWidget *da, struct dpy_t *dp, struct df_buf_t *bp );
void draw_image( GtkWidget *da, struct dpy_t *dp, struct df_buf_t *bp );
void autoscale_image_range( struct dpy_t *dp, struct df_buf_t *bp );
void draw_image_plus_zoom_pseudocolor( struct dpy_t *dp, struct df_buf_t *bp,
     int da_wid, int da_hgt, unsigned char * image_buf);
void draw_image_plus_zoom_truecolor( struct dpy_t *dp, struct df_buf_t *bp,
     int da_wid, int da_hgt, int32_t * image_buf);
void draw_image_minus_zoom( struct dpy_t *dp, struct df_buf_t *bp,
     int da_wid, int da_hgt, unsigned char * image_buf);
void i32_set( uint32_t * buf, uint32_t value, int n);
void draw_header( GtkWidget *da, struct dpy_t *dp, struct df_buf_t *bp );
int  get_histogram_info( struct dpy_t *dp, struct hist_info *hist );
void draw_histogram( GtkWidget *da, struct dpy_t *dp, struct df_buf_t *bp );
int  get_linecut_info( struct dpy_t *dp, struct lcut_info *lcut );
void draw_linecut( GtkWidget *da, struct dpy_t *dp, struct df_buf_t *bp );
void draw_noise( GtkWidget *da, struct dpy_t *dp, struct df_buf_t *bp );
void calc_noise( struct noise_t *noisep, struct noise_t *fnoise,
     struct dpy_t *dp, struct df_buf_t *bp );
void draw_xcut( GtkWidget *da, struct dpy_t *dp, struct df_buf_t *bp );
int  xcut_line( int x0, int y0, int x1, int y1, struct df_buf_t *bp,
     struct xcut_buf_t *destbuf, int num_destbuf ); 
void draw_pointer( GtkWidget *da, struct dpy_t *dp, struct df_buf_t *bp );
void draw_pointer_update( GtkWidget *da, int dpinx, int ax, int ay, int image_dpinx );

int cal_spex_sn( char * reply, double *sn_ratio, int ax,  int ay, 
   int *r_cx, int *r_cy, int *r_wid, int is_bufC, 
   struct df_buf_t *bufc,  struct df_buf_t *bufb );  
   
void draw_stats( GtkWidget *da, struct dpy_t *dp, struct df_buf_t *bp );
void draw_aofig( GtkWidget *da, struct dpy_t *dp, struct df_buf_t *bp );
void draw_aofig_text_format( GtkWidget *da, struct dpy_t *dp, struct df_buf_t *bp);
void draw_aofig_DM_Sensor_format( GtkWidget *da, struct dpy_t *dp, struct df_buf_t *bp );
void draw_sa( GtkWidget *da, struct dpy_t *dp, struct df_buf_t *bp );
void draw_sb( GtkWidget *da, struct dpy_t *dp, struct df_buf_t *bp );
void draw_NA( GtkWidget *da, struct dpy_t *dp, struct df_buf_t *bp );

void dpy_hadj_cb( GtkAdjustment *adj, gpointer data );
void dpy_vadj_cb( GtkAdjustment *adj, gpointer data );
void update_scrollbars( int dpinx );

void draw_xor_box_on_dpy( int dpinx, int x, int y, int wid, int hgt );
void draw_xor_box_on_buffer( int bufinx, int x, int y, int wid, int hgt );
void draw_xor_line_on_dpy( int dpinx, int xbeg, int ybeg, int xend, int yend );
void draw_xor_line_on_buffer( int bufinx, int xbeg, int ybeg, int xend, int yend );

void draw_box_on_dpy( int dpinx, float x, float y, float wid, float hgt, int color);
void draw_line_on_dpy( int dpinx, float xbeg, float ybeg, float xend, float yend, int color );

#if USE_PANGO
struct ds_t * init_ds( struct ds_t *ds,  GtkWidget *da, 
	GdkGC * gc, struct font_info_t *font, int font_wid, int font_hgt );
#else
struct ds_t * init_ds( struct ds_t *ds,  GtkWidget *da, 
	GdkGC * gc, GdkFont *font, int font_wid, int font_hgt );
#endif 
void draw_text( struct ds_t *ds, int units, float x, float y, char * fmt, ...);
void draw_image_text( struct ds_t *ds, int units, float x, float y, char * fmt, ...);
void double2str_len( char *buf, double d, int len, int dec ) ;
void draw_line( struct ds_t *ds, int units, int x1, int y1, int x2, int y2 );
void draw_box( struct ds_t *ds, int units, int x1, int y1, int x2, int y2 );
void clear_box( struct ds_t *ds, int units, int x1, int y1, int x2, int y2 );
void da_set_foreground( struct ds_t *ds, int fg );
void da_set_background( struct ds_t *ds, int bg );

/*-------------------------------------------------------------
** PRINT.c
*/
int print_psheader( FILE *fp );
int print_fitsheader( FILE *fp, struct dpy_t *dp );
void fix_header_string( char *d, char *s );
int print_gdummy( FILE *fp, struct dpy_t *dp );
int print_gray_image( FILE *fp, struct dpy_t *dp );
int print_color_image( FILE *fp, struct dpy_t *dp);
uint32_t color_ps_rgb( int red, int green, int blue );
int print_histogram( FILE *fp, struct dpy_t *dp );
int print_linecut( FILE *fp, struct dpy_t *dp );
int print_noise( FILE *fp, struct dpy_t *dp );
int print_xcut( FILE *fp, struct dpy_t *dp );
int print_aofig_gray( FILE *fp, struct dpy_t *dp );
int print_sa( FILE *fp, struct dpy_t *dp );
int print_sb( FILE *fp, struct dpy_t *dp );

/*--------------------------
** bfp 
*/
int bfp_create_dialog ( struct browseForPath_t *bfp, char * title );
void bfp_hide_dialog ( struct browseForPath_t *bfp );
void bfp_show_dialog ( struct browseForPath_t *bfp );
void bfp_set_path( struct browseForPath_t *bfp, char * path ); 
void bfp_set_ok_cb( struct browseForPath_t *bfp, GCallback func );

/*--------------------------
** moris_guide_adj, or mga 
*/
int  mga_create_dialog ( struct moris_guide_adj_t *mga );
void mga_show_dialog ( struct moris_guide_adj_t *mga );
void mga_hide_dialog ( struct moris_guide_adj_t *mga );


/*************************************************************************/
/*                              The End                                  */
/*************************************************************************/
