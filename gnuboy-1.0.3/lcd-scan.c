#ifndef _LCD_SCAN_C_
#define _LCD_SCAN_C_

/* ============================================================ */
inline void
bg_scan_render (un16 *dest, byte *sbuf, un16 *pal, byte t1, byte t2)
{
  byte p1, p2;
  
  p1 = (t1 & 0xaa) | ((t2 >> 1) & 0x55);
  p2 = ((t1 << 1) & 0xaa) | (t2 & 0x55);
  
  *(--dest) = pal[*(--sbuf) = (p2 & 0x03)]; /* 7 */
  *(--dest) = pal[*(--sbuf) = (p1 & 0x03)]; /* 6 */
  p2 >>= 2;
  p1 >>= 2;
  
  *(--dest) = pal[*(--sbuf) = (p2 & 0x03)]; /* 5 */
  *(--dest) = pal[*(--sbuf) = (p1 & 0x03)]; /* 4 */
  p2 >>= 2;
  p1 >>= 2;
  
  *(--dest) = pal[*(--sbuf) = (p2 & 0x03)]; /* 3 */
  *(--dest) = pal[*(--sbuf) = (p1 & 0x03)]; /* 2 */
  p2 >>= 2;
  p1 >>= 2;
  
  *(--dest) = pal[*(--sbuf) = (p2 & 0x03)]; /* 1 */
  *(--dest) = pal[*(--sbuf) = (p1 & 0x03)]; /* 0 */
  p2 >>= 2;
  p1 >>= 2;
}


inline void
wnd_scan_render (un16 *dest, byte *sbuf, un16 *pal, byte t1, byte t2)
{
  byte p1, p2;
  
  p1 = (t1 & 0xaa) | ((t2 >> 1) & 0x55);
  p2 = ((t1 << 1) & 0xaa) | (t2 & 0x55);
  
  *(--sbuf) |= (p2 & 0x03);
  *(--dest) = pal[p2 & 0x03]; /* 7 */
  *(--sbuf) |= (p1 & 0x03);
  *(--dest) = pal[p1 & 0x03]; /* 6 */
  p2 >>= 2;
  p1 >>= 2;
  
  *(--sbuf) |= (p2 & 0x03);
  *(--dest) = pal[p2 & 0x03]; /* 5 */
  *(--sbuf) |= (p1 & 0x03);
  *(--dest) = pal[p1 & 0x03]; /* 4 */
  p2 >>= 2;
  p1 >>= 2;
  
  *(--sbuf) |= (p2 & 0x03);
  *(--dest) = pal[p2 & 0x03]; /* 3 */
  *(--sbuf) |= (p1 & 0x03);
  *(--dest) = pal[p1 & 0x03]; /* 2 */
  p2 >>= 2;
  p1 >>= 2;
  
  *(--sbuf) |= (p2 & 0x03);
  *(--dest) = pal[p2 & 0x03]; /* 1 */
  *(--sbuf) |= (p1 & 0x03);
  *(--dest) = pal[p1 & 0x03]; /* 0 */
  p2 >>= 2;
  p1 >>= 2;
}


inline void
bg_wnd_scan_render_nospr (un16 *dest, un16 *pal, byte t1, byte t2)
{
  byte p1, p2;
  
  p1 = (t1 & 0xaa) | ((t2 >> 1) & 0x55);
  p2 = ((t1 << 1) & 0xaa) | (t2 & 0x55);
  
  *(--dest) = pal[(p2 & 0x03)]; /* 7 */
  *(--dest) = pal[(p1 & 0x03)]; /* 6 */
  p2 >>= 2;
  p1 >>= 2;
  
  *(--dest) = pal[(p2 & 0x03)]; /* 5 */
  *(--dest) = pal[(p1 & 0x03)]; /* 4 */
  p2 >>= 2;
  p1 >>= 2;
  
  *(--dest) = pal[(p2 & 0x03)]; /* 3 */
  *(--dest) = pal[(p1 & 0x03)]; /* 2 */
  p2 >>= 2;
  p1 >>= 2;
  
  *(--dest) = pal[(p2 & 0x03)]; /* 1 */
  *(--dest) = pal[(p1 & 0x03)]; /* 0 */
  p2 >>= 2;
  p1 >>= 2;
}


