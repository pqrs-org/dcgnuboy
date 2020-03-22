/*
 * dc.c
 * Dreamcast interfaces -- based on svga.c 
 *
 * Takayama Fumihiko <tekezo@catv296.ne.jp>
 *
 * Licensed under the GPLv2, or later.
 */

#include <stdlib.h>
#include <stdio.h>

#include "fb.h"
#include "input.h"
#include "rc.h"
#include "hw.h"

#include "dc_utils.h"
#include "dc_menu.h"
#include "gnuboy.h"


extern int exit_game_loop;
extern int interrupt_game_loop;

static volatile int rest_frames = 1;
static int drop_frames = 0;

static void
clear_rest_frames () 
{
  rest_frames = 1;
  drop_frames = 0;
}


enum
{
	fsr_no_skip,
	fsr_auto, 
	fsr_one_and_half,
	fsr_double,
	fsr_max,
	fsr_frameskip_rate_num
} frameskip_rate;


const struct
{
	int cycles;
	char *vmu_str;
} frameskip_settings [] = {
	{	1,	"NO SKIP", },
	{	60,	"AUTO: x1.0", },
	{	90,	"AUTO: x1.5", },
	{	120,	"AUTO: x2.0", },
	{	240,	"AUTO: x4.0", },
	{	1,	NULL },
};


extern uint8 dc_mlcd;

void 
vmu_draw_settings ()
{
	char vmu_screen[48 * 32];
	
	vmu_icon_clear (vmu_screen, 1);
	
	vmu_icon_draw_string (vmu_screen, 1, 1, "FRAME RATE:");
	vmu_icon_draw_string (vmu_screen, 3, 7, 
			      frameskip_settings[frameskip_rate].vmu_str);
	
	vmu_icon_flip (vmu_screen, dc_mlcd);
}


static void
timer_handler(irq_t source, irq_context_t *context)
{
  ++rest_frames;
}


void
sys_initall ()
{
  frameskip_rate = fsr_no_skip;
  clear_rest_frames ();
}


void
sys_set_timer () 
{
  clear_rest_frames ();
  timer_prime (TMU2, frameskip_settings[frameskip_rate].cycles, true);
  irq_set_handler (EXC_TMU2_TUNI2, timer_handler);
  timer_start (TMU2);
}


void
sys_terminate_timer ()
{
	timer_stop (TMU2);
}


struct fb fb;

static int vmode[3] = { 0, 0, 8 };
static int svga_mode;
static int svga_vsync = 1;

rcvar_t vid_exports[] =
{
	RCV_VECTOR("vmode", vmode, 3),
	RCV_INT("vsync", &svga_vsync),
	RCV_INT("svga_mode", &svga_mode),
	RCV_END
};


void *sys_timer()
{
	return NULL;
}


int sys_elapsed(void *p)
{
	return 0;
}


void sys_sleep(int us)
{
}


void sys_checkdir(char *path, int wr)
{
}


void sys_initpath()
{
}


void sys_sanitize(char *s)
{
}


int *rc_getvec();


void gb_vid_preinit()
{
}

void gb_vid_init()
{
	fb.w = 160;
	fb.h = 144;
	fb.pelsize = 2;
	fb.pitch = 320; /* dc_screen size */
	fb.ptr = dc_get_screen ();
	fb.enabled = 1;
	fb.dirty = 1;
	
	fb.indexed = 0;
	fb.cc[0].r = fb.cc[2].r = 3;
	fb.cc[1].r = 2;
	fb.cc[0].l = 11;
	fb.cc[1].l = 5;
	fb.cc[2].l = 0;
}


void gb_vid_close()
{
}

void vid_settitle(char *title)
{
}

void vid_setpal(int i, int r, int g, int b)
{
}


void vid_begin()
{
}


void vid_end()
{
  if (fb.enabled)
    dc_vid_flip (draw_type_gb_fullscreen);
  
  fb.enabled = 1;
  if (frameskip_settings[frameskip_rate].cycles == 1)
    return;
  
  if (drop_frames)
  {
    fb.enabled = 0;
    --drop_frames;
  }
  else 
  {
    --rest_frames;
    if (rest_frames > 0)
    {
      drop_frames = rest_frames;
      rest_frames = 0;
    }
  }
}

void kb_init()
{
}

void kb_close()
{
}

void kb_poll()
{
}


rcvar_t joy_exports[] =
{
	RCV_END
};

void joy_init()
{
}

void joy_close()
{
}

void joy_poll()
{
}


int is_statesave_available = 0;


void unlink_quickstate ()
{
	is_statesave_available = 0;
}


