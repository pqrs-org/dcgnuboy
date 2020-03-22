/* KallistiOS ##version

   tga.h
   (c)2000-2001 Benoit Miller

   tga.h,v 1.1 2002/02/23 04:42:43 axlen Exp

*/

#ifndef __TGA_H
#define __TGA_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <arch/types.h>

Image *tga_load(const char *fn);
void tga_free(Image *tga);

/* Loads a TGA file into texture RAM, potentially twiddling it. 
   TGA files include an alpha channel. */
int tga_load_texture(const char *fn, int twiddle, uint32 *txr, 
	int *w, int *h);

#if defined(__cplusplus)
}
#endif

#endif	/* __PCX_H */

