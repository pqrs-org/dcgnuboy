

#ifndef __SOUND_H__
#define __SOUND_H__


struct sndchan
{
	int on;
	unsigned pos;
	int cnt, encnt, swcnt;
	int len, enlen, swlen;
	int swfreq;
	int freq;
	int envol, endir;
};


struct snd
{
	int rate;
	struct sndchan ch[4];
	byte wave[16];
};


typedef struct 
{
	un32 timestamp;
	byte address;
	byte value;
} snddata_t;


/* SNDQUEUE_SIZE must be 2^x */
#define SNDQUEUE_SIZE 4096

typedef struct
{
	snddata_t q[SNDQUEUE_SIZE];
	int head, tail;
} sndqueue_t;


extern struct snd snd;


#endif


