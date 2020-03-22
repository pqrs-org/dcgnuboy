#include <kos.h>
#include <stdio.h>
#include "dc_utils.h"


/* ------------------------------------------------------------ */
void 
timer_sleep(int msec)
{
  if (thd_enabled)
    thd_sleep(msec);
  else
    timer_spin_sleep(msec);
}


/* ------------------------------------------------------------ */
#include <dc/sq.h>
/* n must be multiple of 64 */
void
dc_sq_cpy(void *dest, void *src, int n)
{
  uint32 *sq;
  uint32 *d = (uint32 *)dest;
  uint32 *s = (uint32 *)src;
  uint32 *pref_addr;
  
  /* Set store queue memory area as desired */
  QACR0 = ((((uint32)dest)>>26)<<2)&0x1c;
  QACR1 = ((((uint32)dest)>>26)<<2)&0x1c;
  
  n >>= 6;
  while (n--) 
  {
    /* sq0 */ 
    sq = (uint32 *)(0xe0000000);
    pref_addr = (uint32 *)(0xe0000000 | ((uint32)d & 0x3ffffc0));
    *sq++ = *s++; *sq++ = *s++;
    *sq++ = *s++; *sq++ = *s++;
    *sq++ = *s++; *sq++ = *s++;
    *sq++ = *s++; *sq++ = *s++;
    asm("pref @%0" : : "r" (pref_addr));
    d += 8;
    
    /* sq1 */
    sq = (uint32 *)(0xe0000000 | 0x20);
    pref_addr = (uint32 *)(0xe0000000 | ((uint32)d & 0x3ffffc0) | 0x20);
    *sq++ = *s++; *sq++ = *s++;
    *sq++ = *s++; *sq++ = *s++;
    *sq++ = *s++; *sq++ = *s++;
    *sq++ = *s++; *sq++ = *s++;
    asm("pref @%0" : : "r" (pref_addr));
    d += 8;
  }
}


void
dc_wait_sq_cpy_done ()
{
  /* wait for both store queues to complete */
  *((uint32 *)(0xe0000000)) = 0;
  *((uint32 *)(0xe0000000 | 0x20)) = 0;
}


/* ------------------------------------------------------------ */
static uint16 dc_screen[320 * 240] __attribute__ ((aligned (32)));

static screen_rect dc_screen_rect;

void
dc_set_default_screen_rect ()
{
  screen_rect default_rect = {0, 640, 16 * 2, 480 - 16 * 2};
  
  dc_screen_rect = default_rect;
}


void
dc_set_screen_rect (screen_rect *rect)
{
  dc_screen_rect = *rect;
}


void
dc_get_screen_rect (screen_rect *rect)
{
  *rect = dc_screen_rect;
}


/* DC_TEXTURE_ADDRS_SIZE must be 2^x */
#define DC_TEXTURE_ADDRS_SIZE 2
static pvr_ptr_t dc_texture_addrs[DC_TEXTURE_ADDRS_SIZE];
static pvr_poly_hdr_t dc_pvr_poly_headers_filter_none[DC_TEXTURE_ADDRS_SIZE];
static pvr_poly_hdr_t dc_pvr_poly_headers_filter_bilinear[DC_TEXTURE_ADDRS_SIZE];


typedef struct { 
  float u1, u2;
  float v1, v2;
  pvr_poly_hdr_t *poly_headers;
} dc_pvr_setting_t; 


static dc_pvr_setting_t dc_pvr_setting[draw_type_size + 1];


void
dc_set_pvr_setting (draw_type_t dt, 
                    float u1, float u2, 
                    float v1, float v2, 
                    int filtering)
{
  dc_pvr_setting_t *p = dc_pvr_setting + dt;
  
  if (0 <= u1 && u1 <= 1) p->u1 = u1;
  if (0 <= u2 && u2 <= 1) p->u2 = u2;

  if (0 <= v1 && v1 <= 1) p->v1 = v1;
  if (0 <= v2 && v2 <= 1) p->v2 = v2;
  
  switch (filtering) 
  {
    case PVR_FILTER_BILINEAR:
      p->poly_headers = dc_pvr_poly_headers_filter_bilinear;
      break;
      
    case PVR_FILTER_NONE:
    default:
      p->poly_headers = dc_pvr_poly_headers_filter_none;
      break;
  }
}


