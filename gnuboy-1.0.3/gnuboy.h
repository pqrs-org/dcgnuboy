#ifndef __GNUBOY_H
#define __GNUBOY_H

#include <stdio.h>
#include "defs.h"
#include <kos.h>

extern int exit_game_loop;

/* sys/dc/dc.c */
void sys_initall ();
void sys_set_timer ();
void sys_terminate_timer ();

void vmu_draw_settings ();

void gb_vid_init();
void gb_vid_preinit();
void gb_vid_close();

void emu_init();
void emu_reset();
void emu_run();

void ev_poll();
void rc_dokey(int key, int st);

void pcm_init();
void pcm_clear_buffer();
void pcm_close();
void render_pcm ();

void unlink_quickstate ();

/* exports.c */
void init_exports();

/* loader.c */
int loader_init(const char *s);
void loader_unload();
int stateload_from_vmu ();
int statesave_to_vmu ();

/* palette.c */
void pal_set332();
void pal_expire();

/* refresh.c */
void refresh_1(byte *dest, byte *src, byte *pal, int cnt);
void refresh_2(un16 *dest, byte *src, un16 *pal, int cnt);
void refresh_3(byte *dest, byte *src, un32 *pal, int cnt);
void refresh_4(un32 *dest, byte *src, un32 *pal, int cnt);
void refresh_1_2x(byte *dest, byte *src, byte *pal, int cnt);
void refresh_2_2x(un16 *dest, byte *src, un16 *pal, int cnt);
void refresh_3_2x(byte *dest, byte *src, un32 *pal, int cnt);
void refresh_4_2x(un32 *dest, byte *src, un32 *pal, int cnt);
void refresh_2_3x(un16 *dest, byte *src, un16 *pal, int cnt);
void refresh_3_3x(byte *dest, byte *src, un32 *pal, int cnt);
void refresh_4_3x(un32 *dest, byte *src, un32 *pal, int cnt);
void refresh_3_4x(byte *dest, byte *src, un32 *pal, int cnt);
void refresh_4_4x(un32 *dest, byte *src, un32 *pal, int cnt);


/* hw.c */
void hw_interrupt(byte i, byte mask);
void hw_dma(byte b);
void hw_hdma_cmd(byte c);
void hw_hdma();
void pad_refresh();
void pad_press(byte k);
void pad_release(byte k);
void pad_set(byte k, int st);
void hw_reset();

/* sys/ */
void *sys_timer();
int sys_elapsed(void *p);
void sys_sleep(int us);
void sys_initpath();
void sys_sanitize(char *s);
void unlink_quickstate ();

/* save.c */
void loadstate(FILE *f);
void savestate(FILE *f);
#if 0
int get_max_statefile_size ();
#else
extern const int max_statefile_size;
#endif

/* inflate.c */
int unzip (const unsigned char *data, long *p,
           void (* callback) (unsigned char d));

/* lcd.c */
void lcd_reset();

/* sound.c */
void sound_mix(int samples);

#endif
