/* KallistiOS ##version##

   pvr_palette.c
   (C)2002 Dan Potter

 */

#include <assert.h>
#include <dc/pvr.h>
#include "pvr_internal.h"

/*
   In addition to its 16-bit truecolor modes, the PVR also supports some
   nice paletted modes. These aren't useful for super high quality images
   most of the time, but they can be useful for doing some interesting
   special effects, like the old cheap "worm hole".
*/

CVSID("pvr_palette.c,v 1.1 2002/01/27 06:12:49 bardtx Exp");

/* Set the palette format */
void pvr_set_pal_format(int fmt) {
	assert_msg(0, "not implemented yet");
}

