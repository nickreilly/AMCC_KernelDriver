/*******************************************************************
** some function for help parse data type out of strings
** Same as parse.c but parse_r uses strtok_r() function.
** Note user must provide the st_ptr must be the same when parsing
** the same string.
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
#include <strings.h>
#include <math.h>
#include <sys/time.h>
 
#include "ir2.h"

/*----------------------------------------------------------
**  parseInt_r() - parse & convert to int.
**----------------------------------------------------------
*/
int parseInt_r( int * ip, char *buf, const char *tok, char **st_ptr )
{
   char * c;
	long l;
	
	if( (c = strtok_r( buf, tok, st_ptr) ) == NULL)
		return ERR_INV_FORMAT;

   if( -1 == my_atol( c, &l))
		return ERR_INV_FORMAT;

   *ip = (int)l;
   return ERR_NONE;
}

/*----------------------------------------------------------------
**  parseIntR_r() - parse & convert to int, with range checking
**---------------------------------------------------------------
*/
int parseIntR_r( int * ip, char *buf, const char *tok, char **st_ptr, int min, int max )
{
   int rc;

   if( (rc = parseInt_r( ip, buf, tok, st_ptr )) != ERR_NONE )
		return rc;

   if( !INRANGE( min, *ip, max ))
		return ERR_INV_RNG;

   return ERR_NONE;
}

/*----------------------------------------------------------
**  parseDouble_r() - parse & convert to double.
**----------------------------------------------------------
*/
int parseDouble_r( double *dp, char *buf, const char *tok, char **st_ptr )
{
   char * c;
	double d;
	
	if( (c = strtok_r( buf, tok, st_ptr) ) == NULL)
		return ERR_INV_FORMAT;

   if( -1 == my_atof( c, &d))
      return ERR_INV_FORMAT;

   *dp = d;
   return ERR_NONE;
}


/*--------------------------------------------------------------------
**  parseDoubleR_r() - parse & convert to double, with range checking
**--------------------------------------------------------------------
*/
int parseDoubleR_r( double *dp, char *buf, const char *tok, char **st_ptr, double min, double max )
{
   int rc;

   if( (rc = parseDouble_r( dp, buf, tok, st_ptr )) != ERR_NONE )
		return rc;

   if( !INRANGE( min, *dp, max ))
		return ERR_INV_RNG;

   return ERR_NONE;
}

/*----------------------------------------------------------
**  parseFloat_r() - parse & convert to float.
**----------------------------------------------------------
*/
int parseFloat_r( float *fp, char *buf, const char *tok, char **st_ptr )
{
   char * c;
	double d;
	
	if( (c = strtok_r( buf, tok, st_ptr) ) == NULL)
		return ERR_INV_FORMAT;

   if( -1 == my_atof( c, &d))
		return ERR_INV_FORMAT;

   *fp = d;
   return ERR_NONE;
}


/*--------------------------------------------------------------------
**  parseFloatR() - parse & convert to float, with range checking
**--------------------------------------------------------------------
*/
int parseFloatR_r( float *fp, char *buf, const char *tok, char **st_ptr, float min, float max )
{
   int rc;

   if( (rc = parseFloat_r( fp, buf, tok, st_ptr )) != ERR_NONE )
		return rc;

   /* printf("%7.5f %7.5f %7.5f\n", min, *fp, max); */
   if( !INRANGE( min, *fp, max ))
		return ERR_INV_RNG;

   return ERR_NONE;
}

/*--------------------------------------------------------------
**  parseSelection_r() - search a list for item.
**     Return:  >= 0         =  the index of the matching item.
**              ERR_INV_RNG  =  Item is not in list.
**--------------------------------------------------------------
*/
int parseSelection_r( 
   char * buf,     /* parameter for item to locate */
	char * tok,     /* token for parsing */
	char **st_ptr,  /* pointer for strtok_r() */
   char ** list    /* This list must be NULL terminaled */
)
{
   int i;
	char *c;
 
	if( (c = strtok_r( buf, tok, st_ptr) ) == NULL)
		return ERR_INV_FORMAT;

   for( i=0; list[i] != NULL; i++)
   {
     if( !stricmp( c, list[i] ))
       return i;
   }
   return ERR_INV_RNG;
}

/*--------------------------------------------------------------------
**  parseString() - parse & copies a string into buf. 
**--------------------------------------------------------------------
*/
int parseString_r( char *outb, int outb_size, char *buf, const char *tok, char ** st_ptr  )
{
	char *c;

	if( (c = strtok_r( buf, tok, st_ptr) ) == NULL)
		return ERR_INV_FORMAT;

   strxcpy( outb, c, outb_size);
	return ERR_NONE;
}
