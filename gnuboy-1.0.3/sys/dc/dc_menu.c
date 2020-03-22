#include "dc_menu.h"
#include "dc_utils.h"
#include "dc_vmu.h"

#define DC_MENU_PICS_PATH "/cd"


/* ============================================================ */
static int init_file_menu ();
static int init_vmu_menu ();
static int init_vmu_select_menu ();

int
init_menus ()
{
  init_file_menu ();
  init_vmu_menu ();
  init_vmu_select_menu ();
  
  return 1;
}


/* ============================================================ */
static uint16 fm_image[320 * 240];

typedef enum {
  fm_type_none,
  fm_type_dir,
  fm_type_file,
} fm_type_t;

#define FM_PATH_LEN 256
typedef struct {
  char path[FM_PATH_LEN];
  fm_type_t type;
} fm_files_t;

typedef struct {
  /* cursor pos */
  int base_index; /* 0 .. num_files */
  int cur_row;    /* 0 .. DRAW_LINES_DEFAULT */
  
  char path[FM_PATH_LEN];
  int num_files;
} fm_dir_status_t;
#define FM_DIRSTAT_MAX 256
static fm_dir_status_t fm_dirstat[FM_DIRSTAT_MAX];
static int fm_dirstat_size = 0;


static void
fm_init_dir_status (fm_dir_status_t *stat, const char *path)
{
  stat->base_index = 0;
  stat->cur_row = 0;
  
  sprintf (stat->path, "%s", path); /* FIX ME: to use snprintf */
  stat->num_files = 0;
}


static fm_dir_status_t *
fm_find_dir_status (const char *path)
{
  int i;
  
  for (i = 0; i < fm_dirstat_size; ++i)
  {
    if (!strcmp(path, fm_dirstat[i].path))
      return fm_dirstat + i;
  }
  
  /* create new */
  if (i == fm_dirstat_size) 
  {
    if (fm_dirstat_size == FM_DIRSTAT_MAX) 
      i = 0;
    else
      ++fm_dirstat_size;
  }
  fm_init_dir_status (fm_dirstat + i, path);
  return fm_dirstat + i;
}


static int
init_file_menu ()
{
  load_bmp (fm_image, DC_MENU_PICS_PATH "/pics/menu_selection.bmp");
  fm_dirstat_size = 0;
  return 1;
}


static int
fm_read_index (const char *path, fm_files_t *fm_files, int fm_files_size)
{
  int i;
  int num_files;
  file_t d; 
  dirent_t *de;
  
  d = fs_open(path, O_RDONLY | O_DIR);
  if (!d)
    return -1;
  
  num_files = 0;
  de = fs_readdir (d);
  for (i = 0; i < fm_files_size; ++i)
  {
    if (de)
    {
      /* FIX ME! to use snprintf */ 
      sprintf (fm_files[i].path, "%s/%s", path, de->name); 
      fm_files[i].type = de->size == -1 ? fm_type_dir : fm_type_file;
      
      ++num_files;
      
      de = fs_readdir (d);
    }
    else
      fm_files[i].type = fm_type_none;
  }
  fs_close (d);
  
  return num_files;
}


#define FM_DRAW_LINES 20

static void
fm_draw_files (const fm_dir_status_t *stat, const fm_files_t *fm_files, 
               const fm_config_t *config) 
{
  int i;
  int y_pos = 20;
  int x_pos = 75;
  int draw_index;
  int ndraw;
  char str[128];
  
  if (stat->num_files < stat->base_index + FM_DRAW_LINES)
    ndraw = stat->num_files - stat->base_index;
  else
    ndraw = FM_DRAW_LINES;
  
  display_rawimage (fm_image);
  draw_string (10, 10, _white, _none, stat->path);
  
  draw_index = stat->base_index;
  for (i = 0; i < ndraw; ++i)
  {
    uint16 fg_color = i == stat->cur_row ? _yellow : _white;
    switch (fm_files[draw_index].type) 
    {
      case fm_type_file:
        sprintf (str, "%s",  strrchr(fm_files[draw_index].path, '/') + 1);
        break;
        
      case fm_type_dir:
        sprintf (str, "%s/", strrchr(fm_files[draw_index].path, '/') + 1);
        break;
        
      default:
        *str = '\0';
        break;
    }
    
    draw_string (x_pos, y_pos, fg_color, _none, str);
    ++draw_index;
    y_pos += 10;
  }
  
  sprintf (str, "wait v%.3d/h%.3d", config->vertical_wait, config->horizontal_wait);
  draw_string (200, 220, _white, _none, str);
}


