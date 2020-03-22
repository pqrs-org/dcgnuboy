/* KallistiOS ##version##

   snd_stream.c
   (c)2000-2002 Dan Potter

   SH-4 support routines for SPU streaming sound driver
*/

#include <string.h>
#include <stdlib.h>

#include <arch/timer.h>
#include <dc/g2bus.h>
#include <dc/spu.h>
#include <dc/sound/stream.h>

#include "arm/aica_cmd_iface.h"

CVSID("snd_stream.c,v 1.3 2002/02/16 16:52:12 bardtx Exp");

/*

Here we implement a very simple double-buffering, not the rather complex
circular queueing done in most DMA'd sound architectures. This is done for
simplicity's sake. At some point we may upgrade that the circular queue
system. 

Basically this poll routine checks to see which buffer is playing, and
whether a new one is available or not. If not, we ask the user routine
for more sound data and load it up. That's about it.

*/


/* The last write position in the playing buffer */
static int last_write_pos = 0;
static int curbuffer = 0;

/* the address of the sound ram from the SH4 side */
#define SPU_RAM_BASE		0xa0800000

/* buffer size in bytes */
#define BUFFER_SIZE		0x10000

/* Seperation buffers (for stereo) */
static int16 *sep_buffer[2] = { NULL, NULL };

/* "Get data" callback; we'll call this any time we want to get another
   buffer of output data. */
static void* (*str_get_data)(int cnt) = NULL;

/* SPU RAM malloc pointer */
static uint32 ram_base, ram_top;

/* Stereo/mono flag for stream */
static int stereo;

/* This will come from a seperately linked object file */
extern uint8 snd_stream_drv[];
extern uint8 snd_stream_drv_end[];

/* Set "get data" callback */
void snd_stream_set_callback(void *(*func)(int)) {
	str_get_data = func;
}

/* "Kicks" the aica to process commands */
static void kick_aica(void) {
	g2_write_32(SPU_RAM_BASE + AICA_CMD, AICA_CMD_KICK);
	g2_fifo_wait();
	/* wait for the aica */
	while (1) {
		int res;
		
		res=g2_read_32(SPU_RAM_BASE + AICA_CMD);
		g2_fifo_wait();
		if (!res)
			return;
	}
}

/* Performs stereo seperation for the two channels; this routine
   has been optimized for the SH-4. */
static void sep_data(void *buffer, int len) {
	register int16	*bufsrc, *bufdst;
	register int	x, y, cnt;

	if (stereo) {
		bufsrc = (int16*)buffer;
		bufdst = sep_buffer[0];
		x = 0; y = 0; cnt = len / 2;
		do {
			*bufdst = *bufsrc;
			bufdst++; bufsrc+=2; cnt--;
		} while (cnt > 0);

		bufsrc = (int16*)buffer; bufsrc++;
		bufdst = sep_buffer[1];
		x = 1; y = 0; cnt = len / 2;
		do {
			*bufdst = *bufsrc;
			bufdst++; bufsrc+=2; cnt--;
			x+=2; y++;
		} while (cnt > 0);
	} else {
		memcpy(sep_buffer[0], buffer, len);
		memcpy(sep_buffer[1], buffer, len);
	}
}

/* Load sample data from SH-4 ram into SPU ram (auto-allocate RAM) */
uint32 snd_stream_load_sample(const uint16 *src, uint32 len) {
	uint32 where;
	
	where = ram_top;
	spu_memload(where, (uint8*)src, len);
	ram_top = (ram_top + len + 3) & (~3);
	
	return where;
}

/* Dump all loaded sample data */
void snd_stream_dump_samples() {
	ram_top = ram_base;
}

