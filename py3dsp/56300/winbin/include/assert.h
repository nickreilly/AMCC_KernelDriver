/* Copyright (C) 1990, 1995 by Motorola, Inc. */

#undef assert

#if defined( NDEBUG )

#define	assert( expression )	((void)0)

#else

#define	assert( expression ) \
    (( expression ) ? 1 : \
     ( printf ( "Assertion failed: file %s, line %d\n", __FILE__, __LINE__ ),\
       abort ( )))

#endif