fm_type_t
fm_do_body(fm_result_t *result, const char *path, fm_config_t *config)
{
#define FM_MAX_FILES 1024
  fm_files_t fm_files[FM_MAX_FILES];
  int i;
  int new_num_files;
  fm_dir_status_t *stat;
  
  cont_cond_t cont;
  uint16 all_buttons = CONT_A | CONT_Y;
  int wait_time;
  
  new_num_files = fm_read_index (path, fm_files, FM_MAX_FILES);
  if (new_num_files == -1)
  {
    /* FIX ME: to use snprintf */
    sprintf (result->path, "%s", config->root_path); 
    return fm_type_dir;
  }
  
  stat = fm_find_dir_status (path);
  if (new_num_files != stat->num_files)
    fm_init_dir_status (stat, path);
  stat->num_files = new_num_files;
  
  fm_draw_files (stat, fm_files, config);
  dc_vid_flip_fill_renderer (draw_type_fullscreen);
  wait_until_release_buttons (dc_controller_addr[0], all_buttons);
  
  for (;;) 
  {
    cont_get_cond (dc_controller_addr[0], &cont);
    
    wait_time = 0;
    
    if (!(cont.buttons & CONT_DPAD_UP))
    {
      if (stat->cur_row > 0)
	--stat->cur_row;
      else
      {
        if (stat->base_index > 0)
          --stat->base_index;
      }
      if (cont.buttons & CONT_X)
        wait_time = config->vertical_wait;
    }
    
    if (!(cont.buttons & CONT_DPAD_DOWN)) 
    {
      if (stat->cur_row < FM_DRAW_LINES - 1)
      {
        int cur_index = stat->base_index + stat->cur_row;
        
        if (cur_index < stat->num_files - 1)
          ++stat->cur_row;
      }
      else
      {
        if (stat->base_index + FM_DRAW_LINES < stat->num_files)
          ++stat->base_index;
      }
      if (cont.buttons & CONT_X)
        wait_time = config->vertical_wait;
    }
    
    if (!(cont.buttons & CONT_DPAD_LEFT)) 
    {
      for (i = 0; i < FM_DRAW_LINES; ++i)
      {
        if (stat->base_index > 0)
          --stat->base_index;
      }
      if (cont.buttons & CONT_X)
        wait_time = config->horizontal_wait;
    }
    
    if (!(cont.buttons & CONT_DPAD_RIGHT)) 
    {
      for (i = 0; i < FM_DRAW_LINES; ++i)
      {
        if (stat->base_index + FM_DRAW_LINES < stat->num_files)
          ++stat->base_index;
      }
      if (cont.buttons & CONT_X)
        wait_time = config->horizontal_wait;
    }
    
    if (!(cont.buttons & CONT_START)) 
    {
      int cur_index = stat->base_index + stat->cur_row;
      if (fm_files[cur_index].type == fm_type_file)
      {
        /* FIX ME: to use snprintf */
        sprintf (result->path, "%s", fm_files[cur_index].path); 
        return fm_type_file;
      }
    }
    
    if (!(cont.buttons & CONT_A))
    {
      int cur_index = stat->base_index + stat->cur_row;
      if (fm_files[cur_index].type == fm_type_dir)
      {
        /* FIX ME: to use snprintf */
        sprintf (result->path, "%s", fm_files[cur_index].path);
        return fm_type_dir;
      }
    }
    
    if (!(cont.buttons & CONT_Y))
    {
      /* FIX ME: to use snprintf */
      sprintf (result->path, "%s", path);
      
      if (strcmp(result->path, config->root_path))
      {
        char *p = strrchr(result->path, '/');
        if (p) *p = '\0';
      }
      
      return fm_type_dir;
    }
    
    if (cont.rtrig  & cont.ltrig) 
      return fm_type_none;
    
    /* analog up */
    if (cont.joyy < 5)
    {
      if (config->vertical_wait > 0)
        --config->vertical_wait;
    }
    /* analog down */
    if (cont.joyy > 250)
    {
      if (config->vertical_wait < 200)
        ++config->vertical_wait;
    }
    /* analog right */
    if (cont.joyx > 250) 
    {
      if (config->horizontal_wait < 200)
        ++config->horizontal_wait;
    }
    /* analog left */
    if (cont.joyx < 5)
    {
      if (config->horizontal_wait > 0)
        --config->horizontal_wait;
    }
    
    fm_draw_files (stat, fm_files, config);
    if (wait_time)
    {
      dc_vid_flip_fill_renderer (draw_type_fullscreen);
      timer_spin_sleep (wait_time);
    }
    else
      dc_vid_flip (draw_type_fullscreen);
  }
  
  dc_vid_clear ();
  dc_vid_flip_fill_renderer (draw_type_fullscreen);
}


