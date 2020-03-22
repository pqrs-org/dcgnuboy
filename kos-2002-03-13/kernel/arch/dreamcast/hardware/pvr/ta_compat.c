/* KallistiOS ##version##

   ta_compat.c
   (C)2002 Dan Potter

 */

#include <assert.h>
#include <string.h>
#include <dc/sq.h>
#include <dc/pvr.h>
#include <dc/ta_compat.h>
#include "pvr_internal.h"

/*

  Old "ta" module compatibility layer

*/

CVSID("ta_compat.c,v 1.4 2002/02/08 06:00:28 bardtx Exp");

static int current_list = -1;

void pvrthunk_ta_begin_render() {
	assert(current_list == -1);

	pvr_wait_ready();

	current_list = PVR_LIST_OP_POLY;
	pvr_scene_begin();
	pvr_list_begin(current_list);
}

void pvrthunk_ta_commit_eol() {
	if (current_list == PVR_LIST_OP_POLY) {
		current_list = PVR_LIST_TR_POLY;
		pvr_list_finish();
		pvr_list_begin(current_list);
	} else {
		assert(current_list == PVR_LIST_TR_POLY);
		pvr_list_finish();
	}
}

void pvrthunk_ta_finish_frame() {
	assert(current_list == PVR_LIST_TR_POLY);
	
	pvr_scene_finish();
	
	current_list = -1;
}

void pvrthunk_ta_poly_hdr_col(poly_hdr_t *target, int translucent) {
	pvr_poly_cxt_t context;
	
	pvr_poly_cxt_col(&context, (pvr_list_t)translucent);
	context.fmt.color = PVR_CLRFMT_4FLOATS;
	pvr_poly_compile((pvr_poly_hdr_t*)target, &context);
}

void pvrthunk_ta_poly_hdr_txr(poly_hdr_t *target, int translucent,
		int textureformat, int tw, int th, uint32 textureaddr,
		int filtering) {
	pvr_poly_cxt_t context;
	
	pvr_poly_cxt_txr(&context, (pvr_list_t)translucent,
		textureformat, tw, th, (pvr_ptr_t)textureaddr, filtering);
	context.fmt.color = PVR_CLRFMT_4FLOATS;
	pvr_poly_compile((pvr_poly_hdr_t*)target, &context);
}