void
dc_set_default_pvr_setting () 
{
  memset (dc_pvr_setting, 0, sizeof(dc_pvr_setting));
  
  dc_set_pvr_setting (draw_type_fullscreen, 
                      0, 320.0/512, 0, 240.0/256, PVR_FILTER_NONE);
  
  dc_set_pvr_setting (draw_type_gb_fullscreen, 
                      0, 160.0/512, 0, 144.0/256, PVR_FILTER_BILINEAR);
}


/* 256x240 texture */ 
static void
draw_screen(int texture_index, draw_type_t dt)
{
  pvr_vertex_t   vert;
  screen_rect *rect = &dc_screen_rect;
  dc_pvr_setting_t *setting = dc_pvr_setting + dt;
  
  pvr_wait_ready ();
  pvr_scene_begin ();
  pvr_list_begin (PVR_LIST_OP_POLY);
  
  pvr_prim (setting->poly_headers + texture_index, sizeof(pvr_poly_hdr_t));
  
  vert.flags = PVR_CMD_VERTEX;
  vert.x = rect->x1;
  vert.y = rect->y2;
  vert.z = 512.0f;
  vert.u = setting->u1;
  vert.v = setting->v2;
  vert.argb = PVR_PACK_COLOR (1, 1, 1, 1);
  vert.oargb = 0;
  pvr_prim (&vert, sizeof(vert));
  
  vert.x = rect->x1;
  vert.y = rect->y1;
  vert.u = setting->u1;
  vert.v = setting->v1;
  pvr_prim (&vert, sizeof(vert));
  
  vert.x = rect->x2;
  vert.y = rect->y2;
  vert.u = setting->u2;
  vert.v = setting->v2;
  pvr_prim (&vert, sizeof(vert));
  
  vert.flags = PVR_CMD_VERTEX_EOL;
  vert.x = rect->x2;
  vert.y = rect->y1;
  vert.u = setting->u2;
  vert.v = setting->v1;
  pvr_prim (&vert, sizeof(vert));
  
  pvr_list_finish ();
  
  pvr_scene_finish ();
}


static pvr_init_params_t pvr_params = 
{
  /* Enable opaque polygons with size 16 */
  { PVR_BINSIZE_16, PVR_BINSIZE_0, PVR_BINSIZE_0, PVR_BINSIZE_0, PVR_BINSIZE_0 },
  
  /* Vertex buffer size 512K */
  512*1024
};


void
dc_vid_init ()
{
  int i;
  pvr_poly_cxt_t poly;
  
  pvr_init (&pvr_params); 
  
  for (i = 0; i < DC_TEXTURE_ADDRS_SIZE; ++i) {
    pvr_ptr_t addr;
    
    addr = pvr_mem_malloc (512 * 256 * 2);
    
    dc_texture_addrs[i] = addr; 
    
    pvr_poly_cxt_txr (&poly, PVR_LIST_OP_POLY, 
                      PVR_TXRFMT_RGB565 | PVR_TXRFMT_NONTWIDDLED,
                      512, 256, addr, PVR_FILTER_NONE);
    pvr_poly_compile (dc_pvr_poly_headers_filter_none + i, &poly);
    
    pvr_poly_cxt_txr (&poly, PVR_LIST_OP_POLY, 
                      PVR_TXRFMT_RGB565 | PVR_TXRFMT_NONTWIDDLED,
                      512, 256, addr, PVR_FILTER_BILINEAR);
    pvr_poly_compile (dc_pvr_poly_headers_filter_bilinear + i, &poly);
  }
  
  dc_set_default_screen_rect ();
  dc_set_default_pvr_setting ();
  
  dc_vid_empty ();
}


