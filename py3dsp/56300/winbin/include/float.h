/* Copyright (C) 1990, 1995 by Motorola, Inc. */

#if ! defined( __FLOAT )
#define __FLOAT

#if defined ( __DSP56K__ ) || defined( __DSP563C__ )

/* all floating point types are the same on the 56k. */

#define	FLT_ROUNDS	1
#define	FLT_RADIX	2

#define	FLT_MANT_DIG	23
#define	DBL_MANT_DIG	FLT_MANT_DIG
#define	LDBL_MANT_DIG	FLT_MANT_DIG

#define	FLT_DIG		6
#define	DBL_DIG		FLT_DIG
#define	LDBL_DIG	FLT_DIG

#define	FLT_MIN_EXP	-125
#define DBL_MIN_EXP	FLT_MIN_EXP
#define LDBL_MIN_EXP	FLT_MIN_EXP

#define	FLT_MIN_10_EXP	-37
#define DBL_MIN_10_EXP	FLT_MIN_10_EXP
#define LDBL_MIN_10_EXP	FLT_MIN_10_EXP

#define	FLT_MAX_EXP	128
#define DBL_MAX_EXP	FLT_MAX_EXP
#define LDBL_MAX_EXP	FLT_MAX_EXP

#define	FLT_MAX_10_EXP	38
#define DBL_MAX_10_EXP	FLT_MAX_10_EXP
#define LDBL_MAX_10_EXP	FLT_MAX_10_EXP

#define	FLT_MAX		3.402823466e+38
#define	DBL_MAX		FLT_MAX
#define	LDBL_MAX	FLT_MAX

#define FLT_EPSILON	4.76E-7
#define	DBL_EPSILON	FLT_EPSILON
#define	LDBL_EPSILON	FLT_EPSILON

#define	FLT_MIN		1.175494351e-38
#define	DBL_MIN		FLT_MIN
#define	LDBL_MIN	FLT_MIN

#elif defined ( __DSP561C__ ) || defined( __DSP566C__ )

/* all floating point types are the same on the 56k. */

#define	FLT_ROUNDS	1
#define	FLT_RADIX	2

#define	FLT_MANT_DIG	23
#define	DBL_MANT_DIG	FLT_MANT_DIG
#define	LDBL_MANT_DIG	FLT_MANT_DIG

#define	FLT_DIG		6
#define	DBL_DIG		FLT_DIG
#define	LDBL_DIG	FLT_DIG

#define	FLT_MIN_EXP	-125
#define DBL_MIN_EXP	FLT_MIN_EXP
#define LDBL_MIN_EXP	FLT_MIN_EXP

#define	FLT_MIN_10_EXP	-37
#define DBL_MIN_10_EXP	FLT_MIN_10_EXP
#define LDBL_MIN_10_EXP	FLT_MIN_10_EXP

#define	FLT_MAX_EXP	128
#define DBL_MAX_EXP	FLT_MAX_EXP
#define LDBL_MAX_EXP	FLT_MAX_EXP

#define	FLT_MAX_10_EXP	38
#define DBL_MAX_10_EXP	FLT_MAX_10_EXP
#define LDBL_MAX_10_EXP	FLT_MAX_10_EXP

#define	FLT_MAX		3.402823466e+38
#define	DBL_MAX		FLT_MAX
#define	LDBL_MAX	FLT_MAX

#define FLT_EPSILON	4.76E-7
#define	DBL_EPSILON	FLT_EPSILON
#define	LDBL_EPSILON	FLT_EPSILON

#define	FLT_MIN		1.175494351e-38
#define	DBL_MIN		FLT_MIN
#define	LDBL_MIN	FLT_MIN

#else      /* __DSP96K__ */

extern int __flt_rounds ( );

# define FLT_ROUNDS		__flt_rounds ( )/* check the SR bits */
# define FLT_RADIX		2		/* radix of exponent */

# define FLT_MANT_DIG		24		/* # of bits in mantissa */
/* decimal digits of precision */
# define FLT_DIG		6
# define FLT_MIN_EXP		-125		/* min binary exponent */
# define FLT_MIN_10_EXP		-37		/* min decimal exponent */
# define FLT_MAX_EXP		128		/* max binary exponent */
# define FLT_MAX_10_EXP		38		/* max decimal exponent */
# define FLT_MAX		3.402823466e+38	/* max value */
/* smallest such that 1.0+FLT_EPSILON != 1.0 */	
# define FLT_EPSILON		1.192092896e-07
# define FLT_MIN		1.175494351e-38	/* min positive value */

# define DBL_MANT_DIG		32
# define DBL_DIG		9
# define DBL_MIN_EXP		-1021
# define DBL_MIN_10_EXP		-307
# define DBL_MAX_EXP		1024
# define DBL_MAX_10_EXP		308
# define DBL_MAX		1.79769313444e+308
# define DBL_EPSILON		4.65661287308e-10
# define DBL_MIN		2.22507385851e-308

/* long double and double are currently implemented with IEEE SEP on the
   96k. */

# define LDBL_MANT_DIG		DBL_MANT_DIG
# define LDBL_DIG		DBL_DIG
# define LDBL_MIN_EXP		DBL_MIN_EXP
# define LDBL_MIN_10_EXP 	DBL_MIN_EXP
# define LDBL_MAX_EXP		DBL_MAX_EXP
# define LDBL_MAX_10_EXP	DBL_MAX_10_EXP
# define LDBL_MAX		DBL_MAX
# define LDBL_EPSILON		DBL_EPSILON
# define LDBL_MIN		DBL_MIN

#endif     /* __DSP96K__ */
#endif
