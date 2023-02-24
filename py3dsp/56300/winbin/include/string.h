/* Copyright (C) 1990, 1995 by Motorola, Inc. */

#if ! defined( __STRING )
#define __STRING

#if ! defined( NULL )
#define NULL		((void*)0)
#endif

#if ! defined( __SIZE_T )

typedef unsigned int size_t;

#define	__SIZE_T
#endif


extern int   memcmp  ( const void*, const void*, size_t );
extern void* memcpy  ( void*, const void*, size_t );
extern void* memmove ( void*, const void*, size_t );
extern void* memchr  ( const void*, int, size_t );
extern void* memset  ( void*, int, size_t );

extern char* strncpy ( char*, const char*, size_t );
extern char* strcat  ( char*, const char* );
extern char* strncat ( char*, const char*, size_t );
extern int   strcoll ( const char*, const char* );
extern int   strncmp ( const char*, const char*, size_t );
extern size_t strxfrm ( char*, const char*, size_t );
extern char* strchr  ( const char*, int );
extern size_t strcspn ( const char*, const char* );
extern char* strpbrk ( const char*, const char* );
extern char* strrchr ( const char*, int );
extern size_t strspn ( const char*, const char* );
extern char* strstr  ( const char*, const char* );
extern char* strtok  ( char*, const char* );
extern char* strerror ( int );
extern size_t strlen ( const char* );

extern char* strcpy ( char*, const char* );
extern int strcmp ( const char*, const char* );

#if defined ( __DSP56K__ )

#define	strcpy( s, d ) __builtin_strcpy ( s, d )
__inline__ static char*
__builtin_strcpy ( char* dst, char* src )
{
    int tmp;
    char* res = dst;
    
#if defined( __X_MEMORY )
    __asm volatile ( "\n"
"_begin_cpy\n"
"	move	x:(%2)+,%3\n"
"	tst	%3	%3,x:(%1)+\n"
"	jneq	_begin_cpy\n"

		: "=*S*D" ( res ), "=&A" ( dst ), "=&A" ( src ), "=&D" ( tmp )
		: "0" ( res ), "1" ( dst ), "2" ( src ));
#else
    __asm volatile ( "\n"
"_begin_cpy\n"
"	move	y:(%2)+,%3\n"
"	tst	%3	%3,y:(%1)+\n"
"	jneq	_begin_cpy\n"

		: "=*S*D" ( res ), "=&A" ( dst ), "=&A" ( src ), "=&D" ( tmp )
		: "0" ( res ), "1" ( dst ), "2" ( src ));
#endif

    return res;
}

#define strcmp __builtin_strcmp
__inline__ static int
__builtin_strcmp ( const char* s1, const char* s2 )
{
    int op1, op2, tmp = 0;
    
#if defined( __X_MEMORY )
    __asm ( "\n"
"	move	x:(%2)+,%0\n"
"	move	x:(%3)+,%1\n"
"_begin_cmp\n"
"	cmp	%1,%0	x:(%3)+,%1\n"
"	tneq	%4,%0\n"
"	tst	%0	x:(%2)+,%0\n"
"	jneq	_begin_cmp\n"
"	move	(%2)-\n"
"	move	(%3)-\n"
"	move	x:-(%2),%0\n"
"	move	x:-(%3),%1\n"
"	sub	%1,%0\n"

	: "=&D" ( op1 ), "=&D" ( op2 ), "=&A" ( s1 ), "=&A" ( s2 )
	: "S" ( tmp ), "2" ( s1 ), "3" ( s2 ));
#else
    __asm ( "\n"
"	move	y:(%2)+,%0\n"
"	move	y:(%3)+,%1\n"
"_begin_cmp\n"
"	cmp	%1,%0	y:(%3)+,%1\n"
"	tneq	%4,%0\n"
"	tst	%0	y:(%2)+,%0\n"
"	jneq	_begin_cmp\n"
"	move	(%2)-\n"
"	move	(%3)-\n"
"	move	y:-(%2),%0\n"
"	move	y:-(%3),%1\n"
"	sub	%1,%0\n"

	: "=&D" ( op1 ), "=&D" ( op2 ), "=&A" ( s1 ), "=&A" ( s2 )
	: "S" ( tmp ), "2" ( s1 ), "3" ( s2 ));
#endif

    return op1;
}

