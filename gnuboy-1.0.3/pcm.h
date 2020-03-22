
#ifndef __PCM_H__
#define __PCM_H__


#include "defs.h"

struct pcm
{
	int hz, len;
	int stereo;
	n16 *buf;
	int pos;
};

extern struct pcm pcm;


#endif


