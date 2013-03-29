/* keys.h by lutherus 2012 */

#ifndef __KEYS_H__

#define GET_NEW_KEY -1
#define MAX_CMD_SIZE 100
#define CTRL_KEYS 25
#define TAG_DN 1
#define TAG_UP 2
#define SHORTCUT_L 20

#define KEY_PERCENT 37


/* struct k_scut {{{ */
struct _k_scut
{
  int value;
  char buf[SHORTCUT_L];

} k_scut; /* }}} */

/* struct visual {{{ */
struct _visual
{
  int line;
  int start_line;

} visual; /* }}} */

/* delay_mode {{{ */
struct _delay
{
  int status;

} delay; /* }}} */

/* struct cmd_input {{{ */
struct _cmd_line
{
  char buf[MAX_CMD_SIZE];

  int mode;
  int count;
  int cur_pos;

} cmd_line; /* }}} */

/* prototypes {{{ */
int check_key(Tab **curr_tab, cfg_t **cfg, int key, int x, int y);
void marker_goes_top(ListView *view);
void marker_goes_tow(ListView *view);
void mark_goes_cow(ListView *view);
void marker_goes_bot(ListView *view);
void marker_goes_to_lfiw(ListView *view);
////////////////////////////////////////////////
int filter_keystroke(Tab **curr_tab, cfg_t **cfg, int x, int y, int key);
int ret_ui_cmd(keys_t **user_k, int idx);
int check_scase(keys_t **user_k, int idx, int key);
int check_shortcut(keys_t **user_k, int idx, int key);
int check_valid_shortcut(keys_t **user_k, int user_k_size, int key);
int show_cmd_input(int key, int x, int y, WINDOW *win);
int ret_percent_in_list(ListView *view, int vm_count);
void show_error_message(cfg_t *cfg, char *error);
void switch_home(ListView *view, cfg_t **cfg);
//////////////////////////////////////////////////////
void kill_cmd_mode(WINDOW *win);
void del_cmd_character(int key, int y, WINDOW *win);
void visual_tagging(ListView *view, int tag_m, int old_ptr);
void visual_line_mode(ListView *view, cfg_t **cfg, int x, int y); /* }}} */

#endif