static int
set_frameskip_during_game (cont_cond_t *cont)
{
	uint8 x, y;
	uint8 rtrig;
	
	x = cont->joyx;
	y = cont->joyy;
	rtrig = cont->rtrig;
	
	if (y < 5) 
	{
		/* analog UP */
		interrupt_game_loop = 1;
		
		frameskip_rate = fsr_no_skip;
		return 1;
	} 
	else if (y > 250) 
	{
		/* analog DOWN */
		interrupt_game_loop = 1;
		
		frameskip_rate = fsr_double;
		return 1;
	}
	else if (x > 250) 
	{
		/* analog RIGHT */
		interrupt_game_loop = 1;
		
		frameskip_rate = fsr_auto;
		return 1;
	}
	else if (x < 5) 
	{
		/* analog LEFT */
		interrupt_game_loop = 1;
		
		if (rtrig) 
			frameskip_rate = fsr_max;
		else
			frameskip_rate = fsr_one_and_half;
		return 1;
	}
	
	return 0;
}


static void
draw_quick_menu ()
{
  dc_vid_clear ();
  
  draw_string (80,  30,   _white,  _black, "QUICK SAVE MENU");
  draw_string (100, 60,   _green,  _black, "Y: ");
  draw_string (10,  100,  _yellow, _black, "X: Quick save");
  draw_string (130, 100,  _blue,   _black, "B: Return to game");
  draw_string (100, 140,  _red,    _black, "A: Quick load");
}


static void 
do_quick_menu ()
{
  cont_cond_t cont;
  
  pcm_clear_buffer ();
  
  draw_quick_menu ();
  dc_vid_flip_fill_renderer (draw_type_fullscreen);
  
  wait_until_release_trigger (dc_controller_addr[0]);
  
  for (;;) 
  {
    cont_get_cond (dc_controller_addr[0], &cont);
    
    if (set_frameskip_during_game (&cont))
      vmu_draw_settings ();
    
    if (!(cont.buttons & CONT_B))
    {
      timer_sleep (100);
      break;
    }
    
    if (!(cont.buttons & CONT_X))
    {
      FILE *fp; 
      
      draw_quick_menu ();
      draw_string (100, 200, _white, _black, "save...");
      dc_vid_flip_fill_renderer (draw_type_fullscreen);
      
      fp = fopen ("/md/quick", "w");
      savestate (fp);
      fclose (fp);
      is_statesave_available = 1;
      
      draw_quick_menu ();
      draw_string (100, 200, _white, _black, "save...done");
      dc_vid_flip_fill_renderer (draw_type_fullscreen);
      
      timer_sleep(1000);
      break;
    }
    
    if (!(cont.buttons & CONT_A))
    {
      FILE *fp;
      
      draw_quick_menu ();
      draw_string (100, 200, _white, _black, "load...");
      dc_vid_flip_fill_renderer (draw_type_fullscreen);
      
      draw_quick_menu ();
      if (is_statesave_available)
      {
	fp = fopen ("/md/quick", "r");
	loadstate (fp);
	fclose (fp);
	
	draw_string (100, 200, _white, _black, "load...done");
      }
      else
      {
	draw_string (100, 200, _red, _black, "load...failed");
      }
      dc_vid_flip_fill_renderer (draw_type_fullscreen);
      
      timer_sleep(1000);
      break;
    }
    
    draw_quick_menu ();
    dc_vid_flip (draw_type_fullscreen);
  }
  
  dc_vid_empty ();
}


static void
draw_save_menu ()
{
  dc_vid_clear ();
  
  draw_string (80,  30,   _white,  _black, "VMU MENU");
  draw_string (100, 60,   _green,  _black, "Y: ");
  draw_string (10,  100,  _yellow, _black, "X: State save");
  draw_string (130, 100,  _blue,   _black, "B: Return to game");
  draw_string (100, 140,  _red,    _black, "A: State load");
}


