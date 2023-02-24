/* (C) 1990, 1991, 1992 Motorola, Inc. */

/* added for vprintf, etc  prototypes */
#include <stdarg.h>

#if ! defined( __STDIO )
# define __STDIO

/* TYPEDEF SECTION. */

#if ! defined( __SIZE_T )
typedef unsigned int size_t;
#define __SIZE_T
#endif

typedef struct
{
    int _cnt;         /* for READ streams, the number of chars remaining in buffer.
		         for WRITE, the number of chars in buffer. */
    int _flag;        /* IO stream status */
    int _file;        /* descriptor */
    int   _bufsiz;    /* just in case changed by setvbuf */
    char* _buffer;    /* working buffer */
    char* _ptr;       /* current possition in buffer */
}
FILE;
extern FILE __streams[];

typedef long fpos_t;

/* MACRO DEFINITION SECTION. */

#if ! defined( NULL )
#define	NULL		((void*)0)
#endif

/** currently _flag is of size INT and for the 56156, int is 16-bits - so
 ** be careful when adding flags! **/
#define _IOFBF   0x0000     /* input and output fully buffered */
#define _IOLBF   0x0001     /* input and output line buffered */
#define _IONBF   0x0002     /* input and output unbuffered */
#define _IOABUF  0x0004     /* allocated buffer */
#define _IOWRITE 0x0008     /* stream is ready for writing */
#define _IOREAD  0x0010     /* stream is ready for reading */
#define _IOERR   0x0020     /* stream error has occured */
#define _IOEOF   0x0040     /* stream "end of file" occured */
#define _IOBIN   0x0080     /* stream opened BINARY */
#define _IORW    0x0100     /* "update" stream -- read/write */
#define _IOTMPF  0x0200     /* stream opened by tmpfile() */

#define	BUFSIZ   256        /* stream buffer size, also used by setbuf() */

#define EOF (-1)

#define __GET_STDIO
#include <ioprim.h>
#undef __GET_STDIO

#define FILENAME_MAX 256

#define L_tmpnam 9          /* MSDOS helped define this number */

/* NON-ansi, used to identify sprintf/sscanf "files" */
#define NON_FILE -1

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2 

#define TMP_MAX 25 

#define	stdin (&__streams[0])
#define	stdout (&__streams[1])
#define	stderr (&__streams[2])

/* FUNCTION PROTOTYPES */

int remove ( const char* );
int rename ( const char*, const char* );
FILE* tmpfile ( void );
char* tmpnam ( char* );
int fclose ( FILE* );
int fflush ( FILE* );
FILE* fopen ( const char*, const char* );
FILE* freopen ( const char*, const char*, FILE* );
void setbuf ( FILE*, char* );
int setvbuf ( FILE*, char*, int, size_t );
int fprintf ( FILE*, const char*, ... );
int fscanf ( FILE*, const char*, ... );
int printf ( const char*, ... );
int scanf ( const char*, ... );
int sprintf ( char*, const char*, ... );
int sscanf ( const char*, const char*, ... );
int vfprintf ( FILE*, const char*, va_list );
int vprintf ( const char*, va_list );
int vsprintf ( char*, const char*, va_list );
int fgetc ( FILE* );
char* fgets ( char*, int, FILE* );
int fputc ( int, FILE* );
int fputs ( const char*, FILE* );
int getc ( FILE* );
int getchar ( void );
char* gets ( char* );
int putc ( int, FILE* );
int putchar ( int );
int puts ( const char* );
int ungetc ( int, FILE* );
size_t fread ( void*, size_t, size_t, FILE* );
size_t fwrite ( const void*, size_t, size_t, FILE* );
int fgetpos ( FILE*, fpos_t* );
int fseek ( FILE*, long int, int );
int fsetpos ( FILE*, const fpos_t* );
long int ftell ( FILE* );
void rewind ( FILE* );
void clearerr ( FILE* );
int feof ( FILE* );
int ferror ( FILE* );
void perror ( const char* );

/* STDIO macro section */
#define putc(x, p) fputc(x,p)
#define putchar(x) fputc(x,stdout)
#define getc(p) fgetc(p)
#define getchar() fgetc(stdin)

#endif
