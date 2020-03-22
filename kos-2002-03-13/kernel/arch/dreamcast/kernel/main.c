/* KallistiOS ##version##

   main.c
   (c)2000 Dan Potter
*/

extern int _bss_start, end;

#include <stdio.h>
#include <malloc.h>
#include <arch/syscall.h>
#include <arch/dbgio.h>
#include <arch/timer.h>
#include <arch/arch.h>
#include <arch/atexit.h>

CVSID("main.c,v 1.7 2002/02/22 07:41:00 bardtx Exp");

int	mm_init();

int	main(int argc, char **argv);

/* This is the entry point inside the C program */
int arch_main() {
	uint8 *bss_start = (uint8 *)(&_bss_start);
	uint8 *bss_end = (uint8 *)(&end);
	int rv;

#if 0
	/* Ensure that we pull in crtend.c in the linking process */
	__crtend_pullin();
#endif

	/* Clear out the BSS area */
	memset(bss_start, 0, bss_end - bss_start);

	/* Initialize memory management */
	mm_init();

#if 0
	/* Run ctors */
	arch_ctors();
#endif

	/* Call the user's main function */
	rv = main(0, NULL);

        arch_reboot ();

	return rv;
}

/* Called to shut down the system */
void arch_exit() {
	arch_atexit();

	dbglog(DBG_CRITICAL, "arch: shutting down kernel\n");

	/* Ensure that interrupts are disabled */
	irq_disable();
	irq_enable_exc();

	/* Shut down kernel functions */
	hardware_shutdown();

	/* Run dtors */
	arch_dtors();

	/* malloc_stats(); */

	/* Shut down Dreamcast functions */
	irq_shutdown();

	/* Jump back to the boot loader */
	arch_real_exit();
}

/* Called to shut down non-gracefully; assume the system is in peril
   and don't try to call the dtors */
void arch_abort() {
	dbglog(DBG_CRITICAL, "arch: aborting the system\n");

	irq_disable();
	arch_real_exit();
}

/* Called to reboot the system; assume the system is in peril and don't
   try to call the dtors */
void arch_reboot() {
	dbglog(DBG_CRITICAL, "arch: rebooting the system\n");

	/* Ensure that interrupts are disabled */
	irq_disable();

	/* Shut down Dreamcast functions */
	irq_shutdown();

	/* Reboot */
	{ void (*rb)() __noreturn = (void (*)())0xa0000000; rb(); }
}


/* When you make a function called main() in a GCC program, it wants
   this stuff too. */
void _main() { }

/* And some library funcs from newlib want this */
int __errno;

