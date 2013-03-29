/* keys.c by lutherus 2012 */

#include <ctype.h>
#include "seoni.h"
#include "filedata.h"
#include "config.h"
#include "sfbui.h"
#include "tab.h"
#include "utils.h"
#include "keys.h"
#include <unistd.h>

/* ui_cmd {{{ */
char *ui_cmd[] =
{
  "dummy",
  "marker_up",
  "marker_dn",
  "marker_top",
  "marker_bot",
  "marker_top_win",
  "marker_bot_win",
  "marker_center_win",
  "scroll_page_up",
  "scroll_page_dn",
  "scroll_half_page_up",
  "scroll_half_page_dn",
  "prev_tab",
  "next_tab",
  "open_in_new_tab",
  "delete_tab",
  "open_in_hsplit",
  "close_view",
  "visual_mode",
  "normal_mode",
  "cmd_mode",
  "tag_file",
  "toggle_filefilter",
  "toggle_sort",
  "toggle_win",
  "dir_dn",
  "dir_up",
  "redraw",
  "home_switch"
}; /* }}} */

/* marker_goes_top {{{ */
void
marker_goes_top(ListView *view)
{
  int old_ptr = view->ptr_c;

  view->ptr_c = 0;      // total position
  view->scroll_c = 0;   // how many lines are scrolled
  view->ptr_pos_c = 0;  // position in window

  if(visual.line == 1)
    visual_tagging(view, TAG_UP, old_ptr);
  
  view->update_list = UPDATE;
} /* }}} */

/* marker_goes_tow {{{ */
void
marker_goes_tow(ListView *view)
{
  int old_ptr = view->ptr_c;
  int pos_buf = view->ptr_pos_c;

  if(view->ptr_pos_c != 0)
  {
    view->ptr_pos_c = 0;
    view->ptr_c -= pos_buf;
  }
  if(visual.line == 1)
    visual_tagging(view, TAG_UP, old_ptr);
 
  view->update_list = UPDATE;
} /* }}} */

/* marker_goes_cow (center of window) {{{ */
void
marker_goes_cow(ListView *view)
{
  int y = getmaxy(view->win);
  int tag = TAG_UP;
  int old_ptr = view->ptr_c;
  int center_win;

  int file_count = 
    view->dir_entry[view->ptr_c]->Info->file_count;

  if(y < file_count) center_win = (y / 2);
  else center_win = (file_count / 2);

  view->ptr_c 
    = (view->scroll_c + center_win);

  view->ptr_pos_c = (view->ptr_c - view->scroll_c);

  if(visual.line == 1)
  {
    if(view->ptr_c > old_ptr) tag = TAG_DN;
    visual_tagging(view, tag, old_ptr);
  }
  view->update_list = UPDATE;
} /* }}} */

/* marker_goes_bottom {{{ */
void
marker_goes_bot(ListView *view)
{
  int y = getmaxy(view->win);
  int old_ptr = view->ptr_c;

  int file_count =
    view->dir_entry[view->ptr_c]->Info->file_count;

  if(file_count > y)
    view->scroll_c = (file_count - y);
   
  view->ptr_c = file_count-1;

  view->ptr_pos_c =
    (view->ptr_c - view->scroll_c);

  if(visual.line == 1)
    visual_tagging(view, TAG_DN, old_ptr);

  view->update_list = UPDATE;
} /* }}} */

/* marker_goes_to_last_file_in_window {{{ */
void
marker_goes_to_lfiw(ListView *view)
{
  int y = getmaxy(view->win);
  int old_ptr = view->ptr_c;

  int file_count =
    view->dir_entry[view->ptr_c]->Info->file_count;

  if(file_count > y)
    view->ptr_c = (view->scroll_c + y-1);

  else view->ptr_c = file_count-1;

  view->ptr_pos_c = (view->ptr_c - view->scroll_c);

  if(visual.line == 1)
    visual_tagging(view, TAG_DN, old_ptr);

  view->update_list = UPDATE;
} /* }}} */

