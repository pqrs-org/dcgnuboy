#include <stdlib.h>
#include "dc_sound.h"
#include <dc/g2bus.h>

#include "defs.h"
#include "pcm.h"
#include "rc.h"
#include "gnuboy.h"


/* SOUND_BUF_LEN = 2 * (rate * buffer_length_in_frames / 60.0) * (bit / 8) */
/* (format "%x" ((lambda (x) (* 2 (/ (* 44100 x) 60) (/ 16 8))) 3)) */
/* NOTE: AICA require 32-bit (or higher) alignment. */
#define SAMPLE_RATE 44100
#define SAMPLE_BITS 16
#define SOUND_BUF_LEN 0x2000

struct pcm pcm;

static uint8 buf[SOUND_BUF_LEN / 2] __attribute__ ((aligned (32)));

rcvar_t pcm_exports[] =
{
	RCV_END
};


void pcm_clear_buffer()
{
	spu_memset (dc_sound_get_baseaddr (), 0, SOUND_BUF_LEN);
}


void pcm_init()
{
	pcm.hz = SAMPLE_RATE;
        pcm.len = sizeof(buf) / 2; /* sizeof(buf) / sizeof(*(pcm.buf)) */
	pcm.stereo = 0;
	pcm.buf = buf;
	pcm.pos = 0;
	
	pcm_clear_buffer ();
	dc_sound_init (SAMPLE_RATE, SAMPLE_BITS, SOUND_BUF_LEN);
}


void pcm_close()
{
	dc_sound_shutdown ();
}


void
render_pcm ()
{
	static un32 last_pos = 0;
	un32 cur_pos = (dc_sound_get_position () * (SAMPLE_BITS / 8)) > (SOUND_BUF_LEN / 2);
        
	if (last_pos != cur_pos)
	{
#if 0
		pcm.pos = 0;
		sound_mix (sizeof(buf) / 2);
#else
		sound_mix (pcm.len - pcm.pos);
#endif
		spu_memload (dc_sound_get_baseaddr () + sizeof(buf) * last_pos,
			     buf, sizeof(buf));
		last_pos = cur_pos;
		pcm.pos = 0;
	}
}


