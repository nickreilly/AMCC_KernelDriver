#define EXTERN extern

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>

#include "ir2.h"

/*------------------------------------------------------------------------
**  str_replace_sub() - This function scans a src string for a pattern string.
**     The pattern string is replace with the val string. The results 
**     are copied into the dest string.
**  Returns the number of replacement done or -1 on error.
**------------------------------------------------------------------------
*/
int str_replace_sub 
( 
	char * src,      /* source string */
	char * dest,     /* destination string - results are copied here */
	int dest_size,   /* size of destination string in bytes */
	char * pattern,  /* the source string is scaned for this pattern */ 
	char * val       /* the patter is replaced by the val string */
)
{
    int dlen, 
		  vsize,
		  psize,
		  n, cnt;
    char *buf,
			*dptr;

	 if( NULL == (buf = malloc( dest_size )) )
		 return -1;

    vsize = strlen(val);
	 psize = strlen(pattern);
	 cnt = 0;
    dptr = buf;
    for(dlen=0; (dlen<dest_size-1) && *src;)
	 {
		 if( strncmp( src, pattern, psize)==0 )
		 {
			 n = MIN( vsize, dest_size-dlen);
			 memcpy( dptr, val, n);
			 dptr += n;            /* increment dest ptr over  value    */
			 dlen += n;
			 src += psize;         /* increment source ptr over pattern */
			 cnt++;                /* update count on number of replacements */
		 }
		 else
		 {
			 *dptr++ = *src++;
			 dlen++;
       }
	 }
	 *dptr = 0;    /* terminate string buffer */
	 dlen++;

	 memcpy( dest, buf, dlen);   /* copy to caller buffer */
	 free( buf );                /* free allocated memory */
	 return cnt;
}

