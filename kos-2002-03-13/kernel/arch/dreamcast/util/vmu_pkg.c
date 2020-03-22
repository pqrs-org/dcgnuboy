/* KallistiOS ##version##

   vmu_pkg.c
   (c)2002 Dan Potter
*/

#include <assert.h>
#include <stdlib.h>
#include <dc/vmu_pkg.h>

CVSID("vmu_pkg.c,v 1.1 2002/02/24 06:18:17 bardtx Exp");

/*

VMU data files can be stored raw, but if you want to interact with the
rest of the DC world, it's much better to package them in a nice data
file format. This module takes care of that for you.

Thanks to Marcus Comstedt for this information.

*/

/* CRC calculation: calculates the CRC on a VMU file to be written out */
static int vmu_pkg_crc(const uint8 * buf, int size) {
	int	i, c, n;
	
	for (i=0, n=0; i<size; i++) {
		n ^= (buf[i] << 8);
		for (c=0; c<8; c++) {
			if (n & 0x8000)
				n = (n << 1) ^ 4128;
			else
				n = (n << 1);
		}
	}

	return n & 0xffff;
}

/* Converts a vmu_pkg_t structure into an array of uint8's which may be
   written to a VMU file via fs_vmu, or whatever. */
int vmu_pkg_build(vmu_pkg_t *src, uint8 ** dst, int * dst_size) {
	uint8		*out;
	int		ec_size, out_size;
	vmu_hdr_t	*hdr;

	/* First off, figure out how big it will be */
	out_size = sizeof(vmu_hdr_t) + 512 * src->icon_cnt + src->data_len;
	switch(src->eyecatch_type) {
	case VMUPKG_EC_NONE:
		ec_size = 0; break;
	case VMUPKG_EC_16BIT:
		ec_size = 72 * 56 * 2; break;
	case VMUPKG_EC_256COL:
		ec_size = 512 + 72*56; break;
	case VMUPKG_EC_16COL:
		ec_size = 32 + 72*56/2; break;
	default:
		return -1;
	}
	out_size += ec_size;
	*dst_size = out_size;

	/* Allocate a return array */
	out = *dst = malloc(out_size);

	/* Setup some defaults */
	memset(out, 0, out_size);
	hdr = (vmu_hdr_t *)out;
	memset(hdr->desc_short, 32, sizeof(hdr->desc_short));
	memset(hdr->desc_long, 32, sizeof(hdr->desc_long));

	/* Fill in the data from the pkg struct */
	memcpy(hdr->desc_short, src->desc_short, strlen(src->desc_short));
	memcpy(hdr->desc_long, src->desc_long, strlen(src->desc_long));
	strcpy(hdr->app_id, src->app_id);
	hdr->icon_cnt = src->icon_cnt;
	hdr->icon_anim_speed = src->icon_anim_speed;
	hdr->eyecatch_type = src->eyecatch_type;
	hdr->crc = 0;
	hdr->data_len = src->data_len;
	memcpy(hdr->icon_pal, src->icon_pal, sizeof(src->icon_pal));
	out += sizeof(vmu_hdr_t);
	
	memcpy(out, src->icon_data, 512 * src->icon_cnt);
	out += 512 * src->icon_cnt;
	
	memcpy(out, src->eyecatch_data, ec_size);
	out += ec_size;

	memcpy(out, src->data, src->data_len);
	out += src->data_len;

	/* Verify the size */
	assert( (out - *dst) == out_size );
	out = *dst;

	/* Calculate CRC */
	hdr->crc = vmu_pkg_crc(out, out_size);

	return 0;
}

   
