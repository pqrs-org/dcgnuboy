/* KallistiOS ##version##

   pvr_texture.c
   (C)2002 Dan Potter

 */

#include <assert.h>
#include <dc/pvr.h>
#include <dc/sq.h>
#include "pvr_internal.h"

/*

   Texture handling

   Helper functions for handling texture tasks of various kinds.
 
*/

CVSID("pvr_texture.c,v 1.2 2002/01/29 07:40:03 bardtx Exp");


/* Load raw texture data from an SH-4 buffer into PVR RAM */
void pvr_txr_load(void * src, pvr_ptr_t dst, uint32 count) {
	if (count % 4)
		count = (count & 0xfffffffc) + 4;
	sq_cpy((uint32 *)dst, (uint32 *)src, count);
}

/* Load texture data from an SH-4 buffer into PVR RAM, twiddling it
   in the process */
void pvr_txr_load_twiddle(void * src, pvr_ptr_t dst, uint32 w, uint32 h) {
	assert_msg(0, "not implemented yet");
}

/* Load texture data from an SH-4 buffer into PVR RAM, twiddling it
   and scaling it in the process */
void pvr_txr_load_twiddle_scale(void * src, pvr_ptr_t dst,
		uint32 src_w, uint32 src_h, uint32 dst_w, uint32 dst_h) {
	assert_msg(0, "not implemented yet");
}

/* Load texture data from an SH-4 buffer into PVR RAM, running it
   through VQ compression and twiddling in the process. **NOT DONE YET** */
void pvr_txr_load_twiddle_vq(void * src, pvr_ptr_t dst, uint32 w, uint32 h) {
	assert_msg(0, "not implemented yet");
}




