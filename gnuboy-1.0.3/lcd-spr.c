#ifndef _LCD_SPR_C_
#define _LCD_SPR_C_

/* ------------------------------------------------------------ */
inline void
spr_scan_render (un16 *dest, struct obj *vs, un16 *pal, byte t1, byte t2)
{
  byte p1, p2;
  
  p1 = (t1 & 0xaa) | ((t2 >> 1) & 0x55);
  p2 = ((t1 << 1) & 0xaa) | (t2 & 0x55);
  
  if (vs->flags & 0x80) 
  {
    /* lesser priority */
    if (vs->flags & 0x20) 
    {
      byte *sbuf = scan.solid_buf + LCD_SIDE_MARGIN + (vs->x - 8);
      
      /* horizontal mirroring */
      if (*sbuf++ | !(p2 & 0x03)) ++dest; else *dest++ = pal[p2 & 0x03]; /* 7 */
      if (*sbuf++ | !(p1 & 0x03)) ++dest; else *dest++ = pal[p1 & 0x03]; /* 6 */
      p2 >>= 2;
      p1 >>= 2;
      
      if (*sbuf++ | !(p2 & 0x03)) ++dest; else *dest++ = pal[p2 & 0x03]; /* 5 */
      if (*sbuf++ | !(p1 & 0x03)) ++dest; else *dest++ = pal[p1 & 0x03]; /* 4 */
      p2 >>= 2;
      p1 >>= 2;
      
      if (*sbuf++ | !(p2 & 0x03)) ++dest; else *dest++ = pal[p2 & 0x03]; /* 3 */
      if (*sbuf++ | !(p1 & 0x03)) ++dest; else *dest++ = pal[p1 & 0x03]; /* 2 */
      p2 >>= 2;
      p1 >>= 2;
      
      if (*sbuf++ | !(p2 & 0x03)) ++dest; else *dest++ = pal[p2 & 0x03]; /* 1 */
      if (*sbuf++ | !(p1 & 0x03)) ++dest; else *dest++ = pal[p1 & 0x03]; /* 0 */
      p2 >>= 2;
      p1 >>= 2;
    }
    else 
    {
      /* byte *sbuf = scan.solid_buf + LCD_SIDE_MARGIN + (vs->x - 8) + 8 ; */
      byte *sbuf = scan.solid_buf + LCD_SIDE_MARGIN + vs->x;
      
      dest += 8;
      
      if (*(--sbuf) | !(p2 & 0x03)) --dest; else *(--dest) = pal[p2 & 0x03]; /* 7 */
      if (*(--sbuf) | !(p1 & 0x03)) --dest; else *(--dest) = pal[p1 & 0x03]; /* 6 */
      p2 >>= 2;
      p1 >>= 2;
      
      if (*(--sbuf) | !(p2 & 0x03)) --dest; else *(--dest) = pal[p2 & 0x03]; /* 5 */
      if (*(--sbuf) | !(p1 & 0x03)) --dest; else *(--dest) = pal[p1 & 0x03]; /* 4 */
      p2 >>= 2;
      p1 >>= 2;
      
      if (*(--sbuf) | !(p2 & 0x03)) --dest; else *(--dest) = pal[p2 & 0x03]; /* 3 */
      if (*(--sbuf) | !(p1 & 0x03)) --dest; else *(--dest) = pal[p1 & 0x03]; /* 2 */
      p2 >>= 2;
      p1 >>= 2;
      
      if (*(--sbuf) | !(p2 & 0x03)) --dest; else *(--dest) = pal[p2 & 0x03]; /* 1 */
      if (*(--sbuf) | !(p1 & 0x03)) --dest; else *(--dest) = pal[p1 & 0x03]; /* 0 */
      p2 >>= 2;
      p1 >>= 2;
    }
  }
  else
  {
    if (vs->flags & 0x20) 
    {
      /* horizontal mirroring */
      if (!(p2 & 0x03)) ++dest; else *dest++ = pal[p2 & 0x03]; /* 7 */
      if (!(p1 & 0x03)) ++dest; else *dest++ = pal[p1 & 0x03]; /* 6 */
      p2 >>= 2;
      p1 >>= 2;
      
      if (!(p2 & 0x03)) ++dest; else *dest++ = pal[p2 & 0x03]; /* 5 */
      if (!(p1 & 0x03)) ++dest; else *dest++ = pal[p1 & 0x03]; /* 4 */
      p2 >>= 2;
      p1 >>= 2;
      
      if (!(p2 & 0x03)) ++dest; else *dest++ = pal[p2 & 0x03]; /* 3 */
      if (!(p1 & 0x03)) ++dest; else *dest++ = pal[p1 & 0x03]; /* 2 */
      p2 >>= 2;
      p1 >>= 2;
      
      if (!(p2 & 0x03)) ++dest; else *dest++ = pal[p2 & 0x03]; /* 1 */
      if (!(p1 & 0x03)) ++dest; else *dest++ = pal[p1 & 0x03]; /* 0 */
      p2 >>= 2;
      p1 >>= 2;
    }
    else 
    {
      dest += 8;
      
      if (!(p2 & 0x03)) --dest; else *(--dest) = pal[p2 & 0x03]; /* 7 */
      if (!(p1 & 0x03)) --dest; else *(--dest) = pal[p1 & 0x03]; /* 6 */
      p2 >>= 2;
      p1 >>= 2;
      
      if (!(p2 & 0x03)) --dest; else *(--dest) = pal[p2 & 0x03]; /* 5 */
      if (!(p1 & 0x03)) --dest; else *(--dest) = pal[p1 & 0x03]; /* 4 */
      p2 >>= 2;
      p1 >>= 2;
      
      if (!(p2 & 0x03)) --dest; else *(--dest) = pal[p2 & 0x03]; /* 3 */
      if (!(p1 & 0x03)) --dest; else *(--dest) = pal[p1 & 0x03]; /* 2 */
      p2 >>= 2;
      p1 >>= 2;
      
      if (!(p2 & 0x03)) --dest; else *(--dest) = pal[p2 & 0x03]; /* 1 */
      if (!(p1 & 0x03)) --dest; else *(--dest) = pal[p1 & 0x03]; /* 0 */
      p2 >>= 2;
      p1 >>= 2;
    }
  }
}


