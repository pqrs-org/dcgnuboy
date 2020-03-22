/*
** thinlib (c) 2001 Matthew Conte (matt@conte.com)
**
**
** tl_prof.c
**
** Screen border color profiler nastiness.
**
** tl_prof.c,v 1.1.1.1 2002/04/09 14:41:22 tekezo Exp
*/

#include <pc.h>
#include "tl_types.h"
#include "tl_prof.h"

void thin_prof_setborder(int pal_index)
{
   inportb(0x3DA);
   outportb(0x3C0, 0x31);
   outportb(0x3C0, (uint8) pal_index);
}

/*
** tl_prof.c,v
** Revision 1.1.1.1  2002/04/09 14:41:22  tekezo
**
**
*/