/* visual_tagging {{{ */
void
visual_tagging(ListView *view, int tag_m, int old_ptr)
{
  int tag_c;

  if(tag_m == TAG_DN)
  {
    for(tag_c = old_ptr;
        tag_c <= view->ptr_c; tag_c++)
    {
      if(visual.start_line > tag_c)
        view->dir_entry[tag_c]->is_selected = 0;
        
      else view->dir_entry[tag_c]->is_selected = 1;
    }
  }
  else if(tag_m == TAG_UP)
  {
    for(tag_c = old_ptr;
        tag_c >= view->ptr_c; tag_c--)
    {
      if(visual.start_line < tag_c)
        view->dir_entry[tag_c]->is_selected = 0;
        
      else view->dir_entry[tag_c]->is_selected = 1;
    }
  }
} /* }}} */

/* visual_line_mode {{{ [V]*/
void
visual_line_mode(ListView *view, cfg_t **cfg,  int x, int y)
{
  if(visual.line != 1)
  {
    visual.line = 1;
    visual.start_line = view->ptr_c;

    if(visual.start_line == 0)
      visual.start_line++;

    show_cmd_bar(view, cfg, x, y, st2_win, NULL);
  }
  else
  {
    if(view->ptr_c > visual.start_line ||
        view->ptr_c < visual.start_line)
    {
      get_files(&view->dir_entry, &view->entry_buffer_size,
          view->path, view->file_filter, view->sortoption, view->sortmode);

      view->update_list = UPDATE;
    }
    else
    {
      view->dir_entry[(view->ptr_c)]->is_selected = 0;
      view->update_list = UPDATE;
    }
    visual.line = 0;
  }
} /* }}} */

/* switch_home {{{ */
void
switch_home(ListView *view, cfg_t **cfg)
{
  int err = 1;

  err = change_directory(view, cfg, getenv("HOME"));
  switch(err)
  {
    case -1:
      show_error_message(*cfg,
          "path does not exist!");
      break;
    case -2:
      show_error_message(*cfg,
          "you are in home!");
      break;
    case 0:
      show_error_message(*cfg,
          "error while read and init files!");
      break;
  }
} /* }}} */

/* kill_cmd_mode {{{ */
void
kill_cmd_mode(WINDOW *win)
{
  int idx = 0;

  curs_set(0);

  for(; idx < (strlen(cmd_line.buf)); idx++)
    mvwprintw(win, 1, idx, "%c", '\040');

  cmd_line.buf[0] = '\0';
  cmd_line.count = 0;
  cmd_line.mode = 0;
  cmd_line.cur_pos = 0;
} /* }}} */

/* cmd_input {{{ */
int
show_cmd_input(int key, int x, int y, WINDOW *win)
{
  if(key == -1) return 0;
  char *tail, tail_buf[MAX_CMD_SIZE];
  int tail_len = 0, cmd_len = 0;
  //int cmd_outp_len = 0;
  
  curs_set(1);
  cmd_line.mode = 1;
  cmd_len = strlen(cmd_line.buf);
  //cmd_outp_len = ret_utf8_str_column_sum(cmd_line.buf, cmd_len);

  switch(key)
  {
    case 27:
      kill_cmd_mode(win);
      break;

    case 0xA:
      if(cmd_line.buf[0] == ':' && cmd_line.buf[1] == 'q')
        return 1;
      break;

    case KEY_LEFT:
      if(cmd_line.count > 0) cmd_line.count--;
      wmove(win, 1, cmd_line.count);
      break;

    case KEY_RIGHT:
      if(cmd_line.count+1 <= cmd_len) cmd_line.count++;
      wmove(win, 1, cmd_line.count);
      break;

    case KEY_UP:
      // here comes later cmd history
      break;
    case KEY_DOWN:
      // here comes later cmd history
      break;

    case KEY_BACKSPACE:
      del_cmd_character(key, y, win);
      wmove(win, 1, cmd_line.count);
      break;

    case KEY_HOME:
      cmd_line.count = 1;
      wmove(win, 1, cmd_line.count);
      break;

    case KEY_END:
      cmd_line.count = cmd_len;
      wmove(win, 1, cmd_line.count);
      break;

    default:
      if(key <= CTRL_KEYS || (key < 32 || key >= 127)) return 0;
      else if(cmd_len+1 >= MAX_CMD_SIZE)
      {
        curs_set(0);
        kill_cmd_mode(win);
        mvwprintw(win, 1, 0, "command is to long!");
        return 0;
      }
      tail = &cmd_line.buf[cmd_line.count];
      tail_len = (cmd_len - cmd_line.count + 1);
      snprintf(tail_buf, tail_len, "%s", tail);
      cmd_line.buf[cmd_line.count] = key;
      cmd_line.buf[(cmd_line.count + 1)] = '\0';
      strncat(cmd_line.buf, tail_buf, tail_len);

      mvwaddstr(win, 1, 0, cmd_line.buf);
      cmd_line.count++;
      cmd_line.buf[(cmd_len+1)] = '\0';

      wmove(win, 1, cmd_line.count);
      break;
  }
  return 0;
} /* }}} */

