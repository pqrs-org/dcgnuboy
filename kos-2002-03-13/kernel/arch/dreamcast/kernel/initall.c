/* KallistiOS ##version##

   initall.c
   (c)2001 Dan Potter
*/

/*

 The function in this file (arch_init_all) sets up all of the hardware
 and software subsystems in the OS so it's all ready to use. If you
 don't care about the size of your binary and/or limiting what gets
 included, you can use this function.

 This is in a seperate file to force seperate linking so it doesn't
 automatically include all that stuff.

 */

#include <stdio.h>
#include <assert.h>
#include <kos/fs.h>
#include <kos/thread.h>
#include <kos/fs_romdisk.h>
#include <kos/net.h>
#include <dc/fs_iso9660.h>
#include <dc/fs_vmu.h>
#include <dc/fs_dcload.h>
#include <dc/spu.h>
#include <dc/ta.h>
#include <dc/pvr.h>
#include <dc/maple.h>
#include <arch/irq.h>
#include <arch/timer.h>
#include <arch/dbgio.h>

CVSID("initall.c,v 1.8 2002/02/24 00:23:07 bardtx Exp");

extern const char banner[];

int kos_init_all(uint32 enables, void *romdisk) {
	/* If DC-Load is in use, then we'll want to enable that
	   before anything else happens. */
	fs_dcload_init_console();

	/* Do the banner */
	dbgio_init();
	dbglog(DBG_INFO, "\n--\n");		/* Spacer for multiple sessions */
	dbglog(DBG_INFO, banner);

	/* Initialize all of the basic hardware systems */
	irq_init();		/* IRQ */
	timer_init();		/* Timers */
	hardware_init();	/* DC Hardware */
	syscall_init();		/* System call interface */

	/* Initialize VFS */
	dbglog(DBG_KDEBUG, "Initializing VFS\n");
	fs_init();
	if (romdisk != NULL) {
		dbglog(DBG_KDEBUG, "Initializing RD file system\n");
		fs_romdisk_init(romdisk);
	}
	if (*DCLOADMAGICADDR == DCLOADMAGICVALUE) {
		dbglog(DBG_KDEBUG, "Initializing DCLOAD Console file system\n");
		fs_dcload_init();
	}
	dbglog(DBG_KDEBUG, "Initializing CD file system\n");
	fs_iso9660_init();
	dbglog(DBG_KDEBUG, "Initializing VMU file system\n");
	fs_vmu_init();

	/* Enable threads */
	if (enables & THD_ENABLE) {
		dbglog(DBG_KDEBUG, "Initializing threads\n");
		thd_init();
	}

	/* Enable IRQs */
	if (enables & IRQ_ENABLE) {
		dbglog(DBG_KDEBUG, "Enabling IRQs\n");
		irq_enable();
		maple_wait_scan();
	}

	/* Enable networking */
	if (enables & NET_ENABLE) {
		dbglog(DBG_KDEBUG, "Enabling networking\n");
		net_init();
	}

	/* This has to be done AFTER thread init, since it changes its
	   functionality to match the state of threading */
	/* spu_dma_init(); */

	/* Make sure we're not going to foible here */
	assert_msg(!( (enables & TA_ENABLE) && (enables & PVR_ENABLE) ),
		"PVR and TA are mutually exclusive subsystems; cannot proceed");

	/* Potentially enable TA */
	if (enables & TA_ENABLE) {
		dbglog(DBG_KDEBUG, "Initializing 3D system [ta]\n");
		/* dbglog(DBG_WARNING, "  The 'TA' system is deprecated in favor of 'PVR'\n"); */	/* not yet =) */
		ta_init_defaults();
	}

	/* Potentially enable PVR */
	if (enables & PVR_ENABLE) {
		/* This is about as close as we can get to reasonable defaults */
		pvr_init_params_t pvr_params = {
			{ PVR_BINSIZE_32, PVR_BINSIZE_8, PVR_BINSIZE_16, PVR_BINSIZE_8, PVR_BINSIZE_16 },
			512 * 1024
		};
		                
		dbglog(DBG_KDEBUG, "Initializing 3D system [pvr]\n");
		pvr_init(&pvr_params);
	}

	/* Enable millisecond timer */
	dbglog(DBG_KDEBUG, "Initializing Millisecond Timer\n");
	timer_ms_enable();

	return 0;
}

void kos_shutdown_all() {
	irq_disable();
	irq_enable_exc();
	/* spu_dma_shutdown(); */
	net_shutdown();
	thd_shutdown();
	pvr_shutdown();
	ta_shutdown();
	fs_dcload_shutdown();
	fs_vmu_shutdown();
	fs_iso9660_shutdown();
	fs_romdisk_shutdown();
	fs_shutdown();
	irq_shutdown();
}


