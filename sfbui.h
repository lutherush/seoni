/* sfbgui.h by lutherus 2012 */

//#include "filedata.h"

#ifndef __SFBUI_H__
#define __SFBUI_H__

/* macros {{{ */
#define MAX_FILELEN 255
#define MAX_HIST 200
#define PTRH 3000
#define UPDATE 1
#define DONE 0
#define DEEP_ON 1
#define DEEP_OFF 0

#define LWIN 0
#define RWIN 1

#define M_TIME_POS 16
#define F_SIZE_POS 10
#define CENTER_Y(y) ((y-2) >> 1)
#define CENTER_X(x, x1) (x >> x1)
/* }}} */

/* panel_data {{{ */
struct _panel
{
  int bot_filter_show_len;

} panel; /* }}} */

/* Dir {{{ */
typedef struct __Dir
{
  char *path;
  char *filename;

  int pos;
  int scrolled;

  struct __Dir *next_dir;
  struct __Dir *head;

} Dir; /* }}} */

/* dir_history {{{ */
typedef struct __Dir_History
{
  Dir *dir;

} Dir_History; /* }}} */

/* typedefs {{{ */
// c = count, f = flag
typedef struct _ListView
{
  WINDOW *win;

  Dir_Entry **dir_entry;
  Dir_History dir_hist[100];

  int entry_buffer_size;

  char *path;
  int path_size;

  int ptr_c;
  int ptr_pos_c;
  int scroll_c;
  int page_scroll_c;
  int curr_page_sum;
  int file_filter;
  int sortmode;
  int sortoption;

  int update_list;

} ListView; /* }}} */

/* curses elem {{{ */
WINDOW *st_win, *st2_win; /* }}} */

/* prototypes {{{ */

// init
void init_state_win(int y, int x);
ListView *append_view(WINDOW *win, cfg_t **cfg, char *path);
ListView *prepend_view(ListView *query, cfg_t **cfg, char *path);

// motion
void update_ptr_up(ListView *view);
int update_ptr_down(ListView *view);
int update_page_dn(ListView *view, int scroll_step);
int update_page_up(ListView *view, int scroll_step);

// directory movment
void directory_up(ListView *view, cfg_t **cfg);
int directory_dn(ListView *view, cfg_t **cfg, int y, int x);
int change_directory(ListView *view, cfg_t **cfg, char *path);
void update_files(ListView *view, cfg_t **cfg, char *file);

// limitations
void check_limits(ListView *view);

// toggles
void toggle_file_filter(ListView *view, cfg_t **cfg, int y);
void toggle_sortfiles(ListView *view, cfg_t **cfg);

// tags
void tag_file(ListView *view, int y);

// output listviews
void show_list(ListView *view, Color *color, int master);
void set_file_color(ListView *view, Color *color, int file_c);
void show_list_attrib(ListView *view, int file_c, int wd, int curr_win, int x);
void show_name_in_list(ListView *view, Color *color, int c, int curr_win, int x, int master);
void show_fsize_in_list(ListView *view, Color *color, int c, int curr_win, int x);
void show_last_hbar_pos(ListView *view, Color *color, int master);
void print_borders(ListView *view, int master);

// output st2_win
void show_filtered_files(int num_files_f, int x, WINDOW *win);
void show_file_bar(ListView *view, int y, int x, WINDOW *win, int deep_scan);
int show_file_properties(ListView *view, int sel_c, char *size, WINDOW *win);
//void show_full_file_properties(ListView *view, int x, int y, int sel_c, double fs, WINDOW *win);
int show_list_state(ListView * view, int y, int x, WINDOW *win);
void show_shortcut_state(ListView *view, int max_outp, int x, WINDOW *win);
void show_cmd_bar(ListView *view, cfg_t **cfg, int x, int y, WINDOW *win, char *error);

// reload
void reload(ListView *view, cfg_t **cfg);
/* }}} */

#endif