uint16*
dc_get_screen ()
{
  return dc_screen;
}


void
dc_vid_clear ()
{
  memset(dc_screen, 0, sizeof(dc_screen));
}


void
dc_vid_empty ()
{
  int i;
  memset(dc_screen, 0, sizeof(dc_screen));
  
  /* clear texture buffer */
  for (i = 0; i < DC_TEXTURE_ADDRS_SIZE; ++i)
    dc_vid_flip (draw_type_fullscreen);
}


void
dc_vid_flip (draw_type_t dt)
{
  static int texture_index = 0;
  int i;
  uint16 *d, *s;
  
  d = dc_texture_addrs[texture_index];
  s = dc_screen;
  
  if (dt == draw_type_fullscreen)
  {
    for (i = 0; i < 240; ++i)
    {
      dc_sq_cpy (d, s, 320 * 2);
      d += 512;
      s += 320;
    }
    dc_wait_sq_cpy_done ();
  }
  else
  {
    s += 8;
    for (i = 0; i < 144; ++i)
    {
      dc_sq_cpy (d, s, 160 * 2);
      d += 512;
      s += 320;
    }
    dc_wait_sq_cpy_done ();
  }
  
  draw_screen (texture_index, dt);
  texture_index = (texture_index + 1) & (DC_TEXTURE_ADDRS_SIZE - 1);
}


void
dc_vid_flip_fill_renderer (draw_type_t dt)
{
  int i;
  for (i = 0; i < 4; ++i) 
    dc_vid_flip (dt);
}


/* ------------------------------------------------------------ */
/* same as maple_first_device but assign ordinal. */
uint8
dc_maple_find_nth_device(int n, int code) 
{
  int port, unit;
  
  for (port = 0; port < 4; ++port)
  {
    for (unit = 0; unit < 6; ++unit) 
    {
      if (maple_device_func(port, unit) & code)
      {
        if (n-- == 0)
          return maple_create_addr(port, unit);
      }
    }
  }
  return 0;
}


uint8 dc_controller_addr[4];

void
dc_maple_controller_init ()
{
  int i;
  
  for (i = 0; i < 4; ++i)
    dc_controller_addr[i] = dc_maple_find_nth_device (i, MAPLE_FUNC_CONTROLLER);
}


int
is_buttons_press (cont_cond_t *cont, uint16 buttons)
{
  return ((cont->buttons & buttons) != buttons);
}


void
wait_until_release_buttons (uint8 addr, uint16 buttons)
{
  cont_cond_t cont;
  
  for (;;)
  {
    cont_get_cond (addr, &cont);
    
    if (!(is_buttons_press (&cont, buttons)))
      break;
  }
}


void
wait_until_release_trigger (uint8 addr)
{
  cont_cond_t cont;
  
  for (;;)
  {
    cont_get_cond (addr, &cont);
    
    if (!(cont.rtrig || cont.ltrig))
      break;
  }
}


/* ------------------------------------------------------------ */
#include "vmu_icons/3x5fonts.h"

void
vmu_icon_clear (char *vmu_screen, int is_black)
{
  if (is_black)
    memset(vmu_screen, '+', 48 * 32);
  else
    memset(vmu_screen, '.', 48 * 32);
}


void
vmu_icon_draw_char (char *vmu_screen, int x, int y, int ch)
{
  const char *font;
  char *screen;
  int x1, y1;
  
  if (ch <= 31 || ch >= 127) return;
  if (x < 0 || x >= 48 - 3) return;
  if (y < 0 || y >= 32 - 5) return;
  
  font = vmu_3x5fonts + 3 * 5 * (ch - 32);
  screen = vmu_screen + y * 48 + x;
  
  for (y1 = 0; y1 < 5; ++y1)
  {
    for (x1 = 0; x1 < 3; ++x1)
    {
      if (*font == '#')
        *screen = '.';
      ++font;
      ++screen;
    }
    screen += 48 - 3;
  }
}


