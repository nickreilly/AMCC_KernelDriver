/* Copyright (C) 1990, 1995 by Motorola, Inc. */

#if ! defined ( __DSP561C__ ) && ! defined ( __DSP563C__ ) && ! defined ( __DSP566C__ )
#if ! defined ( alloca )

#define alloca( s ) __builtin_alloca ( s )

#endif
#endif