/* del_cmd_character {{{ */
void
del_cmd_character(int key, int y, WINDOW *win)
{
  int tail_len;
  char *del_c;

  if(cmd_line.count >= 1)
  {
    cmd_line.count--;

    tail_len = (strlen(cmd_line.buf) - cmd_line.count);

    char tail_buf[tail_len];

    if(cmd_line.count+1 != strlen(cmd_line.buf))
    {
      del_c = &cmd_line.buf[(cmd_line.count + 1)];
      cmd_line.buf[cmd_line.count] = '\0';

      snprintf(tail_buf, tail_len, "%s", del_c);

      strncat(cmd_line.buf, tail_buf, tail_len);
      mvwprintw(win, 1, 0, "%s", cmd_line.buf);
    }
    else cmd_line.buf[cmd_line.count] = '\0';

    mvwprintw(win, 1, strlen(cmd_line.buf), "%c", '\040');
  }
} /* }}} */

/* filter_keystroke {{{ */
int
filter_keystroke(Tab **curr_tab, cfg_t **cfg, int x, int y, int key)
{
  if(delay.status == ON)
  {
    delay.status = OFF;
    cbreak();
  }
  if(cmd_line.mode == 1)
    return show_cmd_input(key, x, y, st2_win);
  else if(isdigit(key))
  {
    if(key == '0' && k_scut.value == 0);
    else
    {
      k_scut.value = ret_cypher(k_scut.value, key);
      key = GET_NEW_KEY;
    }
  }
  else key = check_valid_shortcut( (*cfg)->user_k,
      (*cfg)->user_k_size, key);
  if(key == GET_NEW_KEY)
  {
    (*curr_tab)->sel_view->update_list = UPDATE;
    return 0;
  }
  else return check_key(curr_tab, cfg, key, x, y);
} /* }}} */

/* ret_ui_cmd {{{ */
int
ret_ui_cmd(keys_t **user_k, int idx)
{
  int c = 0, match = 0;
  int uicmd_size = (sizeof(ui_cmd) / sizeof(ui_cmd[0]));

  if(user_k[idx]->cmd == NULL)
    return 0;

  for(; c < uicmd_size; c++)
  {
    if(strcmp(user_k[idx]->cmd, ui_cmd[c]) == 0)
    {
      match = c;
      break;
    }
  }
  return match;
} /* }}} */

/* check_scase {{{ */
int
check_scase(keys_t **user_k, int idx, int key)
{
  int c, k = 0;

  for(c = 0; c < user_k[idx]->s_count; c++)
    if(key == user_k[idx]->special_case[c]) k = idx;

  if(k != 0 && k_scut.buf[0] != '\0')
    k_scut.buf[0] = '\0';

  return ret_ui_cmd(user_k, k);
} /* }}} */

/* check_shortcut {{{ */
int
check_shortcut(keys_t **user_k, int idx, int key)
{
  int c, k = 0;
  char sc_buf[SHORTCUT_L];
  sc_buf[0] = '\0';

  if(k_scut.buf[0] != '\0')
    snprintf(sc_buf, SHORTCUT_L, "%s%c", k_scut.buf, key); 
  for(c = 0; c < user_k[idx]->count; c++)
  {
    if(k_scut.buf[0] != '\0')
    {
      if(strcmp(sc_buf, user_k[idx]->buf[c]) == 0)
      {
        k = ret_ui_cmd(user_k, idx);
        k_scut.buf[0] = '\0';
        break;
      }
      else continue;
    }
    else if(key == user_k[idx]->buf[c][0])
    { // if strlen > 1 then we search for a sc
      if(strlen(user_k[idx]->buf[c]) > 1)
      {
        snprintf(k_scut.buf, SHORTCUT_L, "%c", key);
        k = GET_NEW_KEY;
      } // else ident is a valid single key
      else k = ret_ui_cmd(user_k, idx);
    }
  }
  return k;
} /* }}} */