static void
do_save_menu ()
{
  cont_cond_t cont;
  
  pcm_clear_buffer ();
  
  draw_save_menu ();
  dc_vid_flip_fill_renderer (draw_type_fullscreen);
  
  wait_until_release_trigger (dc_controller_addr[0]);
  
  for (;;) 
  {
    cont_get_cond (dc_controller_addr[0], &cont);
    
    if (set_frameskip_during_game (&cont))
      vmu_draw_settings ();
    
    if (!(cont.buttons & CONT_B))
    {
      timer_sleep(100);
      break;
    }
    
    if (!(cont.buttons & CONT_X))
    {
      draw_save_menu ();
      draw_string (100, 200, _white, _black, "save...");
      dc_vid_flip_fill_renderer (draw_type_fullscreen);
      
      draw_save_menu ();
      if (statesave_to_vmu ())
      {
	is_statesave_available = 1;
	draw_string (100, 200, _white, _black, "save...done");
      }
      else
	draw_string (100, 200, _red, _black, "save...failed");
      dc_vid_flip_fill_renderer (draw_type_fullscreen);
      
      timer_sleep(1000);
      break;
    }
    
    if (!(cont.buttons & CONT_A))
    {
      draw_save_menu ();
      draw_string (100, 200, _white, _black, "load...");
      dc_vid_flip_fill_renderer (draw_type_fullscreen);
      
      draw_save_menu ();
      if (stateload_from_vmu ())
      {
	is_statesave_available = 1;
	draw_string (100, 200, _white, _black, "load...done");
      }
      else
	draw_string (100, 200, _red, _black, "load...failed");
      dc_vid_flip_fill_renderer (draw_type_fullscreen);
      
      timer_sleep(1000);
      break;
    }
    
    draw_save_menu ();
    dc_vid_flip (draw_type_fullscreen);
  }
  
  dc_vid_empty ();
}


static void
draw_ingame_option_menu ()
{
  dc_vid_clear ();
  
  draw_string (100, 30,  _white,  _black, "OPTION MENU");
  draw_string (100, 60,  _green,  _black, "Y: Adjust screen");
  draw_string (130, 100, _blue,   _black, "B: Return to game");
}


static void
do_ingame_option_menu ()
{
  cont_cond_t cont;
  uint16 all_buttons = 
    CONT_Y | 
    CONT_X | 
    CONT_B | 
    CONT_A | 
    CONT_START;
  
  pcm_clear_buffer ();
  
  draw_ingame_option_menu ();
  dc_vid_flip_fill_renderer (draw_type_fullscreen);
  
  wait_until_release_trigger (dc_controller_addr[0]);
  wait_until_release_buttons (dc_controller_addr[0], all_buttons);
  
  for (;;) 
  {
    cont_get_cond (dc_controller_addr[0], &cont);
    
    if (set_frameskip_during_game (&cont))
      vmu_draw_settings ();
    
    if (!(cont.buttons & CONT_B))
    {
      timer_sleep (100);
      break;
    }
    
    if (!(cont.buttons & CONT_Y))
      do_adjust_screen_rect_menu ();
    
    draw_ingame_option_menu ();
    dc_vid_flip (draw_type_fullscreen);
    
    if (is_buttons_press (&cont, all_buttons) || cont.ltrig)
    {
      dc_vid_flip_fill_renderer (draw_type_fullscreen);
      wait_until_release_buttons (dc_controller_addr[0], all_buttons);
      wait_until_release_trigger (dc_controller_addr[0]);
    }
  }
  
  dc_vid_empty ();
}





void ev_poll()
{
  cont_cond_t cont;
  
  cont_get_cond (dc_controller_addr[0], &cont);
  
  set_frameskip_during_game (&cont);
  
  if (cont.rtrig) 
  {
    if (cont.ltrig && !(cont.buttons & CONT_START))
    {
      exit_game_loop = 1;
      interrupt_game_loop = 1;
      return;
    }
    else if (!(cont.buttons & CONT_B))
    {
      do_quick_menu();
      interrupt_game_loop = 1;
      clear_rest_frames ();
      return;
    }
    else if (!(cont.buttons & CONT_Y))
    {
      do_save_menu ();
      interrupt_game_loop = 1;
      clear_rest_frames ();
      return;
    }
    else if (!(cont.buttons & CONT_A))
    {
      do_ingame_option_menu ();
      interrupt_game_loop = 1;
      clear_rest_frames ();
      return;
    }
  }
  
#define SET_PAD(DC, GB) \
  { \
    if (cont.buttons & DC) \
      pad_release (GB); \
    else \
      pad_press (GB); \
  }
  
  /* start */
  SET_PAD(CONT_START,           PAD_START);
  SET_PAD(CONT_Y,               PAD_SELECT);
  SET_PAD(CONT_X,               PAD_B);
  SET_PAD(CONT_A,               PAD_A);
  SET_PAD(CONT_DPAD_UP,         PAD_UP);
  SET_PAD(CONT_DPAD_DOWN,       PAD_DOWN);
  SET_PAD(CONT_DPAD_LEFT,       PAD_LEFT);
  SET_PAD(CONT_DPAD_RIGHT,      PAD_RIGHT);
}  



