


#include "defs.h"
#include "regs.h"
#include "hw.h"
#include "mem.h"
#include "lcd.h"
#include "rc.h"
#include "fb.h"

struct lcd lcd;

struct scan scan;

#define BUF (vdest)
#define PRI (scan.pri)

#define PAL2 (scan.pal2)

#define VS (scan.vs)
#define NS (scan.ns)

#define L (scan.l) /* line */
#define X (scan.x) /* screen position */
#define Y (scan.y)
#define S (scan.s) /* tilemap position */
#define T (scan.t)
#define U (scan.u) /* position within tile */
#define V (scan.v)
#define WX (scan.wx)
#define WY (scan.wy)
#define WT (scan.wt)
#define WV (scan.wv)

#define DEF_PAL { 0x98d0e0, 0x68a0b0, 0x60707C, 0x2C3C3C }

static int dmg_pal[4][4] = { DEF_PAL, DEF_PAL, DEF_PAL, DEF_PAL };

rcvar_t lcd_exports[] =
{
	RCV_VECTOR("dmg_bgp", dmg_pal[0], 4),
	RCV_VECTOR("dmg_wndp", dmg_pal[1], 4),
	RCV_VECTOR("dmg_obp0", dmg_pal[2], 4),
	RCV_VECTOR("dmg_obp1", dmg_pal[3], 4),
	RCV_END
};

static un16 *vdest;

#define LCD_SCAN_LCDC
#  define LCD_SCAN_NOSPR
#    include "lcd-scan.c"
#  undef LCD_SCAN_NOSPR
#    include "lcd-scan.c"
#undef LCD_SCAN_LCDC
#  define LCD_SCAN_NOSPR
#    include "lcd-scan.c"
#  undef LCD_SCAN_NOSPR
#    include "lcd-scan.c"


void spr_enum()
{
	int i;
        int top = L + 16;
	int bottom = L + ((R_LCDC & 0x04) ? 0 : 8);
	struct obj *o;
	
	NS = 0;
	if (!(R_LCDC & 0x02)) return;
	
	o = lcd.oam.obj;
	
	for (i = 40; i; i--, o++)
	{
		if ((top < o->y) | (bottom >= o->y))
			continue;
		
                VS[NS] = o;
		if (++NS == 10) break;
	}
	
	if (hw.cgb) return;
	
        {
		/* sort sprites */
		int i, j;
		struct obj *w;
		struct obj guard;
		
		VS[NS] = &guard;
		
		i = NS - 1;
		for (i = NS - 1; i >= 0; --i)
		{
			w = VS[i]; 
			guard.x = w->x;
			j = i + 1;
			while (w->x > VS[j]->x) 
			{
				VS[j - 1] = VS[j];
				++j;
			}
			VS[j - 1] = w;
		}
        }
}


#define LCD_SPR_SCAN_16
#include "lcd-spr.c"
#undef LCD_SPR_SCAN_16
#include "lcd-spr.c"


#if 1
void
lcd_display_pattern_helper (un16 *dest, byte *vram)
{
  int i;
  int x, y;
  int tilenum;
  byte t1, t2;
  byte p1, p2;
  un16 *d1, *p;
  un16 *pal = PAL2 + 0;
  
  tilenum = 0;
  for (i = 0; i < 8; ++i)
  {
    dest += fb.pitch * 8;
    d1 = dest;
    
    for (x = 0; x < 32; ++x) 
    {
      d1 += 8;
      
      for (y = 0; y < 8; ++y) 
      {
        p = d1 + fb.pitch * y;
        
        t1 = vram[tilenum * 16 + y * 2 + 0];
        t2 = vram[tilenum * 16 + y * 2 + 1]; 
        
        p1 = (t1 & 0xaa) | ((t2 >> 1) & 0x55);
        p2 = ((t1 << 1) & 0xaa) | (t2 & 0x55);
        
        *(--p) = pal[(p2 & 0x03)]; /* 7 */
        *(--p) = pal[(p1 & 0x03)]; /* 6 */
        p2 >>= 2;
        p1 >>= 2;
        
        *(--p) = pal[(p2 & 0x03)]; /* 5 */
        *(--p) = pal[(p1 & 0x03)]; /* 4 */
        p2 >>= 2;
        p1 >>= 2;
        
        *(--p) = pal[(p2 & 0x03)]; /* 3 */
        *(--p) = pal[(p1 & 0x03)]; /* 2 */
        p2 >>= 2;
        p1 >>= 2;
        
        *(--p) = pal[(p2 & 0x03)]; /* 1 */
        *(--p) = pal[(p1 & 0x03)]; /* 0 */
        p2 >>= 2;
        p1 >>= 2;
      }
      ++tilenum;
    }
  }
}