#elif defined ( __DSP563C__ ) || defined ( __DSP566C__ )

#define	strcpy( s, d ) __builtin_strcpy ( s, d )
__inline__ static char*
__builtin_strcpy ( char* dst, char* src )
{
    int tmp;
    char* res = dst;
    
#if defined( __X_MEMORY )
    __asm volatile ( "\n"
"_begin_cpy\n"
"	move	x:(%2)+,%3\n"
"	tst	%3	%3,x:(%1)+\n"
"	jneq	_begin_cpy\n"

		: "=S" ( res ), "=&A" ( dst ), "=&A" ( src ), "=&D" ( tmp )
		: "0" ( res ), "1" ( dst ), "2" ( src ));
#else
    __asm volatile ( "\n"
"_begin_cpy\n"
"	move	y:(%2)+,%3\n"
"	tst	%3	%3,y:(%1)+\n"
"	jneq	_begin_cpy\n"

		: "=S" ( res ), "=&A" ( dst ), "=&A" ( src ), "=&D" ( tmp )
		: "0" ( res ), "1" ( dst ), "2" ( src ));
#endif

    return res;
}

#define strcmp __builtin_strcmp
__inline__ static int
__builtin_strcmp ( const char* s1, const char* s2 )
{
    int op1, op2, tmp = 0;
    
#if defined( __X_MEMORY )
    __asm ( "\n"
"	move	x:(%2)+,%0\n"
"	move	x:(%3)+,%1\n"
"_begin_cmp\n"
"	cmp	%1,%0	x:(%3)+,%1\n"
"	tneq	%4,%0\n"
"	tst	%0	x:(%2)+,%0\n"
"	bneq	<_begin_cmp\n"
"	move	(%2)-\n"
"	move	(%3)-\n"
"	move	x:-(%2),%0\n"
"	move	x:-(%3),%1\n"
"	sub	%1,%0\n"

	: "=&D" ( op1 ), "=&D" ( op2 ), "=&A" ( s1 ), "=&A" ( s2 )
	: "S" ( tmp ), "2" ( s1 ), "3" ( s2 ));
#else
    __asm ( "\n"
"	move	y:(%2)+,%0\n"
"	move	y:(%3)+,%1\n"
"_begin_cmp\n"
"	cmp	%1,%0	y:(%3)+,%1\n"
"	tneq	%4,%0\n"
"	tst	%0	y:(%2)+,%0\n"
"	bneq	<_begin_cmp\n"
"	move	(%2)-\n"
"	move	(%3)-\n"
"	move	y:-(%2),%0\n"
"	move	y:-(%3),%1\n"
"	sub	%1,%0\n"

	: "=&D" ( op1 ), "=&D" ( op2 ), "=&A" ( s1 ), "=&A" ( s2 )
	: "S" ( tmp ), "2" ( s1 ), "3" ( s2 ));
#endif

    return op1;
}

#elif defined ( __DSP561C__ )

#define	strcpy( s, d ) __builtin_strcpy ( s, d )
__inline__ static char*
__builtin_strcpy ( char* dst, char* src )
{
    int tmp;
    char* res = dst;
    
    __asm volatile ( "\n"
"_begin_cpy\n"
"	move	x:(%2)+,%3\n"
"	tst	%3	%3,x:(%1)+\n"
"	bneq	<_begin_cpy\n"

		: "=DQ" ( res ), "=&A" ( dst ), "=&A" ( src ), "=&D" ( tmp )
		: "0" ( res ), "1" ( dst ), "2" ( src ));

    return res;
}


