#include "gnuboy.h"

#include "fb.h"
#include "defs.h"
#include "regs.h"
#include "hw.h"
#include "cpu.h"
#include "mem.h"
#include "lcd.h"
#include "rc.h"


static int framelen = 16743;
static int framecount;

rcvar_t emu_exports[] =
{
	RCV_INT("framelen", &framelen),
	RCV_INT("framecount", &framecount),
	RCV_END
};







void emu_init()
{
	
}


/*
 * emu_reset is called to initialize the state of the emulated
 * system. It should set cpu registers, hardware registers, etc. to
 * their appropriate values at powerup time.
 */

void emu_reset()
{
	hw_reset();
	lcd_reset();
	cpu_reset();
	mbc_reset();
	sound_reset();
}





void emu_step()
{
	cpu_emulate(cpu.lcdc);
}



/* This mess needs to be moved to another module; it's just here to
 * make things work in the mean time. */

int exit_game_loop = 0;
int interrupt_game_loop = 0;


void emu_run()
{
	void *timer = sys_timer();
	int delay;
	
	sys_initall ();
	vid_begin();
	lcd_begin();
        
	exit_game_loop = 0;
	while (!exit_game_loop)
	{
		interrupt_game_loop = 0;
		sys_set_timer ();
		vmu_draw_settings ();
		
		while (!interrupt_game_loop)
		{
			cpu_emulate(2280);
			while (R_LY > 0 && R_LY < 144)
				emu_step();
			
			vid_end();
			rtc_tick();
#if 1
			render_pcm ();
#endif

			doevents();

			vid_begin();
			
			if (!(R_LCDC & 0x80))
				cpu_emulate(32832);
			
			while (R_LY > 0) /* wait for next frame */
				emu_step();
		}
		
		sys_terminate_timer();
        }
}

