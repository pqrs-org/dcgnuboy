/* KallistiOS ##version##

   abort.c
   (c)2001 Dan Potter

   abort.c,v 1.1 2002/02/09 06:15:43 bardtx Exp
*/

#include <arch/arch.h>

CVSID("abort.c,v 1.1 2002/02/09 06:15:43 bardtx Exp");

/* This is probably the closest mapping we've got for abort() */
void abort() {
	arch_exit();
}

