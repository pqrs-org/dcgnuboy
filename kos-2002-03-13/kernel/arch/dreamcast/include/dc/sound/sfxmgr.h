/* KallistiOS ##version##

   dc/sound/sfxmgr.h
   (c)2002 Dan Potter

   sfxmgr.h,v 1.1 2002/02/10 06:40:37 bardtx Exp

*/

#ifndef __DC_SOUND_SFXMGR_H
#define __DC_SOUND_SFXMGR_H

#include <sys/cdefs.h>
__BEGIN_DECLS

#include <arch/types.h>

/* Load a sound effect from a WAV file and return a handle to it */
int snd_sfx_load(const char *fn);

/* Unload all loaded samples and free their SPU RAM */
void snd_sfx_unload_all();

/* Play a sound effect with the given volume and panning */
void snd_sfx_play(int idx, int vol, int pan);



__END_DECLS

#endif	/* __DC_SOUND_SFXMGR_H */

