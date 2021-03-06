/* KallistiOS ##version##

   dc/vmu_pkg.h
   (c)2002 Dan Potter

   vmu_pkg.h,v 1.1 2002/02/24 06:18:32 bardtx Exp

*/

#ifndef __DC_VMU_PKG_H
#define __DC_VMU_PKG_H

#include <sys/cdefs.h>
__BEGIN_DECLS

#include <arch/types.h>

/* Anyone wanting to package a VMU file should create one of these
   somewhere; eventually it will be turned into a flat file that you
   can save using fs_vmu. */
typedef struct vmu_pkg {
	char		desc_short[20];		/* Short file description */
	char		desc_long[36];		/* Long file description */
	char		app_id[20];		/* Application ID */
	int		icon_cnt;		/* Number of icons */
	int		icon_anim_speed;	/* Icon animation speed */
	int		eyecatch_type;		/* "Eyecatch" type */
	int		data_len;		/* Number of data (payload) bytes */
	uint16		icon_pal[16];		/* Icon palette (ARGB4444) */
	uint8		*icon_data;		/* 512*n bytes of icon data */
	uint8		*eyecatch_data;		/* Eyecatch data */
	uint8		*data;			/* Payload data */
} vmu_pkg_t;

/* Final header format (will go into the VMU file itself) */
typedef struct vmu_hdr {
	char		desc_short[16];		/* Space-padded */
	char		desc_long[32];		/* Space-padded */
	char		app_id[16];		/* Null-padded */
	uint16		icon_cnt;
	uint16		icon_anim_speed;
	uint16		eyecatch_type;
	uint16		crc;
	uint32		data_len;
	uint8		reserved[20];
	uint16		icon_pal[16];
	/* 512*n Icon Bitmaps */
	/* Eyecatch palette + bitmap */
} vmu_hdr_t;

/* Eyecatch types: all eyecatches are 72x56, but the pixel format is variable: */
#define VMUPKG_EC_NONE		0
#define VMUPKG_EC_16BIT		1	/* 16-bit ARGB4444 */
#define VMUPKG_EC_256COL	2	/* 256-color palette */
#define VMUPKG_EC_16COL		3	/* 16-color palette */
/* Note that in all of the above cases which use a palette, the palette entries
   are in ARGB4444 format and come directly before the pixel data itself. */

/* Converts a vmu_pkg_t structure into an array of uint8's which may be
   written to a VMU file via fs_vmu, or whatever. */
int vmu_pkg_build(vmu_pkg_t *src, uint8 ** dst, int * dst_size);


__END_DECLS

#endif	/* __DC_VMU_PKG_H */

