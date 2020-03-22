#ifndef __ARM_AICA_CMD_IFACE_H
#define __ARM_AICA_CMD_IFACE_H

/* aica_cmd_iface.h,v 1.2 2002/02/15 20:06:57 bardtx Exp */

#ifndef __ARCH_TYPES_H
typedef unsigned long uint32;
#endif

/* Make this 8 dwords long for one aica bus queue */
typedef struct {
	uint32		cmd;		/* Command ID		*/
	uint32		pos;		/* Sample position	*/
	uint32		type;		/* (8/16bit/ADPCM)	*/
	uint32		length;		/* Sample length	*/
	uint32		loop;		/* Sample looping	*/
	uint32		loopstart;	/* Sample loop start	*/
	uint32		loopend;	/* Sample loop end	*/
	uint32		freq;		/* Frequency		*/
	uint32		vol;		/* Volume 0-255		*/
	uint32		pan;		/* Pan 0-255		*/
	uint32		dummy[6];	/* Pad values		*/
} aica_channel;


/* Command values */
#define AICA_CMD_MASK		0x0000000f

#define AICA_CMD_NONE		0x00000000
#define AICA_CMD_START		0x00000001
#define AICA_CMD_STOP		0x00000002
#define AICA_CMD_UPDATE		0x00000003


/* Update values */
#define AICA_UPDATE_MASK	0x000ff000

#define AICA_UPDATE_GET_POS	0x00001000 /* position		*/
#define AICA_UPDATE_SET_FREQ	0x00002000 /* frequency		*/
#define AICA_UPDATE_SET_VOL	0x00004000 /* volume		*/
#define AICA_UPDATE_SET_PAN	0x00008000 /* panning		*/


/* for compatibility, should be removed as soon as possible */
#define AICA_CMD_VOL		(AICA_CMD_UPDATE | AICA_UPDATE_SET_VOL)

/* Sample types */
#define SM_8BIT		1
#define SM_16BIT	0
#define SM_ADPCM	2

/* This is where our AICA variables go... */
/* This is the command center. If this gets set to AICA_CMD_KICK the aica
   will process the commands for all channels*/
#define AICA_CMD		0x010000
/* This is the channel base, which is where we'll poke update info. */
#define AICA_CHANNELS		0x010004
#define AICA_CHANNEL(x)		(AICA_CHANNELS + (x) * sizeof(aica_channel))
#define AICA_RAM_START		0x031000
#define AICA_RAM_END		0x200000

#define AICA_CMD_KICK		0x80000000

#endif	/* __ARM_AICA_CMD_IFACE_H */