int
do_file_menu (fm_result_t *result, const char *path, fm_config_t *config)
{
  fm_type_t type;
  char new_path[256];
  
  iso_ioctl (0, NULL, 0);
  
  display_rawimage (fm_image);
  draw_string (75, 75, _white, _none, "Now loading");
  dc_vid_flip_fill_renderer (draw_type_fullscreen);
  
  /* FIX ME! to use snprintf */
  sprintf (new_path, "%s", path);
  
  for (;;)
  {
    type = fm_do_body (result, new_path, config);
    
    switch (type) 
    {
      case fm_type_none:
        return 0;
        
      case fm_type_file:
        return 1;
        
      case fm_type_dir:
        /* FIX ME! to use snprintf */
        sprintf (new_path, "%s", result->path);
        break;
        
      default:
        return 0;
    }
  }
}


/* ============================================================ */
typedef struct {
  char desc_long[64];
  char filename[32];
  uint8 filesize;
  uint8 filetype;
  boolean selected;
} vmu_item;


typedef struct {
  uint8 addr;
  int free_blocks;
  int use_blocks;
  int file_num;
} vmu_info;


#define MAX_VMU_FILES 256
static vmu_item vmu_files[MAX_VMU_FILES];

const int draw_vmu_files_line = 15;

static uint16 vmu_menu_image[320 * 240];

static int 
init_vmu_menu ()
{
  load_bmp (vmu_menu_image, DC_MENU_PICS_PATH "/pics/vmu_menu.bmp");
  return 1;
}


static void
draw_vmu_menu_help ()
{
  display_rawimage (vmu_menu_image);
  draw_string (80, 30, _white, _none, "help");
  draw_string (60, 60, _white, _none, "L: select file");
  draw_string (60, 70, _white, _none, "R+Y: remove selected files");
  draw_string (60, 80, _white, _none, "L+R: exit VMU menu");
  draw_string (80, 200, _white, _none, "start: exit help");
}


static void
do_vmu_menu_help ()
{
  cont_cond_t cont;
  
  draw_vmu_menu_help ();
  dc_vid_flip_fill_renderer (draw_type_fullscreen);
  
  wait_until_release_buttons (dc_controller_addr[0], CONT_START);
  
  for (;;)
  {
    cont_get_cond (dc_controller_addr[0], &cont);
    
    if (!(cont.buttons & CONT_START))
      break;
  }
}


static void
draw_vmu_files (int base_pos, int cur_pos, vmu_info *vi)
{
  int nline;
  char str[128];
  int i;
  int x_pos = 10;
  int y_pos;
  uint16 fg_color, bg_color;
  
  display_rawimage (vmu_menu_image);
  
  if (MAX_VMU_FILES - base_pos > draw_vmu_files_line)
    nline = draw_vmu_files_line;
  else
    nline = MAX_VMU_FILES - base_pos;
  
  draw_string (10, 30, _white, _none, "press L+R to exit/press start to help");
  sprintf (str, "%d free/%d ndc use", vi->free_blocks, vi->use_blocks);
  draw_string (80, 40, _white, _none, str);
  
  y_pos = 60;
  for (i = base_pos; i < base_pos + nline; ++i)
  {
    if (vmu_files[i].filetype != 0x00)
    {
      fg_color = cur_pos == i ? _yellow : _white;
      bg_color = vmu_files[i].selected ? _blue : _none;
      
      sprintf(str, "%s(%d)", vmu_files[i].desc_long, vmu_files[i].filesize);
      draw_string (x_pos, y_pos, fg_color, bg_color, str);
      y_pos += 10;
    }
  }
}