/* check_valid_shortcut {{{ */
int
check_valid_shortcut(keys_t **user_k, int user_k_size, int key)
{
  int idx = 1;
  int k = 0;

  while(idx != user_k_size)
  {
    if((k = check_scase(user_k, idx, key)) == 0)
    {
      if((k = check_shortcut(user_k, idx,
              key)) != 0) break;
      idx++;
    }
    else break;
  }
  if(key == KEY_PERCENT) return key;
  else if(k_scut.buf[0] != '\0' && k == 0)
  {
    k_scut.buf[0] = '\0';
    return check_valid_shortcut(user_k, user_k_size, key);
  }
  else if(k == -1 || k == 0) return GET_NEW_KEY;
  else return k;
} /* }}} */

/* ret_percent_in_list {{{ */
int
ret_percent_in_list(ListView *view, int count)
{
  int x = 100;
  count *= view->dir_entry[view->ptr_c]->Info->file_count-1;

  x = count % x;

  count /= 100;
  if(x > 50) count++;

  return count;
} /* }}} */

/* show_error_message {{{ */
void
show_error_message(cfg_t *cfg, char *error)
{
  wattrset(st2_win, COLOR_PAIR(ERROR)|cfg->color[ERROR].attr);
  mvwprintw(st2_win, 1, 1, "%s", error);
  halfdelay(cfg->redraw_delay);
  delay.status = ON;
} /* }}} */