/* ------------------------------------------------------------ */
inline void
bg_scan_color_render (un16 *dest, byte *sbuf, un16 *pal, byte t1, byte t2, byte attr)
{
  byte p1, p2;
  
  p1 = (t1 & 0xaa) | ((t2 >> 1) & 0x55);
  p2 = ((t1 << 1) & 0xaa) | (t2 & 0x55);
  
  pal += (attr & 0x07) << 2;
  
  if (attr & 0x20)
  {
    /* horizontal mirroring */
    *dest++ = pal[*sbuf++ = (p2 & 0x03)]; /* 7 */
    *dest++ = pal[*sbuf++ = (p1 & 0x03)]; /* 6 */
    p2 >>= 2;
    p1 >>= 2;
    
    *dest++ = pal[*sbuf++ = (p2 & 0x03)]; /* 5 */
    *dest++ = pal[*sbuf++ = (p1 & 0x03)]; /* 4 */
    p2 >>= 2;
    p1 >>= 2;
    
    *dest++ = pal[*sbuf++ = (p2 & 0x03)]; /* 3 */
    *dest++ = pal[*sbuf++ = (p1 & 0x03)]; /* 2 */
    p2 >>= 2;
    p1 >>= 2;
    
    *dest++ = pal[*sbuf++ = (p2 & 0x03)]; /* 1 */
    *dest++ = pal[*sbuf++ = (p1 & 0x03)]; /* 0 */
    p2 >>= 2;
    p1 >>= 2;
  }
  else
  {
    dest += 8;
    sbuf += 8;
    
    *(--dest) = pal[*(--sbuf) = (p2 & 0x03)]; /* 7 */
    *(--dest) = pal[*(--sbuf) = (p1 & 0x03)]; /* 6 */
    p2 >>= 2;
    p1 >>= 2;
    
    *(--dest) = pal[*(--sbuf) = (p2 & 0x03)]; /* 5 */
    *(--dest) = pal[*(--sbuf) = (p1 & 0x03)]; /* 4 */
    p2 >>= 2;
    p1 >>= 2;
    
    *(--dest) = pal[*(--sbuf) = (p2 & 0x03)]; /* 3 */
    *(--dest) = pal[*(--sbuf) = (p1 & 0x03)]; /* 2 */
    p2 >>= 2;
    p1 >>= 2;
    
    *(--dest) = pal[*(--sbuf) = (p2 & 0x03)]; /* 1 */
    *(--dest) = pal[*(--sbuf) = (p1 & 0x03)]; /* 0 */
    p2 >>= 2;
    p1 >>= 2;
  }
}


