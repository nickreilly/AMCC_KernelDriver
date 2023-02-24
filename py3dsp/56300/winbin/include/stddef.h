/* Copyright (C) 1990, 1995 by Motorola, Inc. */

#if ! defined( __STDDEF )
#define __STDDEF

typedef int ptrdiff_t;

#if ! defined( __SIZE_T )

typedef unsigned int size_t;

#define __SIZE_T
#endif

#if ! defined( __WCHAR_T )

typedef int wchar_t;

#define __WCHAR_T
#endif

#if ! defined( NULL )
#define	NULL				((void*)0)
#endif

#if ! defined( offsetof )
#define	offsetof( type, member )	((size_t)&(((type*)0)->member))
#endif

#endif
