/* KallistiOS ##version##

   biosfont.c

   (c)2000-2001 Dan Potter
 */

#include <dc/biosfont.h>

CVSID("biosfont.c,v 1.3 2002/01/06 01:14:47 bardtx Exp");

/*

This module handles interfacing to the BIOS to get its "thin" font,
which is used in the BIOS menus. Among other useful properties (being
there without being loaded! =) this font includes Japanese characters.

Thanks to Marcus Comstedt for this information.

*/


/* A little assembly that grabs the font address */
extern void* get_font_address();
asm(
"	.text\n"
"	.align 2\n"
"_get_font_address:\n"
"	mov.l	syscall_b4,r0\n"
"	mov.l	@r0,r0\n"
"	jmp	@r0\n"
"	mov	#0,r1\n"
"\n"	
"	.align 4\n"
"syscall_b4:\n"
"	.long	0x8c0000b4\n"
);


/* Given a character, find it in the BIOS font if possible */
void *bfont_find_char(int ch) {
	int index = -1;
	void *fa = get_font_address();
	
	/* 33-126 in ASCII are 1-94 in the font */
	if (ch >= 33 && ch <= 126)
		index = ch - 32;
	
	/* 160-255 in ASCII are 96-161 in the font */
	if (ch >= 160 && ch <= 255)
		index = ch - (160 - 96);
	
	/* Map anything else to a space */
	if (index == -1)
		index = 72 << 2;

	return fa + index*36;
}

/* Given a character, draw it into a buffer */
void bfont_draw(uint16 *buffer, int bufwidth, int opaque, int c) {
	uint8 *ch = (uint8*)bfont_find_char(c);
	uint16 word;
	int x, y;

	for (y=0; y<24; ) {
		/* Do the first row */
		word = (((uint16)ch[0]) << 4) | ((ch[1] >> 4) & 0x0f);
		for (x=0; x<12; x++) {
			if (word & (0x0800 >> x))
				*buffer = 0xffff;
			else {
				if (opaque)
					*buffer = 0x0000;
			}
			buffer++;
		}
		buffer += bufwidth - 12;
		y++;
		
		/* Do the second row */
		word = ( (((uint16)ch[1]) << 8) & 0xf00) | ch[2];
		for (x=0; x<12; x++) {
			if (word & (0x0800 >> x))
				*buffer = 0xffff;
			else {
				if (opaque)
					*buffer = 0x0000;
			}
			buffer++;
		}
		buffer += bufwidth - 12;
		y++;
		
		ch += 3;
	}
}

void bfont_draw_str(uint16 *buffer, int width, int opaque, char *str) {
	while (*str)
		bfont_draw(buffer += 12, width, opaque, *str++);
}