extern const char *progname;


static int
read_vmu_items (vmu_info *vi, uint8 addr)
{
  int num_entries;
  dirent_vmu entries[MAX_VMU_FILES];
  int num_vmu_files;
  int i;
  
  num_entries = MAX_VMU_FILES;
  if (ndc_vmu_getall_dirent (entries, &num_entries, addr) < 0)
    return -1;
  
  memset (vmu_files, 0, sizeof(vmu_files));
  num_vmu_files = 0;
  for (i = 0; i < num_entries; ++i)
  {
    if (entries[i].filetype != 0x00)
    {
      char *p;
      uint8 buf[512];
      file_hdr_vmu *hdr = (file_hdr_vmu *)buf;
      
      if (ndc_vmu_read (addr, entries[i].firstblk, buf) < 0)
        continue;
      
      if (strcmp(hdr->app_id, progname))
        continue;
      
      p = vmu_files[num_vmu_files].desc_long;
      strncpy (p, hdr->desc_long, 32);
      p[32] = '\0';
      
      p = vmu_files[num_vmu_files].filename;
      strncpy (p, entries[i].filename, 12);
      p[12] = '\0';
      
      vmu_files[num_vmu_files].filesize = entries[i].filesize;
      vmu_files[num_vmu_files].filetype = entries[i].filetype;
      vmu_files[num_vmu_files].selected = 0;
      
      ++num_vmu_files;
    }
  }
  
  vi->free_blocks = ndc_vmu_check_free_blocks (NULL, addr);
  vi->use_blocks = 0;
  vi->file_num = num_vmu_files;
  for (i = 0; i < num_vmu_files; ++i)
    vi->use_blocks += vmu_files[i].filesize;
  
  return 1;
}


void
do_vmu_menu (uint8 addr)
{
  vmu_info vi;
  int base_pos = 0;
  int cur_pos = 0;
  cont_cond_t cont;
  boolean pulling_ltrig = 0;
  boolean select_mode = 0;
  int wait_time;
  
  display_rawimage (vmu_menu_image);
  draw_string (75, 75, _white, _none, "reading VMU");
  dc_vid_flip_fill_renderer (draw_type_fullscreen);
  
  if (read_vmu_items (&vi, addr) < 0)
    return;
  
  for (;;)
  {
    cont_get_cond (dc_controller_addr[0], &cont);
    
    wait_time = 0;
    
    if (!(cont.buttons & CONT_DPAD_UP))
    {
      if (cur_pos > 0)
      {
        if (cur_pos == base_pos)
          --base_pos;
        --cur_pos;
      }
      if (pulling_ltrig)
        vmu_files[cur_pos].selected = select_mode;
      
      if (cont.buttons & CONT_X)
        wait_time = 80;
    }
    
    if (!(cont.buttons & CONT_DPAD_DOWN)) 
    {
      if (cur_pos < vi.file_num - 1)
      {
        if (cur_pos == base_pos + draw_vmu_files_line - 1)
          ++base_pos;
        ++cur_pos;
      }
      if (pulling_ltrig)
        vmu_files[cur_pos].selected = select_mode;

      if (cont.buttons & CONT_X)
        wait_time = 80;
    }
    
    if (!(cont.buttons & CONT_Y) && cont.rtrig)
    {
      int i;
      for (i = 0; i < vi.file_num; ++i)
      {
        if (vmu_files[i].selected)
        {
          char str[128];
          sprintf(str, "remove %s", vmu_files[i].desc_long);
          dc_print (str);
          ndc_vmu_remove_file (addr, vmu_files[i].filename);
        }
      }
      break;
    }
    
    if (!(cont.buttons & CONT_START))
    {
      do_vmu_menu_help ();
      
      draw_vmu_files (base_pos, cur_pos, &vi);
      dc_vid_flip_fill_renderer (draw_type_fullscreen);
      
      wait_until_release_buttons (dc_controller_addr[0], CONT_START);
    }
    
    if (!(cont.ltrig))
      pulling_ltrig = 0;
    else
    {
      if (cont.rtrig)
        break;
      else
      {
        if (!pulling_ltrig)
        {
          pulling_ltrig = 1;
          vmu_files[cur_pos].selected = !(vmu_files[cur_pos].selected);
          select_mode = vmu_files[cur_pos].selected;
        }
      }
    }
    
    draw_vmu_files (base_pos, cur_pos, &vi);
    if (wait_time) 
    {
      dc_vid_flip_fill_renderer (draw_type_fullscreen);
      timer_spin_sleep (wait_time);
    }
    else
      dc_vid_flip (draw_type_fullscreen);
  }
  
  dc_vid_clear ();
  dc_vid_flip_fill_renderer (draw_type_fullscreen);
}


