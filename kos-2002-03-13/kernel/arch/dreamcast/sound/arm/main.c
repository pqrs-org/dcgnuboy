/* Streaming sound driver
 *
 * (c)2000 Dan Potter
 *
 * This slightly more complicated version allows for sound effect channels,
 * and full sampling rate, panning, and volume control for each. The two
 * streaming channels are still always first and always occur at 0x11000 and
 * 0x21000. All other sample data can begin at 0x31000. "pos" only works for
 * input on the two streaming channels (which will always have the same
 * "pos" value).
 * 
 */

#include "aica.h"
#include "aica_cmd_iface.h"

/****************** Timer *******************************************/

extern volatile int timer;

void timer_wait(int jiffies) {
	int fin = timer + jiffies;
	while (timer <= fin)
		;
}

/****************** Main Program ************************************/

/* Set channel id at 0x80280d (byte), read position at 0x802814 (long) */

volatile uint32 *cmd = (volatile uint32 *)AICA_CMD;
volatile aica_channel *chans = (volatile aica_channel *)AICA_CHANNELS;

void process_chn(uint32 chn) {
	switch(chans[chn].cmd & AICA_CMD_MASK) {
		case AICA_CMD_NONE:
			break;
		case AICA_CMD_START:
			aica_play(chn);
			break;
		case AICA_CMD_STOP:
			aica_stop(chn);
			break;
		case AICA_CMD_UPDATE:
			if (chans[chn].cmd & AICA_UPDATE_GET_POS)
				aica_get_pos(chn);
			if (chans[chn].cmd & AICA_UPDATE_SET_FREQ)
				aica_freq(chn);
			if (chans[chn].cmd & AICA_UPDATE_SET_VOL)
				aica_vol(chn);
			if (chans[chn].cmd & AICA_UPDATE_SET_PAN)
				aica_pan(chn);
			break;
	}
	chans[chn].cmd = 0;
}

int arm_main() {
	int cmdl;

	/* Initialize the AICA part of the SPU */
	aica_init();

	/* Wait for a command */
	while(1) {
		/* Check for a command */
		cmdl = *cmd;
		if (cmdl & AICA_CMD_KICK) {
			int i;

			for (i=0;i<64;i++)
				process_chn(i);
			*cmd = 0;
		}

		/* Little delay to prevent memory lock */
		timer_wait(10);
	}
}
