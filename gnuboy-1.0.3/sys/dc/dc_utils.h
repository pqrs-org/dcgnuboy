#ifndef _DC_UTILS_H_
#define _DC_UTILS_H_

#ifdef __cplusplus
extern "C" {
#endif

#if 0
}
#endif

#include "types.h"

void timer_sleep(int msec);
void dc_sq_cpy(void *dest, void *src, int n);
void dc_wait_sq_cpy_done ();

extern uint8 dc_controller_addr[4];

void dc_vid_init ();
uint16* dc_get_screen ();
void dc_vid_clear ();
void dc_vid_empty ();

typedef struct {
  float x1, x2;
  float y1, y2;
} screen_rect;

void dc_set_default_screen_rect ();
void dc_set_screen_rect (screen_rect *rect);
void dc_get_screen_rect (screen_rect *rect);

typedef enum {
  draw_type_fullscreen,
  draw_type_gb_fullscreen,
  draw_type_size, 
} draw_type_t; 

void dc_set_pvr_setting (draw_type_t dt, 
                         float u1, float u2, 
                         float v1, float v2, 
                         int filtering);

void dc_vid_flip (draw_type_t dt);
void dc_vid_flip_fill_renderer (draw_type_t dt);

uint8 dc_maple_find_nth_device(int n, int code);
void dc_maple_controller_init ();
int is_buttons_press (cont_cond_t *cont, uint16 buttons); 
void wait_until_release_buttons (uint8 addr, uint16 buttons);
void wait_until_release_trigger (uint8 addr);

void vmu_icon_clear (char *vmu_screen, int is_black);
void vmu_icon_draw_char (char *vmu_screen, int x, int y, int ch);
void vmu_icon_draw_string (char *vmu_screen, int x, int y, const char *str);
void vmu_icon_flip(const char *vmu_icon, uint8 addr);

int load_bmp (uint16 *raw, char *filename);

void display_rawimage (uint16 *raw);

// define some common colors 
#define _yellow (((255 >> 3) << 11) | ((255 >> 2) << 5) | ((0 >> 3) << 0))
#define _red    (((255 >> 3) << 11) | ((0 >> 2) << 5) | ((0 >> 3) << 0))
#define _green  (((0 >> 3) << 11) | ((255 >> 2) << 5) | ((0 >> 3) << 0))
#define _blue   (((20 >> 3) << 11) | ((20 >> 2) << 5) | ((255 >> 3) << 0))
#define _black  (((0 >> 3) << 11) | ((0 >> 2) << 5) | ((0 >> 3) << 0))
#define _white  (((255 >> 3) << 11) | ((255 >> 2) << 5) | ((255 >> 3) << 0))
#define _gray   (((200 >> 3) << 11) | ((200 >> 2) << 5) | ((200 >> 3) << 0))
#define _none   (((254 >> 3) << 11) | ((254 >> 2) << 5) | ((254 >> 3) << 0))

void font_set(uint8 *fbm, int fh); 
void draw_char(int x1, int y1, uint16 fg_color, uint16 bg_color, int ch);
void draw_string(int x1, int y1, uint16 fg_color, uint16 bg_color, const char *str);
void dc_print(const char *string);
void dc_put_error(const char *string);

#ifdef __cplusplus
}
#endif

#endif

