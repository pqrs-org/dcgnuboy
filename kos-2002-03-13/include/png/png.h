/* KallistiOS ##version##

   png.h
   (c)2002 Jeffrey McBeth

   png.h,v 1.2 2002/02/22 04:45:06 bardtx Exp
*/

#ifndef __PNG_PNG_H
#define __PNG_PNG_H

#include <sys/cdefs.h>
__BEGIN_DECLS

/* PNG_MASK_ALPHA currently doesn't work, I don't know why */
#define PNG_NO_ALPHA 0
#define PNG_MASK_ALPHA 1
#define PNG_FULL_ALPHA 2
  
/* Load a PNG file, allocating a texture, and returning the size of the file */ 
int png_load_texture(const char *filename, uint32 *tex, uint32 alpha, uint32 *w, uint32 *h);


/* Load a PNG file into a texture; returns 0 for success, -1 for failure. */
int png_to_texture(const char * filename, uint32 tex, uint32 alpha);

__END_DECLS

#endif	/* __PNG_PNG_H */


