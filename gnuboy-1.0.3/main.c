#include "gnuboy.h"
#include <kos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stdarg.h>
#include <signal.h>

#include "input.h"
#include "rc.h"
#include "mem.h"


#include "Version"
#include "sys/dc/dc_utils.h"
#include "sys/dc/dc_menu.h"
#include "sys/dc/dc_vmu.h"
#include "sys/dc/fs_md.h"

#define DC_FILES_ROOTPATH "/cd"

static uint16 startup_image[320 * 240];
static uint16 menu_image[320 * 240];
static uint16 credits_image[320 * 240];
static uint16 options_image[320 * 240];

uint8 dc_mvmu = 0;
uint8 dc_mlcd = 0;

/* vmu screen image */
#include "sys/dc/vmu_icons/kos.h"


void doevents()
{
  event_t ev;
  int st;

  ev_poll();

#if 0
  while (ev_getevent(&ev))
  {
    if (ev.type != EV_PRESS && ev.type != EV_RELEASE)
      continue;
    st = (ev.type != EV_RELEASE);
    rc_dokey(ev.code, st);
  }
#endif
}



#include "hw.h"


static void
run_game(const char *filename)
{
  dc_vid_empty ();
  
  exit_game_loop = 0;
  
  gb_vid_init ();
  pcm_init ();
  mem_init ();
  
  dc_print ("loading ROM");
  if (loader_init(filename)) 
  {
    emu_reset();
    emu_run ();
    
    pcm_clear_buffer();
    loader_unload ();
  }
  
  gb_vid_close();
  pcm_close();
}


static void
draw_main_menu ()
{
  display_rawimage (menu_image);
}


static char last_romfile[256];

static void
do_rom_menu_and_run () 
{
  fm_result_t result;
  char *p;
  char last_romfile_dir[256];
  fm_config_t fm_config;
  
  sprintf (last_romfile_dir, "%s", last_romfile);
  p = strrchr(last_romfile_dir, '/');
  *p = '\0';
  
  fm_config.vertical_wait = 80;
  fm_config.horizontal_wait = 80;
  fm_config.root_path = DC_FILES_ROOTPATH;
  if (do_file_menu (&result, last_romfile_dir, &fm_config))
  {
    if (strcmp(result.path, last_romfile))
      unlink_quickstate ();
    
    run_game (result.path);
    sprintf (last_romfile, "%s", result.path);
  }
}



static void
draw_main_menu_vmu_icon ()
{
  char vmu_screen[48 * 32];
  int vmu_free_blocks;
  char str[32];
  
  timer_spin_sleep (50); /* need wait */
  vmu_free_blocks = ndc_vmu_check_free_blocks (NULL, dc_mvmu);
  
  vmu_icon_clear (vmu_screen, 1);
  vmu_icon_draw_string (vmu_screen, 1, 1, "VMU FREE:");
  sprintf(str, "%d BLOCKS", vmu_free_blocks);
  vmu_icon_draw_string (vmu_screen, 3, 7, str);
  
  vmu_icon_draw_string (vmu_screen, 1, 20, "gnuboy/DC");
  vmu_icon_draw_string (vmu_screen, 3, 26, "1.0.3-0.6");
  
  vmu_icon_flip (vmu_screen, dc_mlcd);
  
  timer_spin_sleep (200); /* need wait */
}


static void 
do_credits() 
{
  int state = 0;
  int count = 0;
  cont_cond_t cont;
  
  for (;;)
  {
    cont_get_cond (dc_controller_addr[0], &cont);
    
    if (cont.rtrig && cont.ltrig)
      break;
    
    display_rawimage (credits_image);
    
    if (++count == 20)
    {
      count = 0;
      state ^= 1;
    }
    
    if (state)
      draw_string (10, 10, _white, _black, "pull L+R");
    
    dc_vid_flip (draw_type_fullscreen);
  }
}


static void
load_main_menu ()
{
  cont_cond_t cont;
	
  draw_main_menu_vmu_icon ();
  
  for (;;) 
  {
    cont_get_cond (dc_controller_addr[0], &cont);
    
    if (cont.rtrig && cont.ltrig && 
	!(cont.buttons & CONT_Y) &&
	!(cont.buttons & CONT_START))
      break;
    
    if (!(cont.buttons & CONT_A)) 
    {
      do_rom_menu_and_run ();
      draw_main_menu_vmu_icon ();
    }
    
#if 0
    if (!(cont.buttons & CONT_X)) 
      do_main_options ();
    
#endif
    if (!(cont.buttons & CONT_Y)) 
      do_credits ();
    
    if (!(cont.buttons & CONT_B)) 
    {
      do_vmu_menu (dc_mvmu);
      draw_main_menu_vmu_icon ();
    }
    
    draw_main_menu ();
    dc_vid_flip (draw_type_fullscreen);
  }
}