/* ============================================================ */
struct {
  char name[32];
  uint8 addr;
  uint8 free_blocks;
} vmu_slots[32];

static uint16 vmu_select_menu_image[320 * 240];

static int
init_vmu_select_menu ()
{
  load_bmp (vmu_select_menu_image, DC_MENU_PICS_PATH "/pics/vmu_menu.bmp");
  return 1;
}


static void
draw_vmu_select_slots (int cur_pos, int vmu_num)
{
  int i;
  int y_pos = 40;
  int x_pos = 75;
  char str[128];
  
  display_rawimage (vmu_select_menu_image);
  
  draw_string (100, 10, _white, _none, "VMU select");
  draw_string (100, 20, _white, _none, "press A to select");
  for (i = 0; i < vmu_num; ++i)
  {
    uint16 fg_color = cur_pos == i ? _yellow : _white;
    sprintf (str, "%s (%d free blocks)", vmu_slots[i].name, vmu_slots[i].free_blocks);
    draw_string (x_pos, y_pos, fg_color, _none, str);
    y_pos += 10;
  }
}


uint8 
do_vmu_select_menu ()
{
  int port, unit;
  int vmu_num;
  int cur_pos = 0;
  
  dc_vid_clear ();
  dc_vid_flip_fill_renderer (draw_type_fullscreen);
  
  vmu_num = 0;
  for (port=0; port<4; port++)
  {
    for (unit=0; unit<6; unit++) 
    {
      if (maple_device_func(port, unit) & MAPLE_FUNC_MEMCARD)
      {
        uint8 addr = maple_create_addr(port, unit);
        
        sprintf(vmu_slots[vmu_num].name, "p%d/u%d", port, unit);
        vmu_slots[vmu_num].addr = addr; 
        vmu_slots[vmu_num].free_blocks = ndc_vmu_check_free_blocks (NULL, addr);
        ++vmu_num;
      }
    }
  }
  
  if (!vmu_num)
    return 0;
  
  draw_vmu_select_slots (cur_pos, vmu_num);
  dc_vid_flip_fill_renderer (draw_type_fullscreen);
  for (;;)
  {
    cont_cond_t cont;
    int update = 0;
    
    cont_get_cond (dc_controller_addr[0], &cont);
    
    if (!(cont.buttons & CONT_DPAD_UP))
    {
      if (cur_pos > 0)
	--cur_pos;
      update = 1;
    }
    else if (!(cont.buttons & CONT_DPAD_DOWN)) 
    {
      if (cur_pos < vmu_num - 1)
	++cur_pos;
      update = 1;
    }
    else if (!(cont.buttons & CONT_A))
      break;
    
    if (update)
    {
      draw_vmu_select_slots (cur_pos, vmu_num);
      dc_vid_flip_fill_renderer (draw_type_fullscreen);
      wait_until_release_buttons (dc_controller_addr[0], 
                                  CONT_DPAD_UP | CONT_DPAD_DOWN);
    }
  }
  
  dc_vid_clear ();
  dc_vid_flip_fill_renderer (draw_type_fullscreen);
  
  return vmu_slots[cur_pos].addr;
}