inline void
spr_scan_color_render (un16 *dest, struct obj *vs, un16 *pal, byte t1, byte t2)
{
  byte p1, p2;
  
  p1 = (t1 & 0xaa) | ((t2 >> 1) & 0x55);
  p2 = ((t1 << 1) & 0xaa) | (t2 & 0x55);
  
  if (vs->flags & 0x80) 
  {
    /* lesser priority */
    
    if (vs->flags & 0x20) 
    {
      /* horizontal mirroring */
      
      byte *sbuf = scan.solid_buf + LCD_SIDE_MARGIN + (vs->x - 8);
      
      if (*sbuf++ | !(p2 & 0x03)) ++dest; else *dest++ = pal[p2 & 0x03]; /* 7 */
      if (*sbuf++ | !(p1 & 0x03)) ++dest; else *dest++ = pal[p1 & 0x03]; /* 6 */
      p2 >>= 2;
      p1 >>= 2;
      
      if (*sbuf++ | !(p2 & 0x03)) ++dest; else *dest++ = pal[p2 & 0x03]; /* 5 */
      if (*sbuf++ | !(p1 & 0x03)) ++dest; else *dest++ = pal[p1 & 0x03]; /* 4 */
      p2 >>= 2;
      p1 >>= 2;
      
      if (*sbuf++ | !(p2 & 0x03)) ++dest; else *dest++ = pal[p2 & 0x03]; /* 3 */
      if (*sbuf++ | !(p1 & 0x03)) ++dest; else *dest++ = pal[p1 & 0x03]; /* 2 */
      p2 >>= 2;
      p1 >>= 2;
      
      if (*sbuf++ | !(p2 & 0x03)) ++dest; else *dest++ = pal[p2 & 0x03]; /* 1 */
      if (*sbuf++ | !(p1 & 0x03)) ++dest; else *dest++ = pal[p1 & 0x03]; /* 0 */
      p2 >>= 2;
      p1 >>= 2;
    }
    else 
    {
      /* byte *sbuf = scan.solid_buf + LCD_SIDE_MARGIN + (vs->x - 8) + 8 ; */
      byte *sbuf = scan.solid_buf + LCD_SIDE_MARGIN + vs->x;
      
      dest += 8;
      
      if (*(--sbuf) | !(p2 & 0x03)) --dest; else *(--dest) = pal[p2 & 0x03]; /* 7 */
      if (*(--sbuf) | !(p1 & 0x03)) --dest; else *(--dest) = pal[p1 & 0x03]; /* 6 */
      p2 >>= 2;
      p1 >>= 2;
      
      if (*(--sbuf) | !(p2 & 0x03)) --dest; else *(--dest) = pal[p2 & 0x03]; /* 5 */
      if (*(--sbuf) | !(p1 & 0x03)) --dest; else *(--dest) = pal[p1 & 0x03]; /* 4 */
      p2 >>= 2;
      p1 >>= 2;
      
      if (*(--sbuf) | !(p2 & 0x03)) --dest; else *(--dest) = pal[p2 & 0x03]; /* 3 */
      if (*(--sbuf) | !(p1 & 0x03)) --dest; else *(--dest) = pal[p1 & 0x03]; /* 2 */
      p2 >>= 2;
      p1 >>= 2;
      
      if (*(--sbuf) | !(p2 & 0x03)) --dest; else *(--dest) = pal[p2 & 0x03]; /* 1 */
      if (*(--sbuf) | !(p1 & 0x03)) --dest; else *(--dest) = pal[p1 & 0x03]; /* 0 */
      p2 >>= 2;
      p1 >>= 2;
    }
  }
  else
  {
    if (vs->flags & 0x20) 
    {
      /* horizontal mirroring */
      byte *sbuf = scan.solid_buf + LCD_SIDE_MARGIN + (vs->x - 8);
      byte *abuf = scan.cgb_attr_buf + LCD_SIDE_MARGIN + (vs->x - 8);
      
      if ( ((*(sbuf++) == 0) | (*(abuf) == 0)) && (p2 & 0x03)) /* 7 */
        *dest++ = pal[p2 & 0x03];
      else
        ++dest;
      if ( ((*sbuf++ == 0) | (*abuf++ == 0)) && (p1 & 0x03)) /* 6 */
        *dest++ = pal[p1 & 0x03];
      else
        ++dest;
      p2 >>= 2;
      p1 >>= 2;
      
      if ( ((*sbuf++ == 0) | (*abuf++ == 0)) && (p2 & 0x03)) /* 5 */
        *dest++ = pal[p2 & 0x03];
      else
        ++dest;
      if ( ((*sbuf++ == 0) | (*abuf++ == 0)) && (p1 & 0x03)) /* 4 */
        *dest++ = pal[p1 & 0x03];
      else
        ++dest;
      p2 >>= 2;
      p1 >>= 2;
      
      if ( ((*sbuf++ == 0) | (*abuf++ == 0)) && (p2 & 0x03)) /* 3 */
        *dest++ = pal[p2 & 0x03];
      else
        ++dest;
      if ( ((*sbuf++ == 0) | (*abuf++ == 0)) && (p1 & 0x03)) /* 2 */
        *dest++ = pal[p1 & 0x03];
      else
        ++dest;
      p2 >>= 2;
      p1 >>= 2;

      if ( ((*sbuf++ == 0) | (*abuf++ == 0)) && (p2 & 0x03)) /* 1 */
        *dest++ = pal[p2 & 0x03];
      else
        ++dest;
      if ( ((*sbuf++ == 0) | (*abuf++ == 0)) && (p1 & 0x03)) /* 0 */
        *dest++ = pal[p1 & 0x03];
      else
        ++dest;
      p2 >>= 2;
      p1 >>= 2;
    }
    else 
    {
      byte *sbuf = scan.solid_buf + LCD_SIDE_MARGIN + vs->x;
      byte *abuf = scan.cgb_attr_buf + LCD_SIDE_MARGIN + vs->x;
      
      dest += 8;
      if ( ((*(--sbuf) == 0) | (*(--abuf) == 0)) && (p2 & 0x03)) /* 7 */
        *(--dest) = pal[p2 & 0x03];
      else
        --dest;
      if ( ((*(--sbuf) == 0) | (*(--abuf) == 0)) && (p1 & 0x03)) /* 6 */
        *(--dest) = pal[p1 & 0x03];
      else
        --dest;
      p2 >>= 2;
      p1 >>= 2;
      
      if ( ((*(--sbuf) == 0) | (*(--abuf) == 0)) && (p2 & 0x03)) /* 5 */
        *(--dest) = pal[p2 & 0x03];
      else
        --dest;
      if ( ((*(--sbuf) == 0) | (*(--abuf) == 0)) && (p1 & 0x03)) /* 4 */
        *(--dest) = pal[p1 & 0x03];
      else
        --dest;
      p2 >>= 2;
      p1 >>= 2;

      if ( ((*(--sbuf) == 0) | (*(--abuf) == 0)) && (p2 & 0x03)) /* 3 */
        *(--dest) = pal[p2 & 0x03];
      else
        --dest;
      if ( ((*(--sbuf) == 0) | (*(--abuf) == 0)) && (p1 & 0x03)) /* 2 */
        *(--dest) = pal[p1 & 0x03];
      else
        --dest;
      p2 >>= 2;
      p1 >>= 2;

      if ( ((*(--sbuf) == 0) | (*(--abuf) == 0)) && (p2 & 0x03)) /* 1 */
        *(--dest) = pal[p2 & 0x03];
      else
        --dest;
      if ( ((*(--sbuf) == 0) | (*(--abuf) == 0)) && (p1 & 0x03)) /* 0 */
        *(--dest) = pal[p1 & 0x03];
      else
        --dest;
      p2 >>= 2;
      p1 >>= 2;
    }
  }
}