inline void
wnd_scan_color_render (un16 *dest, byte *sbuf, un16 *pal, byte t1, byte t2, byte attr)
{
  byte p1, p2;
  
  p1 = (t1 & 0xaa) | ((t2 >> 1) & 0x55);
  p2 = ((t1 << 1) & 0xaa) | (t2 & 0x55);
  
  pal += (attr & 0x07) << 2;
  
  if (attr & 0x20)
  {
    /* horizontal mirroring */
    *sbuf++ |= p2 & 0x03;
    *dest++ = pal[p2 & 0x03]; /* 7 */
    *sbuf++ |= p1 & 0x03;
    *dest++ = pal[p1 & 0x03]; /* 6 */
    p2 >>= 2;
    p1 >>= 2;
    
    *sbuf++ |= p2 & 0x03;
    *dest++ = pal[p2 & 0x03]; /* 5 */
    *sbuf++ |= p1 & 0x03;
    *dest++ = pal[p1 & 0x03]; /* 4 */
    p2 >>= 2;
    p1 >>= 2;
    
    *sbuf++ |= p2 & 0x03;
    *dest++ = pal[p2 & 0x03]; /* 3 */
    *sbuf++ |= p1 & 0x03;
    *dest++ = pal[p1 & 0x03]; /* 2 */
    p2 >>= 2;
    p1 >>= 2;
    
    *sbuf++ |= p2 & 0x03;
    *dest++ = pal[p2 & 0x03]; /* 1 */
    *sbuf++ |= p1 & 0x03;
    *dest++ = pal[p1 & 0x03]; /* 0 */
    p2 >>= 2;
    p1 >>= 2;
  }
  else
  {
    dest += 8;
    sbuf += 8;
    
    *(--sbuf) |= p2 & 0x03;
    *(--dest) = pal[p2 & 0x03]; /* 7 */
    *(--sbuf) |= p1 & 0x03;
    *(--dest) = pal[p1 & 0x03]; /* 6 */
    p2 >>= 2;
    p1 >>= 2;
    
    *(--sbuf) |= p2 & 0x03;
    *(--dest) = pal[p2 & 0x03]; /* 5 */
    *(--sbuf) |= p1 & 0x03;
    *(--dest) = pal[p1 & 0x03]; /* 4 */
    p2 >>= 2;
    p1 >>= 2;
    
    *(--sbuf) |= p2 & 0x03;
    *(--dest) = pal[p2 & 0x03]; /* 3 */
    *(--sbuf) |= p1 & 0x03;
    *(--dest) = pal[p1 & 0x03]; /* 2 */
    p2 >>= 2;
    p1 >>= 2;
    
    *(--sbuf) |= p2 & 0x03;
    *(--dest) = pal[p2 & 0x03]; /* 1 */
    *(--sbuf) |= p1 & 0x03;
    *(--dest) = pal[p1 & 0x03]; /* 0 */
    p2 >>= 2;
    p1 >>= 2;
  }
}


inline void
bg_wnd_scan_color_render_nospr (un16 *dest, un16 *pal, byte t1, byte t2, byte attr)
{
  byte p1, p2;
  
  p1 = (t1 & 0xaa) | ((t2 >> 1) & 0x55);
  p2 = ((t1 << 1) & 0xaa) | (t2 & 0x55);
  
  pal += (attr & 0x07) << 2;
  
  if (attr & 0x20)
  {
    /* horizontal mirroring */
    *dest++ = pal[(p2 & 0x03)]; /* 7 */
    *dest++ = pal[(p1 & 0x03)]; /* 6 */
    p2 >>= 2;
    p1 >>= 2;
    
    *dest++ = pal[(p2 & 0x03)]; /* 5 */
    *dest++ = pal[(p1 & 0x03)]; /* 4 */
    p2 >>= 2;
    p1 >>= 2;
    
    *dest++ = pal[(p2 & 0x03)]; /* 3 */
    *dest++ = pal[(p1 & 0x03)]; /* 2 */
    p2 >>= 2;
    p1 >>= 2;
    
    *dest++ = pal[(p2 & 0x03)]; /* 1 */
    *dest++ = pal[(p1 & 0x03)]; /* 0 */
    p2 >>= 2;
    p1 >>= 2;
  }
  else
  {
    dest += 8;
    
    *(--dest) = pal[(p2 & 0x03)]; /* 7 */
    *(--dest) = pal[(p1 & 0x03)]; /* 6 */
    p2 >>= 2;
    p1 >>= 2;
    
    *(--dest) = pal[(p2 & 0x03)]; /* 5 */
    *(--dest) = pal[(p1 & 0x03)]; /* 4 */
    p2 >>= 2;
    p1 >>= 2;
    
    *(--dest) = pal[(p2 & 0x03)]; /* 3 */
    *(--dest) = pal[(p1 & 0x03)]; /* 2 */
    p2 >>= 2;
    p1 >>= 2;
    
    *(--dest) = pal[(p2 & 0x03)]; /* 1 */
    *(--dest) = pal[(p1 & 0x03)]; /* 0 */
    p2 >>= 2;
    p1 >>= 2;
  }
}


