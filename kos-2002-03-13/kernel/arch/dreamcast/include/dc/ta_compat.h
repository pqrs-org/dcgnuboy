/* KallistiOS ##version##

   ta_compat.h
   (c)2002 Dan Potter

   ta_compat.h,v 1.4 2002/02/08 06:00:28 bardtx Exp
*/

#ifndef __DC_TACOMPAT_H
#define __DC_TACOMPAT_H

#ifdef __DC_TA_H
#	error ta_compat.h and ta.h cannot both be included
#endif

#include <sys/cdefs.h>
__BEGIN_DECLS

#include <assert.h>

/* Include the new PVR API */
#include <dc/pvr.h>

/* No equivalent in the new system unfortunately */
typedef struct {
	uint32	flags;
	float	x, y, z;
	float	a, r, g, b;
} vertex_oc_t;

typedef struct {
	uint32	flags;
	float	x, y, z, u, v;
	uint32	dummy1, dummy2;
	float	a, r, g, b;
	float	oa, or, og, ob;
} vertex_ot_t;

/* This has an equivalent, but we need different names */
typedef struct {
	uint32	flags1, flags2, flags3, flags4;
	uint32	dummy1, dummy2, dummy3, dummy4;
} poly_hdr_t;

/* Some commonly used stuff */
#define ta_init_defaults() do { \
	pvr_init_params_t	params = { \
		{ PVR_BINSIZE_32, PVR_BINSIZE_0, PVR_BINSIZE_32, PVR_BINSIZE_0, PVR_BINSIZE_0 }, \
		512 * 1024 \
	}; \
	pvr_init(&params); } while(0)

#define ta_shutdown() pvr_shutdown()

#define ta_txr_release_all() pvr_mem_reset()
#define ta_txr_allocate(s) (uint32)pvr_mem_malloc(s)
#define ta_txr_map(s) ((uint16 *)(s))
#define ta_txr_load(dst, src, count) pvr_txr_load(src, (pvr_ptr_t)(dst), count)

#define ta_commit_poly_hdr(hdr) pvr_prim(hdr, sizeof(poly_hdr_t))
#define ta_commit_vertex(v, sz) pvr_prim(v, sz)
#define ta_commit32_inline(src) assert_msg(0, "Not supported")
#define ta_commit32_nocopy() assert_msg(0, "Not supported")

/* These we'll actually handle in a compat module (a bit too complex for #define's) */
void pvrthunk_ta_begin_render();
void pvrthunk_ta_commit_eol();
void pvrthunk_ta_finish_frame();

void pvrthunk_ta_poly_hdr_col(poly_hdr_t *target, int translucent);
void pvrthunk_ta_poly_hdr_txr(poly_hdr_t *target, int translucent,
	int textureformat, int tw, int th, uint32 textureaddr,
	int filtering);

/* We rename these at compile time because otherwise the linker may get confused
   between the original TA and new PVR modules (we want to be able to put both
   in the same binary lib) */
#define ta_begin_render		pvrthunk_ta_begin_render
#define ta_commit_eol		pvrthunk_ta_commit_eol
#define ta_finish_frame		pvrthunk_ta_finish_frame
#define ta_poly_hdr_col		pvrthunk_ta_poly_hdr_col
#define ta_poly_hdr_txr		pvrthunk_ta_poly_hdr_txr

/* Texture format mappings */
#define TA_VERTEX_NORMAL		PVR_CMD_VERTEX
#define TA_VERTEX_EOL			PVR_CMD_VERTEX_EOL
#define TA_OPAQUE			PVR_LIST_OP_POLY
#define TA_TRANSLUCENT			PVR_LIST_TR_POLY
#define TA_NO_TEXTURE			0
#define TA_ARGB1555			(PVR_TXRFMT_ARGB1555 | PVR_TXRFMT_NONTWIDDLED)
#define TA_RGB565			(PVR_TXRFMT_RGB565   | PVR_TXRFMT_NONTWIDDLED)
#define TA_ARGB4444			(PVR_TXRFMT_ARGB4444 | PVR_TXRFMT_NONTWIDDLED)
#define TA_YUV422			(PVR_TXRFMT_YUV422   | PVR_TXRFMT_NONTWIDDLED)
#define TA_BUMP				(PVR_TXRFMT_BUMP     | PVR_TXRFMT_NONTWIDDLED)
#define TA_PAL4BPP			(PVR_TXRFMT_PAL4BPP  | PVR_TXRFMT_NONTWIDDLED)
#define TA_PAL8BPP			(PVR_TXRFMT_PAL8BPP  | PVR_TXRFMT_NONTWIDDLED)
#define TA_ARGB1555_TWID		(PVR_TXRFMT_ARGB1555 | PVR_TXRFMT_TWIDDLED)
#define TA_RGB565_TWID			(PVR_TXRFMT_RGB565   | PVR_TXRFMT_TWIDDLED)
#define TA_ARGB4444_TWID		(PVR_TXRFMT_ARGB4444 | PVR_TXRFMT_TWIDDLED)
#define TA_YUV422_TWID			(PVR_TXRFMT_YUV422   | PVR_TXRFMT_TWIDDLED)
#define TA_BUMP_TWID			(PVR_TXRFMT_BUMP     | PVR_TXRFMT_TWIDDLED)
#define TA_NO_FILTER			PVR_FILTER_NONE
#define TA_BILINEAR_FILTER		PVR_FILTER_BILINEAR

__END_DECLS

#endif	/* __DC_TA_H */

