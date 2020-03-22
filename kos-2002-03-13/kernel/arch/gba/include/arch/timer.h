/* KallistiOS ##version##

   arch/gba/include/timer.h
   (c)2000-2001 Dan Potter

   timer.h,v 1.1.1.1 2001/09/26 07:05:11 bardtx Exp

*/

#ifndef __ARCH_TIMER_H
#define __ARCH_TIMER_H

#include <arch/types.h>

/* Enable the main kernel timer */
void timer_enable_primary();
void timer_disable_primary();

/* Init function */
int timer_init();

/* Shutdown */
void timer_shutdown();

#endif	/* __ARCH_TIMER_H */