#endif


/* ============================================================ */
void
#ifdef LCD_SCAN_LCDC
#  ifdef LCD_SCAN_NOSPR
bg_scan_lcdc1_nospr(void)
#  else
bg_scan_lcdc1(void)
#  endif
#else
#  ifdef LCD_SCAN_NOSPR
bg_scan_lcdc0_nospr(void)
#  else
bg_scan_lcdc0(void)
#  endif
#endif
{
  un16 *dest = BUF + LCD_SIDE_MARGIN - U;
#ifndef LCD_SCAN_NOSPR
  byte *sbuf = scan.solid_buf + LCD_SIDE_MARGIN - U;
#endif
  byte *vram = lcd.vbank[0];
  int cnt;
  int rest_cnt;
  byte t1, t2;
  byte *tile;
  int tile_y = V << 1;
  
  if (WX <= 0)
    return;
  
  tile = lcd.vbank[0] + ((R_LCDC & 0x08) ? 0x1C00 : 0x1800) + (T << 5) + S;
  
  rest_cnt = ((WX + U) >> 3) + 1;
  if (rest_cnt > 21) rest_cnt = 21;
  
  cnt = 32 - S;
  if (cnt < rest_cnt) 
    rest_cnt -= cnt;
  else
  {
    cnt = rest_cnt;
    rest_cnt = 0;
  }
  
  while (cnt--)
  {
#ifdef LCD_SCAN_LCDC
    t2 = vram[*tile * 16 + tile_y + 0];
    t1 = vram[*tile * 16 + tile_y + 1];
#else
    t2 = vram[(256 + (n8)(*tile)) * 16 + tile_y + 0];
    t1 = vram[(256 + (n8)(*tile)) * 16 + tile_y + 1];
#endif
    ++tile;
    
    dest += 8;
#ifdef LCD_SCAN_NOSPR
    bg_wnd_scan_render_nospr (dest, PAL2, t1, t2);
#else
    sbuf += 8;
    bg_scan_render (dest, sbuf, PAL2, t1, t2);
#endif
  }
  
  tile -= 32;
  
  while (rest_cnt--)
  {
#ifdef LCD_SCAN_LCDC
    t2 = vram[*tile * 16 + tile_y + 0];
    t1 = vram[*tile * 16 + tile_y + 1];
#else
    t2 = vram[(256 + (n8)(*tile)) * 16 + tile_y + 0];
    t1 = vram[(256 + (n8)(*tile)) * 16 + tile_y + 1];
#endif
    ++tile;
    
    dest += 8;
#ifdef LCD_SCAN_NOSPR
    bg_wnd_scan_render_nospr (dest, PAL2, t1, t2);
#else
    sbuf += 8;
    bg_scan_render (dest, sbuf, PAL2, t1, t2);
#endif
  }
}