#endif


void 
#ifdef LCD_SPR_SCAN_16
spr_scan_16()
#else
spr_scan_8()
#endif
{
  byte ns;
  un16 *dest;
  byte *vram = lcd.vbank[0];
  byte t1, t2;
  struct obj *vs;
  int tile_y;
  
  for (ns = NS; ns; ns--)
  {
    vs = VS[ns - 1];
    
    if ((vs->x >= 168) | (vs->x == 0)) continue;
    
    if (vs->flags & 0x40) 
    {
      /* vertically mirroring */
#ifdef LCD_SPR_SCAN_16
      /* tile_y = (15 - (L - vs->y + 16)) << 1; */
      tile_y = ((un8)(vs->y) - L - 1) << 1;
#else
      /* tile_y = (7 - (L - vs->y + 16)) << 1; */
      tile_y = ((un8)(vs->y) - L - 9) << 1;
#endif
    }
    else
      tile_y = (L - (un8)(vs->y) + 16) << 1;
    
#ifdef LCD_SPR_SCAN_16
    t2 = vram[(un8)(vs->pat & 0xfe) * 16 + tile_y + 0];
    t1 = vram[(un8)(vs->pat & 0xfe) * 16 + tile_y + 1];
#else
    t2 = vram[(un8)(vs->pat) * 16 + tile_y + 0];
    t1 = vram[(un8)(vs->pat) * 16 + tile_y + 1];
#endif
    
    dest = BUF + LCD_SIDE_MARGIN + (vs->x - 8);
    
    spr_scan_render (dest, vs, PAL2 + 0x20 + ((vs->flags & 0x10) >> 2), t1, t2);
  }
}


