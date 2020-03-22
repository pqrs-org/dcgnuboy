/* KallistiOS ##version##

   dc/maple/vmu.h
   (C)2000-2002 Jordan DeLong and Dan Potter

   vmu.h,v 1.1 2002/02/22 07:32:41 bardtx Exp

*/

#ifndef __DC_MAPLE_VMU_H
#define __DC_MAPLE_VMU_H

#include <sys/cdefs.h>
__BEGIN_DECLS

#include <arch/types.h>

int vmu_draw_lcd(uint8 addr, void *bitmap);
int vmu_block_read(uint8 addr, uint16 blocknum, uint8 *buffer);
int vmu_block_write(uint8 addr, uint16 blocknum, uint8 *buffer);

/* Init / Shutdown */
int vmu_init();
void vmu_shutdown();

__END_DECLS

#endif	/* __DC_MAPLE_VMU_H */

