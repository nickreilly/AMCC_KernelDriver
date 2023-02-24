/* Copyright (C) 1990, 1995 by Motorola, Inc. */

#if ! defined( __SETJMP )
#define __SETJMP

#if defined( __DSP561C__ )

/* 14: sp, pc, and all callee-save registers are saved by setjmp. */
/* b[0-2], x[01], y[01], r[0-1,3] */
#define __JUMP_BUF_SIZE 14

#elif defined( __DSP96K__ )

/* 33: fp, sp, pc, and all callee-save registers are saved by setjmp. */
/* d[2-7].[hml], r[1-5,7], n[1-5,7] */
#define __JUMP_BUF_SIZE 33

#elif defined( __DSP56K__ )

/* 16: fp, sp, pc, and all callee-save registers are saved by setjmp. */
/* b[0-2], x[01], y[01], r[1-5,7] */
#define __JUMP_BUF_SIZE 16

#elif defined( __DSP563C__ ) || defined( __DSP566C__ )

/* 10: sp, pc, and all callee-save registers are saved by setjmp. */
/* y[01], r[2-3,7], n[2-3,7] */
#define __JUMP_BUF_SIZE 10

#endif

typedef unsigned int jmp_buf[ __JUMP_BUF_SIZE ];

int setjmp ( jmp_buf );
void longjmp ( jmp_buf, int );

#endif /* __SETJMP */
