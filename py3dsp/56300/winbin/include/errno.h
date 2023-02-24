/* Copyright (C) 1990, 1991, 1992, 1995 by Motorola, Inc. */

#if ! defined( __ERRNO )

#define __GET_ERRNO
#include <ioprim.h>
#undef __GET_ERRNO

extern int errno;

#endif /* __ERRNO */