void
vmu_icon_draw_string (char *vmu_screen, int x, int y, const char *str)
{
  while (*str) 
  {
    vmu_icon_draw_char(vmu_screen, x, y, *str);
    x += 4;
    str++;
  }
}


void
vmu_icon_flip(const char *vmu_icon, uint8 addr) 
{
  uint8 bitmap[(48 / 8) * 32];
  uint8 *b;
  const char *v;
  int i, tile_x, y;
  
  if (!addr) return;
  
  memset (bitmap, 0, sizeof(bitmap));
  
  b = bitmap;
  v = vmu_icon + 48 * 32;
  for (y = 0; y < 32; y++)
  {
    /* draw (8x1 tile) x 6 */
    for (tile_x = 0; tile_x < 6; ++tile_x)
    {
      uint8 t = 0;
      
      for (i = 0; i < 8; ++i)
      {
        t <<= 1;
        if (*--v == '+') t |= 0x1;
      }
      *b++ = t;
    }
  }
  
  vmu_draw_lcd(addr, bitmap);
}


/* ------------------------------------------------------------ */
int
load_bmp (uint16 *raw, char *filename)
{
  FILE *fp = NULL;
  uint8 buffer[320 * 240 * 3];
  uint8 *bmp;
  int x, y;
  uint16 *p;
  
  fp = fopen(filename, "r");
  if (!fp) goto error;
  
  fseek(fp, 54, SEEK_SET);
  if (fread(buffer, sizeof(buffer), 1, fp) != 1) 
    goto error;
  
  fclose(fp);
  
  p = raw;
  for (y = 0; y < 240; ++y)
  {
    bmp = buffer + 320 * 3 * (239 - y);
    for (x = 0; x < 320; ++x)
    {
      uint16 r, g, b;
      
      b = *bmp++ * 32 / 256;
      g = *bmp++ * 64 / 256;
      r = *bmp++ * 32 / 256;
      
      *p++ = (r << 11) | (g << 5) | (b << 0);
    }
  }
  
  return 0;
  
error:
  if (fp) fclose(fp);
  memset (raw, 0, 320 * 240 * 2);
  return -1;
}


void
display_rawimage (uint16 *raw)
{
  memcpy (dc_get_screen (), raw, 320 * 240 * 2);
}


static uint8 dc_font_8x8[] = {
#include "dc_font_8x8.h"
};

/* Assumes a PM_RGB565 display mode. */
/* now works only 320x240. */
void 
draw_char(int x1, int y1, uint16 fg_color, uint16 bg_color, int ch)
{
  int x, y;
  uint8 *font = dc_font_8x8 + 4 + ch * 8;
  uint16 *screen = dc_get_screen () + y1 * 320 + x1;
  
  if (x1 < 0 || x1 >= 320 - 8) return;
  if (y1 < 0 || y1 >= 240 - 8) return;
  
  for (y = 0; y < 8; y++)
  {
    int mask = 0x80;
    for (x = 0; x < 8; x++)
    {
      if (*font & mask) 
        *screen = fg_color;
      else 
      {
        if (bg_color != _none)
          *screen = bg_color;
      }
      mask >>= 1;
      ++screen;
    }
    screen += 320 - 8;
    ++font;
  }
}


void
draw_string(int x1, int y1, uint16 fg_color, uint16 bg_color, const char *str) 
{
  while (*str) 
  {
    draw_char(x1, y1, fg_color, bg_color, *str);
    x1 += 8; str++;
  }
}


void
dc_print(const char *string)
{
  dc_vid_clear ();
  draw_string(10, 10, _white, _black, string);
  dc_vid_flip_fill_renderer (draw_type_fullscreen);
}


void 
dc_put_error (const char *string)
{
  dc_vid_clear ();
  draw_string(10, 10, _red, _black, string);
  dc_vid_flip_fill_renderer (draw_type_fullscreen);
  timer_sleep (1000);
}