/* Prefill buffers -- do this before calling start() */
void snd_stream_prefill() {
	void *buf;

	if (!str_get_data) return;

	/* Load first buffer */
	if (stereo)
		buf = str_get_data(BUFFER_SIZE);
	else
		buf = str_get_data((BUFFER_SIZE/2));
	sep_data(buf, (BUFFER_SIZE/2));
	spu_memload(0x11000 + (BUFFER_SIZE/2)*0, (uint8*)sep_buffer[0], (BUFFER_SIZE/2));
	spu_memload(0x21000 + (BUFFER_SIZE/2)*0, (uint8*)sep_buffer[1], (BUFFER_SIZE/2));

	/* Load second buffer */
	if (stereo)
		buf = str_get_data(BUFFER_SIZE);
	else
		buf = str_get_data((BUFFER_SIZE/2));
	sep_data(buf, (BUFFER_SIZE/2));
	spu_memload(0x11000 + (BUFFER_SIZE/2)*1, (uint8*)sep_buffer[0], (BUFFER_SIZE/2));
	spu_memload(0x21000 + (BUFFER_SIZE/2)*1, (uint8*)sep_buffer[1], (BUFFER_SIZE/2));

	/* Start with playing on buffer 0 */
	last_write_pos = 0;
	curbuffer = 0;
}

/* Initialize stream system */
int snd_stream_init(void* (*callback)(int)) {
	int amt;
	
	/* Create stereo seperation buffers */
	if (!sep_buffer[0]) {
		sep_buffer[0] = memalign(32, (BUFFER_SIZE/2));
		sep_buffer[1] = memalign(32, (BUFFER_SIZE/2));
	}

	/* Finish loading the stream driver */
	spu_disable();
	spu_memset(0, 0, 0x31000);
	amt = snd_stream_drv_end - snd_stream_drv;
	if (amt % 4)
		amt = (amt + 4) & ~3;
	spu_memload(0, snd_stream_drv, amt);

	spu_enable();
	timer_spin_sleep(10);

	ram_base = ram_top = AICA_RAM_START;

	/* Setup the callback */
	snd_stream_set_callback(callback);
	
	return 0;
}

/* Shut everything down and free mem */
void snd_stream_shutdown() {
	if (sep_buffer[0]) {
		free(sep_buffer[0]);	sep_buffer[0] = NULL;
		free(sep_buffer[1]);	sep_buffer[1] = NULL;
	}
}

/* Start streaming */
void snd_stream_start(uint32 freq, int st) {
	aica_channel newval;
	
	if (!str_get_data) return;

	stereo = st;

	/* Prefill buffers */
	snd_stream_prefill();

	/* Start streaming */
	/* Channel 0 */
	newval.cmd = AICA_CMD_START;
	newval.pos = 0x11000;
	newval.type = SM_16BIT;
	newval.length = (BUFFER_SIZE/2);
	newval.loop = 1;
	newval.loopstart = 0;
	newval.loopend = (BUFFER_SIZE/2);
	newval.freq = freq;
	newval.vol = 240;
	newval.pan = 0;
	spu_memload(AICA_CHANNEL(0), &newval, sizeof(newval));
	/* Channel 1 */
	newval.pos = 0x21000;
	newval.pan = 255;
	spu_memload(AICA_CHANNEL(1), &newval, sizeof(newval));
	/* Process the changes */
	kick_aica();
}

/* Stop streaming */
void snd_stream_stop() {
	aica_channel newval;
	
	if (!str_get_data) return;

	/* Stop stream */
	/* Channel 0 */
	newval.cmd = AICA_CMD_STOP;
	spu_memload(AICA_CHANNEL(0), &newval, sizeof(newval));
	/* Channel 1 */
	spu_memload(AICA_CHANNEL(1), &newval, sizeof(newval));
	kick_aica();
}