void
lcd_display_pattern (un16 *dest)
{
  lcd_display_pattern_helper (dest, lcd.vbank[0]);
  dest += fb.pitch * 8 * 10;
  lcd_display_pattern_helper (dest, lcd.vbank[0] + 0x800);
  dest += fb.pitch * 8 * 10;
  lcd_display_pattern_helper (dest, lcd.vbank[1]);
  dest += fb.pitch * 8 * 10;
  lcd_display_pattern_helper (dest, lcd.vbank[1] + 0x800);
}
#endif


void lcd_begin()
{
	vdest = fb.ptr;
	WY = R_WY;
}

void lcd_refreshline()
{
	if (!fb.enabled) return;
	
	if (!(R_LCDC & 0x80))
		return; /* should not happen... */
	
	L = R_LY;
	X = R_SCX;
	Y = (R_SCY + L) & 0xff;
	S = X >> 3;
	T = Y >> 3;
	U = X & 7;
	V = Y & 7;
	
	WX = R_WX - 7;
	if (WY>L || WY<0 || WY>143 || WX<-7 || WX>159 || !(R_LCDC&0x20))
		WX = 160;
	WT = (L - WY) >> 3;
	WV = (L - WY) & 7;
	
        /* need call spr_enum() before (bg|wnd)_scan_color_* 
           to decide to skip sprites setup in (bg|wnd)_scan_color_* */
        spr_enum ();
        
	if (hw.cgb)
	{
		if (!NS) 
		{
			if (R_LCDC & 0x10) 
			{
				bg_scan_color_lcdc1_nospr();
				wnd_scan_color_lcdc1_nospr();
			}
			else
			{
				bg_scan_color_lcdc0_nospr();
				wnd_scan_color_lcdc0_nospr();
			}
		}
		else
		{
			if (R_LCDC & 0x10) 
			{
				bg_scan_color_lcdc1();
				wnd_scan_color_lcdc1();
			}
			else
			{
				bg_scan_color_lcdc0();
				wnd_scan_color_lcdc0();
			}
			if (R_LCDC & 0x04) 
				spr_scan_color_16();
			else
				spr_scan_color_8();
		}
	}
	else
	{
		if (!NS)
		{
			if (R_LCDC & 0x10) 
			{
				bg_scan_lcdc1_nospr();
				wnd_scan_lcdc1_nospr();
			}
			else
			{
				bg_scan_lcdc0_nospr();
				wnd_scan_lcdc0_nospr();
			}
		}
		else
		{
			if (R_LCDC & 0x10) 
			{
				bg_scan_lcdc1();
				wnd_scan_lcdc1();
			}
			else
			{
				bg_scan_lcdc0();
				wnd_scan_lcdc0();
			}
			
			if (R_LCDC & 0x04) 
				spr_scan_16();
			else
				spr_scan_8();
		}
	}
	
	vdest += fb.pitch;
}


static void updatepalette(int i)
{
	int c, r, g, b;
        
	c = (lcd.pal[i<<1] | ((int)lcd.pal[(i<<1)|1] << 8)) & 0x7FFF;
	r = (c & 0x001F) << 3;
	g = (c & 0x03E0) >> 2;
	b = (c & 0x7C00) >> 7;
	r |= (r >> 5);
	g |= (g >> 5);
	b |= (b >> 5);
        
	r = (r >> fb.cc[0].r) << fb.cc[0].l;
	g = (g >> fb.cc[1].r) << fb.cc[1].l;
	b = (b >> fb.cc[2].r) << fb.cc[2].l;
	c = r|g|b;
	
	PAL2[i] = c;
}

void pal_write(int i, byte b)
{
	if (lcd.pal[i] == b) return;
	lcd.pal[i] = b;
	updatepalette(i>>1);
}

void pal_write_dmg(int i, int mapnum, byte d)
{
	int j;
	int *cmap = dmg_pal[mapnum];
	int c, r, g, b;

	if (hw.cgb) return;

	/* if (mapnum >= 2) d = 0xe4; */
	for (j = 0; j < 8; j += 2)
	{
		c = cmap[(d >> j) & 3];
		r = (c & 0xf8) >> 3;
		g = (c & 0xf800) >> 6;
		b = (c & 0xf80000) >> 9;
		c = r|g|b;
		/* FIXME - handle directly without faking cgb */
		pal_write(i+j, c & 0xff);
		pal_write(i+j+1, c >> 8);
	}
}

void vram_write(int a, byte b)
{
	lcd.vbank[R_VBK&1][a] = b;
}

void pal_dirty()
{
	int i;
	if (!hw.cgb)
	{
		pal_write_dmg(0, 0, R_BGP);
		pal_write_dmg(8, 1, R_BGP);
		pal_write_dmg(64, 2, R_OBP0);
		pal_write_dmg(72, 3, R_OBP1);
	}
	for (i = 0; i < 64; i++)
		updatepalette(i);
}

void lcd_reset()
{
	memset(&lcd, 0, sizeof lcd);
	lcd_begin();
	pal_dirty();
}
















