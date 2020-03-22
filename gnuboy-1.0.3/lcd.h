

#ifndef __LCD_H__
#define __LCD_H__

#include "defs.h"

enum {
	LCD_SCREEN_WIDTH = 160,
	LCD_SIDE_MARGIN = 8,
	LCD_BUF_WIDTH = LCD_SCREEN_WIDTH + LCD_SIDE_MARGIN * 2,
};


struct scan
{
	byte solid_buf[LCD_BUF_WIDTH];
	byte cgb_attr_buf[LCD_BUF_WIDTH]; /* use for bg,window priority */
	un16 pal2[64];
	struct obj *vs[16];
	int ns, l, x, y, s, t, u, v, wx, wy, wt, wv;
};

struct obj
{
	byte y;
	byte x;
	byte pat;
	byte flags;
};

struct lcd
{
	byte vbank[2][8192];
	union
	{
		byte mem[256];
		struct obj obj[40];
	} oam;
	byte pal[128];
};

extern struct lcd lcd;
extern struct scan scan;





#endif