void 
#ifdef LCD_SPR_SCAN_16
spr_scan_color_16()
#else
spr_scan_color_8()
#endif
{
  byte ns;
  un16 *dest;
  byte *vram = lcd.vbank[0];
  byte t1, t2;
  struct obj *vs;
  int tile_y;
  
  for (ns = NS; ns; ns--)
  {
    vs = VS[ns - 1];
    
    if ((vs->x >= 168) | (vs->x == 0)) continue;
    
    vram = lcd.vbank[(vs->flags & 0x08) >> 3];
    
    if (vs->flags & 0x40) 
    {
      /* vertically mirroring */
#ifdef LCD_SPR_SCAN_16
      /* tile_y = (15 - (L - vs->y + 16)) << 1; */
      tile_y = (vs->y - L - 1) << 1;
#else
      /* tile_y = (7 - (L - vs->y + 16)) << 1; */
      tile_y = (vs->y - L - 9) << 1;
#endif
    }
    else
      tile_y = (L - vs->y + 16) << 1;
    
#ifdef LCD_SPR_SCAN_16
    t2 = vram[(un8)(vs->pat & 0xfe) * 16 + tile_y + 0];
    t1 = vram[(un8)(vs->pat & 0xfe) * 16 + tile_y + 1];
#else
    t2 = vram[(un8)(vs->pat) * 16 + tile_y + 0];
    t1 = vram[(un8)(vs->pat) * 16 + tile_y + 1];
#endif
    
    dest = BUF + LCD_SIDE_MARGIN + (vs->x - 8);
    spr_scan_color_render (dest, vs, PAL2 + 0x20 + ((vs->flags & 0x07) << 2), t1, t2);
  }
}


