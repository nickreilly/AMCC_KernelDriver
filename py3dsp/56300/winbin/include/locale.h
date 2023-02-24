/* Copyright (C) 1990, 1995 by Motorola, Inc. */

#if ! defined( __LOCALE )
#define __LOCALE

#if ! defined( NULL )
#define	NULL		0
#endif


#define	LC_ALL		0
#define	LC_COLLATE	1
#define	LC_CTYPE	2
#define	LC_MONETARY	3
#define	LC_NUMERIC	4
#define	LC_TIME		5


struct lconv
{
    char* decimal_point;	/* decimal point character */
    char* thousands_sep;	/* thousands separator character */
    char* grouping;	        /* grouping of digits */
    char* int_curr_symbol;	/* international currency symbol */
    char* currency_symbol;	/* local currency symbol */
    char* mon_decimal_point;	/* monetary decimal point character */
    char* mon_thousands_sep;	/* monetary thousands separator */
    char* mon_grouping;	        /* monetary grouping of digits */
    char* positive_sign;	/* monetary credit symbol */
    char* negative_sign;	/* monetary debit symbol */
    char  int_frac_digits;      /* intl monetary number of fractional digits */
    char  frac_digits;    	/* monetary number of fractional digits */
    char  p_cs_precedes;	/* true if currency symbol precedes credit */
    char  p_sep_by_space;	/* true if space separates c.s.  from credit */
    char  n_cs_precedes;	/* true if currency symbol precedes debit */
    char  n_sep_by_space;	/* true if space separates c.s.  from debit */
    char  p_sign_posn;	        /* position of sign for credit */
    char  n_sign_posn;    	/* position of sign for debit */
};


extern char*	        setlocale( int category, const char* locale );
extern struct lconv*	localeconv( void );


#endif