static void
draw_startup_image (const char *string)
{
  display_rawimage (startup_image);
#ifdef __DC_PAL__
  draw_string(10, 200, _white, _black, "gnuboy/DC 1.0.3-0.6-PAL");
#else
  draw_string(10, 200, _white, _black, "gnuboy/DC 1.0.3-0.6");
#endif
  draw_string(10, 210, _white, _none, string);
  dc_vid_flip_fill_renderer (draw_type_fullscreen);
}


static const char *save_filename = "dgb_r0.3";

static uint8 
find_vmu ()
{
  int n;
  uint8 addr;
	
  n = 0;
  for (;;)
  {
    addr = dc_maple_find_nth_device (n, MAPLE_FUNC_MEMCARD);
    ++n;
    if (!addr)
      return 0;
    else
    {
      if (ndc_vmu_get_dirent (NULL, addr, save_filename) >= 0)
	return addr;
    }
  }
  return 0;
}


static void
load_option_from_vmu (uint8 addr)
{
  int32 firstblk = ndc_vmu_get_dirent (NULL, addr, save_filename);
  if (firstblk < 0) return;
	
  uint8 buf[1024];
  if (ndc_vmu_read_blocks (addr, firstblk, buf, 2) != 2)
  {
    dc_put_error ("error loading user settings");
    return;
  }
	
#if 0	
  set_cont_scheme                         (buf[640 + 0]);
  set_sound                               (buf[640 + 1]);
  set_use_vmu                             (buf[640 + 2]);
  set_exsound                             (buf[640 + 3]);
  set_bi_filter                           (buf[640 + 4]);
#endif
}


static void
save_option_to_vmu (uint8 addr)
{
  if (!addr) return;
	
  dc_print ("saving user settings...");
	
  uint8 buf[1024];
  ndc_vmu_create_vmu_header (buf, save_filename, "gnuboy/DC settings compat0.3",
			     1024 - 128, NULL);
  memset(buf + 640, 0, 1024 - 640);
#if 0
  buf[640 + 0] = cont_scheme;
  buf[640 + 1] = sound_on;
  buf[640 + 2] = use_vmu;
  buf[640 + 3] = exsound;
  buf[640 + 4] = bilinear_filter;
#endif
  ndc_vmu_do_crc (buf, 1024);
	
  int32 firstblk = ndc_vmu_get_dirent (NULL, addr, save_filename);
  if (firstblk < 0) 
  {
    uint8 free_blocks = ndc_vmu_check_free_blocks (NULL, addr);
		
    if (free_blocks < 2) 
    {
      dc_put_error ("You need 2 free blocks to Save user settings");
      return;
    }
		
    firstblk = ndc_vmu_allocate_file (addr, save_filename, 2);
    if (firstblk < 0)
    {
      dc_put_error ("error updating fat & directory");
      return;
    }
  }
	
  if (ndc_vmu_write_blocks (addr, firstblk, buf, 2) != 2)
  {
    dc_put_error ("error writing data");
    return;
  }
}


int main()
{
  kos_init_all(IRQ_ENABLE, NULL);
  dc_vid_init ();
  
  fs_md_init (max_statefile_size);
  dc_maple_controller_init ();
  
  dc_mlcd = maple_first_lcd ();
  vmu_icon_flip (vmu_kos_xpm, dc_mlcd);
  
  load_bmp (startup_image, DC_FILES_ROOTPATH "/pics/startup.bmp");
  
  draw_startup_image ("startup");
  load_bmp (menu_image, DC_FILES_ROOTPATH "/pics/menu.bmp");
  draw_startup_image ("startup.");
  load_bmp (credits_image, DC_FILES_ROOTPATH "/pics/credits.bmp");
  draw_startup_image ("startup..");
  load_bmp (options_image, DC_FILES_ROOTPATH "/pics/options.bmp");
  draw_startup_image ("startup...");
  init_menus ();
  
  sprintf (last_romfile, DC_FILES_ROOTPATH "/gb/default.gb");
  
  draw_startup_image ("load setting");
  dc_mvmu = find_vmu ();
  if (!dc_mvmu)
  {
    dc_mvmu = do_vmu_select_menu ();
    if (dc_mvmu)
      save_option_to_vmu (dc_mvmu);
  }
  
  run_game (DC_FILES_ROOTPATH "/gb/default.gb");
  
  load_main_menu ();
  
  {
    char vmu_screen[48 * 32];
    vmu_icon_clear (vmu_screen, 0);
    vmu_icon_flip(vmu_screen, dc_mlcd);
  }
  
  fs_md_shutdown ();
  kos_shutdown_all ();
  return 0;
}