/* Poll streamer to load more data if neccessary */
int snd_stream_poll() {
	aica_channel	chan0;
	aica_channel	chan1;
	int		realbuffer;
	int		current_play_pos;
	int		needed_samples;
	void		*data;

	if (!str_get_data) return -1;

	/* Get "real" buffer */
	chan0.cmd = AICA_CMD_UPDATE | AICA_UPDATE_GET_POS;
	chan1.cmd = AICA_CMD_UPDATE | AICA_UPDATE_GET_POS;
	spu_memload(AICA_CHANNEL(0), &chan0, sizeof(chan0));
	spu_memload(AICA_CHANNEL(1), &chan1, sizeof(chan1));
	kick_aica();
	spu_memread(&chan0, AICA_CHANNEL(0), sizeof(chan0));
	spu_memread(&chan1, AICA_CHANNEL(1), sizeof(chan1));
	realbuffer = !((chan0.pos < (BUFFER_SIZE/4)) && (chan1.pos < (BUFFER_SIZE/4)));

#if 1
	current_play_pos = (chan0.pos < chan1.pos)?(chan0.pos):(chan1.pos);
	/* count just till the end of the buffer, so we don't have to
	   handle buffer wraps */
	if (last_write_pos <= current_play_pos)
		needed_samples = current_play_pos - last_write_pos;
	else
		needed_samples = (BUFFER_SIZE/2) - last_write_pos;
	/* round it a little bit */
	needed_samples &= ~0x7ff;
	//printf("last_write_pos %6i, current_play_pos %6i, needed_samples %6i\n",last_write_pos,current_play_pos,needed_samples);

	if (needed_samples > 0) {
		if (stereo)
			data = str_get_data(needed_samples * 4);
		else
			data = str_get_data(needed_samples * 2);
		if (data == NULL) {
			/* Fill the "other" buffer with zeros */
			spu_memset(0x11000 + (last_write_pos * 2), 0, needed_samples * 2);
			spu_memset(0x21000 + (last_write_pos * 2), 0, needed_samples * 2);
			return -1;
		}
		sep_data(data, needed_samples * 2);
		spu_memload(0x11000 + (last_write_pos * 2), (uint8*)sep_buffer[0], needed_samples * 2);
		spu_memload(0x21000 + (last_write_pos * 2), (uint8*)sep_buffer[1], needed_samples * 2);

		last_write_pos += needed_samples;
		if (last_write_pos>=(BUFFER_SIZE/2))
			last_write_pos -= (BUFFER_SIZE/2);
	}
#else
	/* Has the channel moved on from the "current" buffer? */
	if (curbuffer != realbuffer) {
		/* Yep, adjust "current" buffer and initiate a load */

		/*printf("Playing in buffer %d, loading %d\r\n",
			realbuffer, curbuffer);*/
		if (stereo)
			data = str_get_data(BUFFER_SIZE);
		else
			data = str_get_data((BUFFER_SIZE/2));
		if (data == NULL) {
			/* Fill the "other" buffer with zeros */
			spu_memset(0x11000 + (BUFFER_SIZE/2)*curbuffer, 0, (BUFFER_SIZE/2));
			spu_memset(0x21000 + (BUFFER_SIZE/2)*curbuffer, 0, (BUFFER_SIZE/2));
			/* Wait for the current buffer to complete */
			/* do {
				val = chans[0].pos;
				spu_write_wait();
				realbuffer = !(val < 0x4000);
				if (realbuffer != curbuffer) thd_pass();
			} while (curbuffer != realbuffer); */
			return -1;
		}
		sep_data(data, (BUFFER_SIZE/2));
		spu_memload(0x11000 + (BUFFER_SIZE/2)*curbuffer, (uint8*)sep_buffer[0], (BUFFER_SIZE/2));
		spu_memload(0x21000 + (BUFFER_SIZE/2)*curbuffer, (uint8*)sep_buffer[1], (BUFFER_SIZE/2));
		curbuffer = realbuffer;
	}
#endif	
	return 0;
}

/* Start a sound sample on the given channel */
void snd_stream_play_effect(int chn, uint32 src, uint32 freq, uint32 len, uint32 vol, uint32 pan) {
	aica_channel	chan;
	
	chan.cmd = AICA_CMD_START;
	chan.pos = src;
	chan.type = SM_16BIT;
	chan.length = len;
	chan.loop = 0;
	chan.loopstart = 0;
	chan.loopend = len;
	chan.freq = freq;
	chan.vol = vol;
	chan.pan = pan;
	spu_memload(AICA_CHANNEL(chn), &chan, sizeof(chan));
	kick_aica();
}

/* Stop a sound sample on the given channel */
void snd_stream_stop_effect(int chn) {
	aica_channel	chan;
	chan.cmd = AICA_CMD_STOP;
	spu_memload(AICA_CHANNEL(chn), &chan, sizeof(chan));
	kick_aica();
}

/* Set the volume on the streaming channels */
void snd_stream_volume(int vol) {
	aica_channel	chan;
	chan.cmd = AICA_CMD_UPDATE | AICA_UPDATE_SET_VOL;
	chan.vol = vol;
	spu_memload(AICA_CHANNEL(0), &chan, sizeof(chan));
	spu_memload(AICA_CHANNEL(1), &chan, sizeof(chan));
	kick_aica();
}