/* ============================================================ */
void
draw_adjust_screen_rect_menu ()
{
  uint16 *screen;
  int x, y;
  
  screen = dc_get_screen ();
  for (y = 0; y < 8; ++y)
    for (x = 0; x < 320; ++x)
      *screen++ = _black;
  
  for (y = 8; y < 16; ++y)
    for (x = 0; x < 320; ++x)
      *screen++ = _red;
  
  for (y = 16; y < 224; ++y)
  {
    for (x = 0; x < 8; ++x)
      *screen++ = _red;
    
    for (x = 8; x < 312; ++x)
      *screen++ = _black;
    
    for (x = 312; x < 320; ++x)
      *screen++ = _red;
  }
  
  for (y = 224; y < 232; ++y)
    for (x = 0; x < 320; ++x)
      *screen++ = _red;
  
  for (y = 232; y < 240; ++y)
    for (x = 0; x < 320; ++x)
      *screen++ = _black;
  
  draw_string (80, 50, _white, _black, "Screen adjust menu");
  
  draw_string (100, 60, _white, _black, "L + R: exit");
  
  draw_string (80,  80, _white, _black, "up/down/left/right:");
  draw_string (100, 90, _white, _black, "adjust position");
  
  draw_string (80,  110, _white, _black, "A + up/down/left/right:");
  draw_string (100, 120, _white, _black, "adjust size");
  
  draw_string (80, 140, _white, _black, "Y: set ratio to 4:3");
  draw_string (80, 150, _white, _black, "R + Y: reset setting");
  draw_string (80, 160, _white, _black, "R + B: set default");
  
  draw_string (80, 180, _white, _black, "X: speedup");
}


void
do_adjust_screen_rect_menu ()
{
  cont_cond_t cont;
  screen_rect new_rect;
  screen_rect orig_rect;
  
  draw_adjust_screen_rect_menu ();
  dc_vid_flip_fill_renderer (draw_type_fullscreen);
  wait_until_release_buttons (dc_controller_addr[0], CONT_Y);
  
  dc_get_screen_rect (&new_rect);
  dc_get_screen_rect (&orig_rect);
  for (;;)
  {
    int adjust_size;
    
    cont_get_cond (dc_controller_addr[0], &cont);
    
    if (cont.rtrig & cont.ltrig) 
      break;
    
    if (!(cont.buttons & CONT_X))
      adjust_size = 2;
    else
      adjust_size = 1;
    
    if (!(cont.buttons & CONT_DPAD_UP))
    {
      if (cont.buttons & CONT_A)
      {
        new_rect.y1 -= adjust_size;
        new_rect.y2 -= adjust_size;
      }
      else
      {
        new_rect.y1 -= adjust_size;
        new_rect.y2 += adjust_size;
      }
      dc_set_screen_rect (&new_rect);
    }
    
    if (!(cont.buttons & CONT_DPAD_DOWN))
    {
      if (cont.buttons & CONT_A)
      {
        new_rect.y1 += adjust_size;
        new_rect.y2 += adjust_size;
      }
      else
      {
        new_rect.y1 += adjust_size;
        new_rect.y2 -= adjust_size;
      }
      dc_set_screen_rect (&new_rect);
    }
    
    if (!(cont.buttons & CONT_DPAD_LEFT))
    {
      if (cont.buttons & CONT_A)
      {
        new_rect.x1 -= adjust_size;
        new_rect.x2 -= adjust_size;
      }
      else
      {
        new_rect.x1 -= adjust_size;
        new_rect.x2 += adjust_size;
      }
      dc_set_screen_rect (&new_rect);
    }
    
    if (!(cont.buttons & CONT_DPAD_RIGHT))
    {
      if (cont.buttons & CONT_A)
      {
        new_rect.x1 += adjust_size;
        new_rect.x2 += adjust_size;
      }
      else
      {
        new_rect.x1 += adjust_size;
        new_rect.x2 -= adjust_size;
      }
      dc_set_screen_rect (&new_rect);
    }
    
    if (!(cont.buttons & CONT_Y))
    {
      if (cont.rtrig)
      {
        new_rect = orig_rect;
        dc_set_screen_rect (&new_rect);
      }
      else
      {
        int yw = new_rect.y2 - new_rect.y1;
        int xw = new_rect.x2 - new_rect.x1;
        int new_xw = yw * 4 / 3;
        
        if (xw > new_xw)
        {
          new_rect.x1 += (xw - new_xw) / 2;
          new_rect.x2 -= (xw - new_xw) / 2;
        }
        else
        {
          new_rect.x1 -= (new_xw - xw) / 2;
          new_rect.x2 += (new_xw - xw) / 2;
        }
        dc_set_screen_rect (&new_rect);
      }
    }
    
    if (!(cont.buttons & CONT_B) && cont.rtrig)
    {
      dc_set_default_screen_rect ();
      dc_get_screen_rect (&new_rect);
    }
    
    dc_vid_flip (draw_type_fullscreen);
  }  
}


