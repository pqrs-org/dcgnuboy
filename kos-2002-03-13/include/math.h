/* KallistiOS ##version##

   math.h
   (c)2001 Dan Potter

   math.h,v 1.4 2002/01/27 00:56:54 bardtx Exp
*/

#ifndef __KOS_MATH_H
#define __KOS_MATH_H

#include <sys/cdefs.h>
__BEGIN_DECLS

/* Just pull in the Newlib math routines for the right platform */
#ifdef _arch_dreamcast
#include <newlib-libm-sh4/math.h>
#elif _arch_gba
#error No math.h support for GBA yet
#else
#error Invalid architecture or no architecture specified
#endif

__END_DECLS

#endif	/* __KOS_MATH_H */

