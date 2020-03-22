/* KallistiOS ##version##

   dc/sound/stream.h
   (c)2002 Dan Potter

   stream.h,v 1.1 2002/02/10 06:40:37 bardtx Exp

*/

#ifndef __DC_SOUND_STREAM_H
#define __DC_SOUND_STREAM_H

#include <sys/cdefs.h>
__BEGIN_DECLS

#include <arch/types.h>

/* Set "get data" callback */
void snd_stream_set_callback(void *(*func)(int));

/* Load sample data from SH-4 ram into SPU ram (auto-allocate RAM) */
uint32 snd_stream_load_sample(const uint16 *src, uint32 len);

/* Dump all loaded sample data */
void snd_stream_dump_samples();

/* Prefill buffers -- do this before calling start() */
void snd_stream_prefill();

/* Initialize stream system */
int snd_stream_init(void* (*callback)(int));

/* Shut everything down and free mem */
void snd_stream_shutdown();

/* Start streaming */
void snd_stream_start(uint32 freq, int st);

/* Stop streaming */
void snd_stream_stop();

/* Poll streamer to load more data if neccessary */
int snd_stream_poll();

/* Start a sound sample on the given channel */
void snd_stream_play_effect(int chn, uint32 src, uint32 freq,
	uint32 len, uint32 vol, uint32 pan);

/* Stop a sound sample on the given channel */
void snd_stream_stop_effect(int chn);

/* Set the volume on the streaming channels */
void snd_stream_volume(int vol);

__END_DECLS

#endif	/* __DC_SOUND_STREAM_H */