/* check_key {{{ */
int
check_key(Tab **curr_tab, cfg_t **cfg, int key, int x, int y)
{
  int c = 0, scut_loop = 1;
  int ret = 0, exit = 0;
  int scrl_step = 1, err = 0;

  Tab *old_tab = (*curr_tab), *tmp = NULL;
  ListView *view = (*curr_tab)->sel_view;

  if(k_scut.value != 0) scut_loop = k_scut.value; 
  
  switch(key)
  {
    case NORMAL_MODE:
      if(visual.line == 1)
        visual_line_mode(view, cfg, x, y);
      show_cmd_bar(view, cfg, x, y, st2_win, NULL);
      break;

    case CMD_MODE:
      return show_cmd_input(':', x, y, st2_win);
      break;

    case DIR_DN:
      if(exit == 1) ret = 1;
      else if(visual.line == 0)
      {
        err = directory_dn(view, cfg, y, x);

        if(err == -1) show_error_message(*cfg,
            "permission denied!");
        else if(err == -2) show_error_message(*cfg,
            "file is not a directory!");
        else if(err == 0) show_error_message(*cfg,
            "directory was reloaded!");
      }
      else
      {
        if(view->ptr_c != 0)
          visual_tagging(view, TAG_DN, view->ptr_c);
        update_ptr_down(view);

        view->update_list = UPDATE;
      }
      break;
    
    case DIR_UP:
      if(visual.line != 1)
        directory_up(view, cfg);
      break;

    case KEY_PERCENT:
      scut_loop =
        ret_percent_in_list(view, scut_loop);
      
      marker_goes_top(view);
      while(c != scut_loop)
      {
        update_ptr_down(view);
        if(visual.line == 1)
          visual_tagging(view, TAG_DN, view->ptr_c-1);
        c++;
      }
      break;

    case MARKER_DN:
      while(c != scut_loop)
      {
        if(visual.line == 1)
          visual_tagging(view, TAG_DN, view->ptr_c);
        update_ptr_down(view);
        c++;
      }
      view->update_list = UPDATE;
      break;

    case MARKER_UP:
      while(c != scut_loop)
      {
        if(visual.line == 1)
          visual_tagging(view, TAG_UP, view->ptr_c);
        update_ptr_up(view);
        c++;
      }
      view->update_list = UPDATE;
      break;

    case SCROLL_PAGE_DN:
      while(c != scut_loop)
      {
        if(view->ptr_c !=
            view->dir_entry[view->ptr_c]->Info->file_count-1)
          update_page_dn(view, scrl_step);
        else break;
        c++;
      }
      break;

    case SCROLL_HALF_PAGE_DN:
      if(y > 6) scrl_step++;
      while(c != scut_loop)
      {
        if(view->ptr_c !=
            view->dir_entry[view->ptr_c]->Info->file_count-1)
          update_page_dn(view, scrl_step);
        else break;
        c++;
      }
      break;

    case SCROLL_PAGE_UP:
      while(c != scut_loop)
      {
        if(view->ptr_c != 0)
          update_page_up(view, scrl_step);
        else break;
        c++;
      }
      break;

    case SCROLL_HALF_PAGE_UP:
      if(y > 6) scrl_step++;
      while(c != scut_loop)
      {
        if(view->ptr_c != 0)
          update_page_up(view, scrl_step);
        else break;
        c++;
      }
      break;

    case KEY_BACKSPACE: // DEL
      if(visual.line != 1)
        directory_up(view, cfg);
      else
      {
        if(view->ptr_c != 0)
          visual_tagging(view, TAG_UP, view->ptr_c);
        update_ptr_up(view);

        view->update_list = UPDATE;
      }
      break;

    case TOGGLE_FILEFILTER:
      toggle_file_filter(view, cfg, y);
      break;

    case TOGGLE_SORT:
      toggle_sortfiles(view, cfg);
      break;

    case TAG_FILE:
      tag_file(view, y);
      break;

    case NEXT_TAB:
      next_tab(curr_tab, cfg, y, x);
      break;

    case PREV_TAB:
      prev_tab(curr_tab, cfg, y, x);
      break;      

    case VISUAL_MODE:
      visual_line_mode(view, cfg, x, y);
      break;

    case OPEN_IN_NEW_TAB:
      tmp = create_new_tab(*curr_tab,
          cfg, y, x, (*cfg)->always_split, &err);
      if (tmp == NULL)
      {
        if(err == 1) show_error_message(*cfg,
            "not enouth space available!");
        else if(err == 2) show_error_message(*cfg,
            "files cannot open in new tab!");
        else if(err == 3) show_error_message(*cfg,
            "failed to open path!");
      }
      else
      {
        *curr_tab = tmp;
        switch_to_tab(*curr_tab, old_tab, cfg, y, x);
      }
      break;

    case OPEN_IN_HSPLIT:
      err = open_new_view(curr_tab, cfg, y, x);
      if(err == -1) show_error_message(*cfg,
          "files cannot open in new view!");
      else if(err == 0) show_error_message(*cfg,
          "failed to open path!");
      break;

    case CLOSE_VIEW:
      close_view(curr_tab, cfg, y, x);
      break;

    case DELETE_TAB:
      kill_sel_tab(curr_tab, cfg, y, x);
      break;

    case MARKER_BOT:
      marker_goes_bot(view);
      if(k_scut.value != 0)
      {
        c = view->dir_entry[view->ptr_c]->Info->file_count-1;
        while(c != scut_loop && c != 0)
        {
          if(visual.line == 1)
            visual_tagging(view, TAG_UP, view->ptr_c);
          update_ptr_up(view);
          c--;
        }
        view->update_list = UPDATE;
      }
      break;

    case MARKER_TOP:
      marker_goes_top(view);
      if(k_scut.value != 0)
      {
        while(c != scut_loop)
        {
          if(visual.line == 1)
            visual_tagging(view, TAG_DN, view->ptr_c);
          update_ptr_down(view);
          c++;
        }
        view->update_list = UPDATE;
      }
      break;

    case MARKER_TOP_WIN:
      marker_goes_tow(view);
      break;

    case MARKER_CENTER_WIN:
      marker_goes_cow(view);
      break;

    case MARKER_BOT_WIN:
      marker_goes_to_lfiw(view); // lfiw = last file in window
      break;

    case TOGGLE_WIN:
      if( (*curr_tab)->r_view != NULL &&
          visual.line == 0)
        toggle_view(*curr_tab, cfg, y, x);
      break;

    case REDRAW:
      reload(view, cfg);
      break;

    case HOMESWITCH:
      switch_home(view, cfg);
      break;
  }
  k_scut.value = 0;
  return ret;
} /* }}} */
