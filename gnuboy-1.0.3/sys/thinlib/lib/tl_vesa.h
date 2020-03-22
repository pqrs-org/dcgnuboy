/*
** thinlib (c) 2001 Matthew Conte (matt@conte.com)
**
**
** tl_vesa.h
**
** VESA code header
**
** tl_vesa.h,v 1.1.1.1 2002/04/09 14:41:22 tekezo Exp
*/

#ifndef _TL_VESA_H_
#define _TL_VESA_H_

#include "tl_bmp.h"

extern int thin_vesa_init(int width, int height, int bpp, int param);
extern void thin_vesa_shutdown(void);

extern int thin_vesa_setmode(int width, int height, int bpp);

extern bitmap_t *thin_vesa_lockwrite(void);
extern void thin_vesa_freewrite(int num_dirties, rect_t *dirty_rects);

#endif /* !_TL_VESA_H_ */

/*
** tl_vesa.h,v
** Revision 1.1.1.1  2002/04/09 14:41:22  tekezo
**
**
*/
