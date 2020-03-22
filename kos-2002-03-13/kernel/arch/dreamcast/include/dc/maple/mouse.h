/* KallistiOS ##version##

   dc/maple/mouse.h
   (C)2000-2002 Jordan DeLong and Dan Potter

   mouse.h,v 1.1 2002/02/22 07:32:41 bardtx Exp

*/

#ifndef __DC_MAPLE_MOUSE_H
#define __DC_MAPLE_MOUSE_H

#include <sys/cdefs.h>
__BEGIN_DECLS

#include <arch/types.h>

/* mouse defines */
#define MOUSE_RIGHTBUTTON	(1<<1)
#define MOUSE_LEFTBUTTON	(1<<2)
#define MOUSE_SIDEBUTTON	(1<<3)

#define MOUSE_DELTA_CENTER      0x200

/* mouse condition structure */
typedef struct {
	uint16 buttons;
	uint16 dummy1;
	int16 dx;
	int16 dy;
	int16 dz;
	uint16 dummy2;
	uint32 dummy3;
	uint32 dummy4;
} mouse_cond_t;

/* Old maple interface */
int mouse_get_cond(uint8 addr, mouse_cond_t *cond);

/* New interface (subject to change still...) */
typedef struct mouse_state {
	int	buttons;	/* Are buttons pressed? */
	int	x, y, z;	/* Mouse position */
} mouse_state_t;

int mouse_get_state(int port, int unit, mouse_state_t * out);

/* Init / Shutdown */
int	mouse_init();
void	mouse_shutdown();

__END_DECLS

#endif	/* __DC_MAPLE_MOUSE_H */