void
#ifdef LCD_SCAN_LCDC
#  ifdef LCD_SCAN_NOSPR
wnd_scan_lcdc1_nospr(void)
#  else
wnd_scan_lcdc1(void)
#  endif
#else
#  ifdef LCD_SCAN_NOSPR
wnd_scan_lcdc0_nospr(void)
#  else 
wnd_scan_lcdc0(void)
#  endif
#endif
{
  int cnt;
  byte *vram = lcd.vbank[0];
  un16 *dest = BUF + LCD_SIDE_MARGIN + WX;
#ifndef LCD_SCAN_NOSPR
  byte *sbuf = scan.solid_buf + LCD_SIDE_MARGIN + WX;
#endif
  byte *tile;
  byte t1, t2;
  int tile_y = WV << 1;
  
  if (WX >= 160) return;
  tile = lcd.vbank[0] + ((R_LCDC & 0x40) ? 0x1C00 : 0x1800) + (WT << 5);
  
  cnt = ((160 - WX) >> 3) + 1;
  
  while (cnt--)
  {
#ifdef LCD_SCAN_LCDC
    t2 = vram[*tile * 16 + tile_y + 0];
    t1 = vram[*tile * 16 + tile_y + 1];
#else
    t2 = vram[(256 + (n8)(*tile)) * 16 + tile_y + 0];
    t1 = vram[(256 + (n8)(*tile)) * 16 + tile_y + 1];
#endif
    ++tile;
    
    dest += 8;
#ifdef LCD_SCAN_NOSPR
    bg_wnd_scan_render_nospr (dest, PAL2, t1, t2);
#else
    sbuf += 8;
    wnd_scan_render (dest, sbuf, PAL2, t1, t2);
#endif
  }
}


/* ============================================================ */
void
#ifdef LCD_SCAN_LCDC
#  ifdef LCD_SCAN_NOSPR
bg_scan_color_lcdc1_nospr(void)
#  else
bg_scan_color_lcdc1(void)
#  endif
#else
#  ifdef LCD_SCAN_NOSPR
bg_scan_color_lcdc0_nospr(void)
#  else
bg_scan_color_lcdc0(void)
#  endif
#endif
{
  un16 *dest = BUF + LCD_SIDE_MARGIN - U;
#ifndef LCD_SCAN_NOSPR
  byte *sbuf = scan.solid_buf + LCD_SIDE_MARGIN - U;
  byte *abuf = scan.cgb_attr_buf + LCD_SIDE_MARGIN - U;
#endif
  byte *vram;
  int cnt;
  int rest_cnt;
  byte t1, t2;
  int base;
  byte *tile;
  byte *attr;
  int tile_y;
  
  if (WX <= 0)
    return;
  
  base = ((R_LCDC & 0x08) ? 0x1C00 : 0x1800) + (T << 5) + S;
  tile = lcd.vbank[0] + base;
  attr = lcd.vbank[1] + base;
  
  rest_cnt = ((WX + U) >> 3) + 1;
  if (rest_cnt > 21) rest_cnt = 21;
  
  cnt = 32 - S;
  if (cnt < rest_cnt) 
    rest_cnt -= cnt;
  else
  {
    cnt = rest_cnt;
    rest_cnt = 0;
  }
  
  while (cnt--)
  {
    vram = lcd.vbank[(*attr & 0x08) >> 3];
    
    if (*attr & 0x40)
      tile_y = (7 - V) << 1;
    else
      tile_y = V << 1;
    
#ifdef LCD_SCAN_LCDC
    t2 = vram[*tile * 16 + tile_y + 0];
    t1 = vram[*tile * 16 + tile_y + 1];
#else
    t2 = vram[(256 + (n8)(*tile)) * 16 + tile_y + 0];
    t1 = vram[(256 + (n8)(*tile)) * 16 + tile_y + 1];
#endif
    ++tile;

#ifdef LCD_SCAN_NOSPR    
    bg_wnd_scan_color_render_nospr (dest, PAL2, t1, t2, *attr);
#else
    bg_scan_color_render (dest, sbuf, PAL2, t1, t2, *attr);
    
    *abuf++ = (*attr & 0x80); *abuf++ = (*attr & 0x80);
    *abuf++ = (*attr & 0x80); *abuf++ = (*attr & 0x80);
    *abuf++ = (*attr & 0x80); *abuf++ = (*attr & 0x80);
    *abuf++ = (*attr & 0x80); *abuf++ = (*attr & 0x80);
    
    sbuf += 8;
#endif
    dest += 8;
    ++attr;
  }
  
  tile -= 32;
  attr -= 32;
  
  while (rest_cnt--)
  {
    vram = lcd.vbank[(*attr & 0x08) >> 3];
    
    if (*attr & 0x40)
      tile_y = (7 - V) << 1;
    else
      tile_y = V << 1;
    
#ifdef LCD_SCAN_LCDC
    t2 = vram[*tile * 16 + tile_y + 0];
    t1 = vram[*tile * 16 + tile_y + 1];
#else
    t2 = vram[(256 + (n8)(*tile)) * 16 + tile_y + 0];
    t1 = vram[(256 + (n8)(*tile)) * 16 + tile_y + 1];
#endif
    ++tile;
    
#ifdef LCD_SCAN_NOSPR    
    bg_wnd_scan_color_render_nospr (dest, PAL2, t1, t2, *attr);
#else
    bg_scan_color_render (dest, sbuf, PAL2, t1, t2, *attr);
    
    *abuf++ = (*attr & 0x80); *abuf++ = (*attr & 0x80);
    *abuf++ = (*attr & 0x80); *abuf++ = (*attr & 0x80);
    *abuf++ = (*attr & 0x80); *abuf++ = (*attr & 0x80);
    *abuf++ = (*attr & 0x80); *abuf++ = (*attr & 0x80);
    
    sbuf += 8;
#endif
    dest += 8;
    ++attr;
  }
}


