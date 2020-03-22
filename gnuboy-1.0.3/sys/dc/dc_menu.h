#ifndef _DC_MENU_H_
#define _DC_MENU_H_

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

#if 0
}
#endif

int init_menus ();

/* ---------------------------------------- */
typedef struct {
  uint8 vertical_wait;
  uint8 horizontal_wait;
  const char *root_path;
} fm_config_t;


typedef struct {
  char path[256];
} fm_result_t;


int do_file_menu (fm_result_t *result, const char *path, fm_config_t *config);
/* ---------------------------------------- */

void do_vmu_menu (uint8 addr);
uint8 do_vmu_select_menu ();
void do_adjust_screen_rect_menu (); 


#ifdef __cplusplus
}
#endif

#endif


