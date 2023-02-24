/* Copyright (C) 1990, 1995 by Motorola, Inc. */

#if ! defined( __STDARG )
#define __STDARG

/* How stdargs works:
 *
 * va_list is the argument pointer type. it is used to point all sorts of
 * objects.
 *
 * va_start ( ap, first_param ) initializes ap to point to the first location
 * underneath the first param:

 ^ stack grows up ^
 ------------ 
 first_param
 ...
 ------------
 second_param  <- ap points here after va_start.
 ...
 ------------
 third_param
 ...
 ------------

 NOTE that ap points to the top word of the next param on the stack.

 * va_arg ( ap, type ) returns the next argument off the stack and changes ap
 * to point to the first location underneath that argument.

 * va_end ( ap ) does nothing at this time. you should still use it for future
 * compatability.
 */

typedef void* va_list;

#define	va_start( ap, parm )		(ap = (void*)(&parm - sizeof(parm)))

#define va_arg( ap, type )		\
    (ap = (void*)(((type*)ap)-1),*(type*)(void*)(((char*)ap)+1))
    
#define	va_end( ap )

#endif
