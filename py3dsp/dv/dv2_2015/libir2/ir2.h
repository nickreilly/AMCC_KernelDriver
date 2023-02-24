/****************************************************************************
** IR Library, Version 2
**
** A general purpose library for some IRTF application.
** Version 2 an update to libir1.  
**   + Should be ok for 32/64 bit OS.
**   + The orginial ir1 was installed in /usr/local/include, lib/, ir2 is ment to be 
**      compile into the application.
** The source code home is ~s2/src/dv/libir2 (DV in spex's development account).
**
** Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php
** Copyright (C) 2012 <NASA IRTF. http://irtfweb.ifa.hawaii.edu>
**
** Note: these C files are not compiled in, but include in this source (to share)
**   keypress.c
**   sysV_ipc_help.c
*****************************************************************************
*/

#ifndef __ir2_h
#define __ir2_h

#ifdef   __cplusplus
extern "C" {
#endif

/*-------------------------------------
**  dislist structs & defines
*/
#define DIRLIST_MAX_LIST  50
#define DIRLIST_MAX_PATH  512

struct dirlist_t
{
	char path[DIRLIST_MAX_PATH];
	int  nele;
	char *list[DIRLIST_MAX_LIST];
};

/*-------------------------------------
**  strings...
*/
char * btos( char * s,  unsigned long l, int nbits );
int  matchname( char *pattern, char *name);
int my_atof( char *s, double *d_ptr );
int  my_atol( char *s, long *l);
int  str_replace_sub( char *src, char *dest, int dest_size, char *pattern, char *val);
char *str_rws( char *s, unsigned int maxlen);
int stricmp( char *s, char *t);
char *strlwr( char * string);
char *strupr( char * string);
char *strxcat( char *dest, char *src, int dest_size);
char *strxcpy( char *dest, const char *src, int maxlen);
char *unpad( char *s, int c);

int timeStr2sec( double *Rsec, char *str);
char * sec2timeStr ( char *outbuf, int  outbuf_size, double sec, int decimals, int show_plus );

/*-------------------------------------
**  files & directories ...
*/
char *cat_pathname( char *pathname, char *path, char *filename, int maxlen);
int create_path( char * p);
void dir_from_path( char *dir, char *pathname, int dir_size );
void dirlist_free( struct dirlist_t *dl );
void dirlist_init( struct dirlist_t *dl );
void dirlist_makelist( struct dirlist_t * dl );
void dirlist_newpath( struct dirlist_t *dl, char *path );
void dirlist_showlist( struct dirlist_t * dl );
int exist_path( char * pathname );
int expand_pathname(  char *pathout, int  pathout_size, char *pathin);
void filename_from_path( char *filename, char *pathname, int filename_size );
int get_full_pathname( char * pathout, int pathout_size, char *pathin);

/*-------------------------------------
**  socket...
*/
int sock_open( char * hostname, int port, int type, int protocol );
int sock_setup_server( int port_no );
int32_t sock_read_data( int fd, char *buf, int32_t bufsize, int blocking, int timeout_ms);
int32_t sock_write_data( int fd, char *buf, int32_t bufsize, int blocking, int timeout_ms);
int sock_write_msg( int fd, char *msg, int blocking, int socket_timeout_ms);
int sock_read_msg( int fd, char *msg, int blocking, int socket_timeout_ms);
int sock_udp_message( int fd, void * send_buf, int nbytes_send,
void * read_buf, int nbytes_read, int timeout_ms );
int sock_readline( int fd, char * rbuf, int sizeof_rbuf, int * bytes_read, int timeout_ms);
int sock_flush( int fd );

/*--------------------------------------
** string parsing & conversion
*/
int parseInt_r( int * ip, char *buf, const char *tok, char **st_ptr );
int parseIntR_r( int * ip, char *buf, const char *tok, char **st_ptr, int min, int max );
int parseDouble_r( double *dp, char *buf, const char *tok, char **st_ptr );
int parseDoubleR_r( double *dp, char *buf, const char *tok, char **st_ptr, double min, double max);
int parseFloat_r( float *fp, char *buf, const char *tok, char **st_ptr );
int parseFloatR_r( float *fp, char *buf, const char *tok, char **st_ptr, float min, float max );
int parseSelection_r( char * buf, char * tok, char **st_ptr, char ** list);
int parseString_r( char *outb, int outb_size, char *buf, const char *tok, char **st_ptr );

/*-----------------------------------------------------------------
** Types and prototype for 2D coordinate transformation functions
** See t2d.c for details.
*/
typedef double t2d_matrix[3][3];   /* matrix type */
typedef double t2d_vector[3];      /* vector type */

int t2d_tranlation( t2d_matrix m, double tx, double ty );
int t2d_rotation( t2d_matrix m, double deg );
int t2d_scaling( t2d_matrix m, double sx, double sy );

int t2d_forward_matrix( t2d_matrix m,  double Tx, double Ty, double Sx, double Sy, double R);
int t2d_reverse_matrix( t2d_matrix m, double Tx, double Ty, double Sx, double Sy, double R);

int  t2d_apply_to_point( double *dx, double *dy, t2d_matrix m, double sx, double sy);

void t2d_m_mul( t2d_matrix p, t2d_matrix a, t2d_matrix b);
void t2d_m_copy(t2d_matrix dst, t2d_matrix src);
void t2d_m_identity(t2d_matrix m);
void t2d_m_print(t2d_matrix m);

/*--------------------------------------
** others
*/
double d_clip_range( double d, double min, double max );
double d_round( double d, double resolution );
double drange( double v, double min, double max);
double dpoly( double x, double *coeff, int degree );
void get_vector( int * dir, double *mag, double start, double dest, double size );

double map( double point, double pref1, double pref2, double nref1, double nref2 );
int point_in_polygon( double   pgon[][2], int   numverts, double   point[2]);

void d_stats_p1( double *r_min, double *r_max, double *r_mean, double *data, const int n );
void d_stats_p2( double * r_std, double * data, const int n, const double mean );
void l_stats_p1( int32_t * Rmin, int32_t *Rmax, double *Rmean, int32_t * addr, int n);
void l_stats_p2( double *Rstd, int32_t * addr, int n, double mean);

int32_t elapse_msec( struct timeval *start, struct timeval *end);
double elapse_sec( struct timeval *start, struct timeval *end);
int32_t my_timegm( int year, int yday, int hr, int min, int sec);

/*--------------------------------------------------------------------------
** defines related to socket port number and IPC ID, etc.
**--------------------------------------------------------------------------
*/
#define IRTF_DV_PORT          30123    /* Port ID for IRTF Viewer program (dv)   */

#define SOCKET_MSG_LEN      160    /* Size of message string used in sockets */
#define SOCK_PACKET_SIZE  32768    /* Max num of bytes to socket write/read  */

/*--------------------------------------
**  Very common definitions.
**--------------------------------------
*/

#ifndef TRUE
#define TRUE    1
#define FALSE   0
#define ON      1
#define OFF     0
#endif
  
#ifndef INRANGE
#define INRANGE(a,x,b)   ((a) <= (x) && (x) <= (b))
#endif
	
#ifndef MAX
#define MAX( a, b )      ((a) < (b) ? (b) : (a))
#define MIN( a, b )      ((a) < (b) ? (a) : (b))
#endif
	 
#ifndef isodd
#define isodd(a)         ((a) % 2)
#define iseven(a)        (!((a) % 2))
#endif

/*------------------------------------------------------------------
**  Error codes 
**------------------------------------------------------------------
*/

/* this funtions return error string for any ERR_* code */
char * error_string( int rc );

#define ERR_NONE                    (0)    /* No error        */
#define ERR_INV_KW                 (-1)    /* Invalid keywork */
#define ERR_INV_RNG                (-2)    /* Invalid Range   */
#define ERR_INV_FORMAT             (-3)    /* Invalid Format or syntax error */
#define ERR_SOCKET_ERR            ( -4)    /* Socket communication error     */
#define ERR_SOCKET_TIMEOUT        ( -5)    /* Socket communication error     */
#define ERR_INV_PATH              ( -6)    /* This is not a valid path       */
#define ERR_MEM_ALLOC             ( -7)    /* Memory allocation error        */
#define ERR_FILE_CREATE           ( -8)    /* Error Creating File            */
#define ERR_FILE_WRITE            ( -9)    /* Error writing file             */
#define ERR_FILE_READ             (-10)    /* Error reading a file           */
#define ERR_FILE_FORMAT           (-11)    /* Invalid file format            */
#define ERR_INV_OPERATION         (-12)    // This is an invalid operation
#define ERR_RESTRICTED            (-13)    // This option is currently restricted
#define ERR_MSGQ                  (-14)    // System error on message queues
#define ERR_FILE_OPEN             (-15)    // Error Opening File
#define ERR_SEM_GET               (-16)    // Semaphore acquisition error
#define ERR_NOT_AVAILABLE         (-17)    // Reqested action or service is not available
#define ERR_NOT_READY             (-18)    // Unable to execute, object is busy
#define ERR_INV_DATA              (-19)    // Invalid data or input
#define ERR_IPC                   (-20)    // error or failure with IPCs
#define ERR_TASK_EXECL            (-21)    // error on starting forked task
#define ERR_STREAM                (-22)    // stream output command called with non-stream fd
#define ERR_BUFF_OVERRUN          (-23)    // command causes overflow on buffer limits
#define ERR_DEVICE_IO             (-24)    // communication error with hardware device
#define ERR_DEVICE_ERROR          (-25)    // Device error - did not responsd as needed
#define ERR_EXCEED_LIMIT          (-26)    // Error due to execeding a set limit
#define ERR_SAFETY_CONDITION      (-27)    // Unable to execute due to safety restriction
#define ERR_UNABLE_TO_DO          (-28)    // Unable to comply at this time
#define ERR_SUBARRAY_CNT          (-29)    // Invalid subarry count
#define ERR_SUBARRAY_FORMAT       (-30)    // Invalid sumarray dimension
#define ERR_NO_DATA               (-31)    // Requested data is not available.
#define ERR_STOP                  (-32)    // Operation was stopped
#define ERR_TIMEOUT               (-33)    // A operation did not complete in time.
#define ERR_FAILED                (-34)    // request or operation failed
#define ERR_DIFF_SIZE             (-35)    // error due to size
#define NUM_ERR_CODES               36     // number of defined error codes.

/****************************************************************************
** df_ functions are Double Fits Library.
*****************************************************************************
*/

/* FITS BITPIX values for data types */
#define DF_BITPIX_LONG         32
#define DF_BITPIX_SHORT        16
#define DF_BITPIX_FLOAT       -32
#define DF_BITPIX_DOUBLE       64
#define DF_BITPIX_CHAR          8

#define DF_FITS_RECORD_LEN   2880  /* Fits format has 2880 bytes per record  */
#define DF_FITS_LEN_FILENAME   40
#define DF_FITS_LEN_PATH       200
#define DF_RW_BUFFER_SIZE   (DF_FITS_RECORD_LEN*10)  /* Internal buffer for reading/writing data */
                                                     /* should be multiple of DF_FITS_RECORD_LEN */
 
#define DF_MAX_SIGNED_INT32    ((int32_t)0x7FFFFFFF)
#define DF_MIN_SIGNED_INT32    ((int32_t)0x80000001)
 
#define DF_EMPTY      0          /* File or Buffer status                  */
#define DF_UNSAVED    1
#define DF_SAVED      2

#define DF_MATH_ADD   0          /* Operation code for df_math() function  */
#define DF_MATH_SUB   1
#define DF_MATH_MUL   2
#define DF_MATH_DIV   3
#define DF_MATH_COPY  4
#define DF_MATH_SQRT  5

#define DF_ROT_M90    0
#define DF_ROT_P90    1
#define DF_ROT_180    2

#define DF_ORIGIN_TL  0 /* (0,0) is at the top left */
#define DF_ORIGIN_BL  1 /* (0,0) is at the bottom left */

/*--------------------------------------------------------
**  DF structures for holding data and sending fits data
*/
 
union u_df_lf               /* for long/float converstion with ntohl() */
{
   uint32_t l;
   float    f;
};

struct df_fheader_t
{
   struct df_fheader_t * next;
   char               buf[DF_FITS_RECORD_LEN];
};

 
struct df_internal_vars
{
   int divbycoadd;   /* should the divisor be applied to the data? */
   int origin;       /* where is the origin */
};


struct df_buf_t
{
   short              status;        /* Buffer status: empty, unsaved,...  */
   short              naxis1;        /* Number of points in NAXIS1         */
   short              naxis2;        /* Number of points in NASIS2         */
   short              size;          /* sizeof data value in bytes         */
   short              bitpix;        /* BITPIX code                        */
   int                N;             /* Number of data points              */
   float              max;           /* max data value                     */
   float              min;           /* min data value                     */
   float              mean;          /* mean data value in frame           */
   float              stddev;        /* STD of data in frame               */
   float              arcsec_pixel;  /* Number of arcseconds per pixel     */
	float              pos_angle;     /* position angle of image            */
   float              divisor;       /* Used by divbycoadds.               */
   float              itime;         /* itime */
	float              filter_zp;     /* filter zero point - default = 0    */
	float              ext_coff;      /* extinction co-efficient. default=0 */
	float              airmass;       /* airmass. default=0                 */
   short              Nheader;       /* Number of lines in the fits hdr.   */

	short              org_size;      /* original sizeof(pixel_data) in bytes */
	short              org_bitpix;    /* original bitpix value */
	float              org_bscale;    /* original bscale */
	float              org_bzero;     /* original bzero */
	int                sizeof_header; /* sizeof header in the file. */
	int                nframes;       /* number of movie frames ie: naxis3 */

   short              gbox_enable;   /* Is a guidebox associated with this data */
   int                gbox_dim[4];   /* guidebox x,y,wid,hgt */
   float              gbox_from[4];  /* guidebox from x,y */
   float              gbox_to[4];    /* guidebox to x,y */

   char               directory[DF_FITS_LEN_PATH];
   char               filename[DF_FITS_LEN_FILENAME];
   struct df_fheader_t * fheader;      /* Pointer to header block.           */
   float *            fdata;        /* Pointer to block of data           */
};

/*------------------------------------------------------------------
**  df_ Prototypes
**------------------------------------------------------------------
*/
int df_init( void );
int df_print_options( void );
int dfset_divbycoadd( int divbycoadd );
int dfget_divbycoadd( void );
int dfset_origin( int origin );
double dfdatamn( struct df_buf_t * bufp, int m, int n );
double dfdataxy( struct df_buf_t * bufp, int x, int y );
double dfdatainx( struct df_buf_t * bufp, int i );
double df_data_f( struct df_buf_t * bufp, int i );

int df_write_fits( int fd, char *path, char *filename, struct df_buf_t *bufp );

int df_read_fits( int fd, char *path, char *filename, struct df_buf_t *bufp, 
	int is_socket, int is_3d );

int df_search_fheader( struct df_fheader_t *hdr, char *keyword, char *val_str,
						  int val_str_size, struct df_fheader_t **Rhdr, int *offset, int debug);
int df_free_fbuffer( struct df_buf_t * bufp);

char * df_build_card( char *cptr, char *keyword, char *value, char *comment);

int df_buffer_math( struct df_buf_t * dest, struct df_buf_t * op1, 
						  struct df_buf_t * op2, int operation ); 
int df_constant_math( struct df_buf_t * dest, struct df_buf_t * op1, 
						  float op2, int operation ); 
int df_buffer_rotate( struct df_buf_t * dest, struct df_buf_t * op1, int operation);
int df_copy_subarray( struct df_buf_t * dest, struct df_buf_t * op1, 
							 int op_x, int op_y, int op_wid, int op_hgt);
int df_buffer_userfun( struct df_buf_t * dest, struct df_buf_t * op1,
	int (*userfun) ( struct df_buf_t *dest, struct df_buf_t *op1 ));
int df_math_copyheader( struct df_buf_t * dest, struct df_buf_t * src);
int df_math_fixheader( struct df_buf_t * bufp, char * history);

int df_stats( struct df_buf_t *bufp );
int df_stats_p1( struct df_buf_t *bufp );
int df_stats_p2( struct df_buf_t *bufp, double mean );

/****************************************************************************
** cbl related defines. CBL is Command Buffer List 
*****************************************************************************
*/

#define CBL_MAX_CMD_LEN    160

/* Special list commands */
#define CBL_CMD_COMMIT     0x80000000
#define CBL_CMD_UP         0x80000001
#define CBL_CMD_DOWN       0x80000002
#define CBL_CMD_LEFT       0x80000003
#define CBL_CMD_RIGHT      0x80000004
#define CBL_CMD_DELETE     0x80000005
#define CBL_CMD_BACKSPACE  0x80000006

/* Does the value fit the list command format? */
#define CBL_IS_VALID_LIST_CMD(c)   ((c) & 0x80000000))

/* Examine an error code to determine if it represents a failure value */
#define CBL_FAILED(e)      (((unsigned)(e)) & 0x80000000)

/* List entry */
typedef struct _cbl_entry
{
   char   cmd[CBL_MAX_CMD_LEN + 1];
}cbl_entry;

/* Command buffer list */
typedef struct _cbl_list
{
   cbl_entry*  entries;     // Committed entries
   cbl_entry   edited;      // The entry being edited now
   int  browsing;           // T/F, Is the list is being browsed?
   int  new_command;        // T/F, Is there a new command?
   int  current;            // Index of the current entry
   int  count;              // Number of elements in the list
   int  max_entries;        // Maximum number of entries in the list
   int  entry_cursor;       // The position of the insertion character for the current entry
} cbl_list;

/*--------------------------------
** cbl prototypes 
*/

cbl_list *cbl_new(int max_entries);
void cbl_destroy(cbl_list *pThis);
int  cbl_get_max_entries(const cbl_list *pThis);
int  cbl_get_count(const cbl_list *pThis);
int  cbl_has_new_command(const cbl_list *pThis);
void cbl_get_last_command(cbl_list *pThis, char *buf, int buf_size );
int  cbl_get_command(const cbl_list *pThis, int index, char *buf, int buf_size );
void cbl_get_current_command(const cbl_list *pThis, char *buf, int buf_size );
void cbl_set_current_command( cbl_list *pThis, const char *text);
int  cbl_get_entry_cursor(const cbl_list *pThis);
void cbl_set_entry_cursor( cbl_list *pThis, int cursor);
int  cbl_handle_command( cbl_list *pThis, int command);

/****************************************************************************
** clo related defines. CBL is Command Line Option function.
*****************************************************************************
*/

// clo options structure
struct clo_option_t
{
   int  type;          // CLO_TYPE_ - type of argument
   char * flag;        // command line flag string, ie: "-v"
   char * default_str; // Default value, as a sting
   void * user_var;    // pointer to user variable to hold option's value (must match type)
   int  sizeof_var;    // size of user_var (only important for char [])
};

// clo defines
#define CLO_TYPE_NO_ARG  0       // no argument - boolean type (assume int).
#define CLO_TYPE_INT32   1       // INT32   - 32bit int
#define CLO_TYPE_DOUBLE  2       // double  - double float.
#define CLO_TYPE_STRING  3       // char    - character string

// clo function prototypes
int clo_parse( int *argc, char *argv[], struct clo_option_t clo_options[], int num_clo_options);


#ifdef   __cplusplus
}
#endif

#endif /* __ir2_h */