#define strcmp __builtin_strcmp
__inline__ static int
__builtin_strcmp ( const char* s1, const char* s2 )
{
    int op1, op2;
    
    __asm ( "\n"
"	move	x:(%2)+,%0\n"
"	move	#>0,x0\n"
"	move	x:(%3)+,%1\n"
"_begin_cmp\n"
"	cmp	%1,%0	x:(%3)+,%1\n"
"	tneq	x0,%0\n"
"	tst	%0	x:(%2)+,%0\n"
"	bneq	<_begin_cmp\n"
"	move	(%2)-\n"
"	move	(%3)-\n"
"	move	x:-(%2),%0\n"
"	move	x:-(%3),%1\n"
"	sub	%1,%0\n"

	: "=&D" ( op1 ), "=&D" ( op2 ), "=&A" ( s1 ), "=&A" ( s2 )
	: "2" ( s1 ), "3" ( s2 ) : "x0" );

    return op1;
}

#elif defined( __DSP96K__ )

#define	strcpy( s, d ) __builtin_strcpy ( s, d )
__inline__ static char*
__builtin_strcpy ( char* dst, char* src )
{
    int tmp;
    char* res = dst;
    
#if defined( __X_MEMORY )
    __asm volatile ( "\n"
"_begin_cpy\n"
"	move	x:(%2)+,%3.l\n"
"	tst	%3	%3.l,x:(%1)+\n"
"	jneq	_begin_cpy\n"

	       : "=*a*d" ( res ), "=&a" ( dst ), "=&a" ( src ),
	       "=&d" ( tmp )
	       : "0" ( res ), "1" ( dst ), "2" ( src ));
#else
    __asm volatile ( "\n"
"_begin_cpy\n"
"	move	y:(%2)+,%3.l\n"
"	tst	%3	%3.l,y:(%1)+\n"
"	jneq	_begin_cpy\n"

	       : "=*a*d" ( res ), "=&a" ( dst ), "=&a" ( src ),
	       "=&d" ( tmp )
	       : "0" ( res ), "1" ( dst ), "2" ( src ));
#endif

    return res;
}

#define strcmp __builtin_strcmp
__inline__ static int
__builtin_strcmp ( const char* s1, const char* s2 )
{
    int op1, op2;
    
#if defined( __X_MEMORY )
    __asm ( "\n"
"	move	x:(%2)+,%0.l\n"
"	move	x:(%3)+,%1.l\n"
"_begin_cmp\n"
"	cmp	%1,%0	x:(%3)+,%1.l\n"
"	clr	%0	ifneq\n"
"	tst	%0	x:(%2)+,%0.l\n"
"	jneq	_begin_cmp\n"
"	move	(%2)-\n"
"	move	(%3)-\n"
"	move	x:-(%2),%0.l\n"
"	move	x:-(%3),%1.l\n"
"	sub	%1,%0\n"

	: "=&d" ( op1 ), "=&d" ( op2 ), "=&a" ( s1 ), "=&a" ( s2 )
	: "2" ( s1 ), "3" ( s2 ));
#else
    __asm ( "\n"
"	move	y:(%2)+,%0.l\n"
"	move	y:(%3)+,%1.l\n"
"_begin_cmp\n"
"	cmp	%1,%0	y:(%3)+,%1.l\n"
"	clr	%0	ifneq\n"
"	tst	%0	y:(%2)+,%0.l\n"
"	jneq	_begin_cmp\n"
"	move	(%2)-\n"
"	move	(%3)-\n"
"	move	y:-(%2),%0.l\n"
"	move	y:-(%3),%1.l\n"
"	sub	%1,%0\n"

	: "=&d" ( op1 ), "=&d" ( op2 ), "=&a" ( s1 ), "=&a" ( s2 )
	: "2" ( s1 ), "3" ( s2 ));
#endif

    return op1;
}

#endif
#endif