void
#ifdef LCD_SCAN_LCDC
#  ifdef LCD_SCAN_NOSPR
wnd_scan_color_lcdc1_nospr(void)
#  else
wnd_scan_color_lcdc1(void)
#  endif
#else
#  ifdef LCD_SCAN_NOSPR
wnd_scan_color_lcdc0_nospr(void)
#  else
wnd_scan_color_lcdc0(void)
#  endif
#endif
{
  int cnt;
  byte *vram;
  un16 *dest = BUF + LCD_SIDE_MARGIN + WX;
#ifndef LCD_SCAN_NOSPR
  byte *sbuf = scan.solid_buf + LCD_SIDE_MARGIN + WX;
  byte *abuf = scan.cgb_attr_buf + LCD_SIDE_MARGIN + WX;
#endif
  int base;
  byte *tile;
  byte *attr;
  byte t1, t2;
  int tile_y = WV << 1;
  
  if (WX >= 160) return;
  base = ((R_LCDC & 0x40) ? 0x1C00 : 0x1800) + (WT << 5);
  tile = lcd.vbank[0] + base;
  attr = lcd.vbank[1] + base;
  
  cnt = ((160 - WX) >> 3) + 1;
  
  while (cnt--)
  {
    vram = lcd.vbank[(*attr & 0x08) >> 3];
    
    if (*attr & 0x40)
      tile_y = (7 - WV) << 1;
    else
      tile_y = WV << 1;
    
#ifdef LCD_SCAN_LCDC
    t2 = vram[*tile * 16 + tile_y + 0];
    t1 = vram[*tile * 16 + tile_y + 1];
#else
    t2 = vram[(256 + (n8)(*tile)) * 16 + tile_y + 0];
    t1 = vram[(256 + (n8)(*tile)) * 16 + tile_y + 1];
#endif
    ++tile;
    
#ifdef LCD_SCAN_NOSPR
    bg_wnd_scan_color_render_nospr (dest, PAL2, t1, t2, *attr);
#else
    wnd_scan_color_render (dest, sbuf, PAL2, t1, t2, *attr);
    
    *abuf++ = (*attr & 0x80); *abuf++ = (*attr & 0x80);
    *abuf++ = (*attr & 0x80); *abuf++ = (*attr & 0x80);
    *abuf++ = (*attr & 0x80); *abuf++ = (*attr & 0x80);
    *abuf++ = (*attr & 0x80); *abuf++ = (*attr & 0x80);
    
    sbuf += 8;
#endif
    dest += 8;
    ++attr;
  }
}


