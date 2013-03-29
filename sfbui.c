/* sfbui.c by lutherus 2012 */
/* includes {{{ */
#include "seoni.h"
#include "filedata.h"
#include "utils.h"
#include "config.h"
#include "sort.h"
#include "sfbui.h"
#include "history.h"
#include "tab.h"
#include "keys.h"
#include "mimetypes.h"
#include <unistd.h> /* }}} */

/* macros {{{ */
#define LIST_L 1
#define LIST_R 2
#define ON 1
#define OFF 0
#define U_SORT 1
#define D_SORT 2
#define UPDATE 1
#define DONE 0
#define DUMPLOAD 1
/* }}} */

/* append_view {{{ */
ListView
*append_view(WINDOW *win, cfg_t **cfg, char *path)
{
  char *pwd = NULL;
  int idx = 0, path_len = 0;

  ListView *new_view = (ListView *)malloc(sizeof(ListView));

  new_view->path          = NULL;
  new_view->path_size     = 0;
  new_view->ptr_c         = 0;
  new_view->scroll_c      = 0;
  new_view->ptr_pos_c     = 0;
  new_view->page_scroll_c = 0;

  new_view->file_filter  = ON;
  new_view->sortmode = U_SORT;
  new_view->sortoption = (*cfg)->sort;
  new_view->update_list = DONE;
  new_view->dir_entry = NULL;
  new_view->entry_buffer_size = 1000;
  new_view->win = win;

  for(; idx < 100; idx++)
    new_view->dir_hist[idx].dir = NULL;

  if(path == NULL)
  {
    pwd = getenv("PWD");
    if(pwd == NULL || access(pwd, 00) == -1)
      pwd = "/";
  }
  else pwd = path;
  path_len = strlen(pwd)+1;
  new_view->path = (char *)malloc(path_len * sizeof(char));
  new_view->path_size = path_len;

  snprintf(new_view->path, path_len, "%s", pwd);
  if(access(new_view->path, 04) != -1)
  {
    get_files(&new_view->dir_entry, &new_view->entry_buffer_size,
        new_view->path, new_view->file_filter, new_view->sortoption,
        new_view->sortmode);
  }
  return new_view;
} /* }}} */

/* prepend_view {{{ */
ListView
*prepend_view(ListView *query, cfg_t **cfg, char *path)
{
  int path_len = 0;
  if(query == NULL) return NULL;

  query->ptr_c         = 0;
  query->scroll_c      = 0;
  query->ptr_pos_c     = 0;
  query->page_scroll_c = 0;
  query->file_filter = ON;
  query->sortmode = U_SORT;
  query->sortoption = (*cfg)->sort;
  query->update_list = DONE;

  zero_history(query);

  if(access(path, 00) != -1)
  {
    path_len = strlen(path)+1;
    if(path_len > query->path_size)
    {
      query->path = (char *)realloc(query->path,
          path_len * sizeof(char));

      query->path_size = path_len;
    }
    snprintf(query->path, query->path_size, "%s", path);

    get_files(&query->dir_entry, &query->entry_buffer_size,
        query->path, query->file_filter, query->sortoption,
        query->sortmode);
  }
  return query;
} /* }}} */

/* init_state_win {{{ */
void
init_state_win(int y, int x)
{
  st_win = newwin(2, x, 0, 0);
  st2_win = newwin(2, x, y-2, 0);
} /* }}} */

/* update_ptr_down {{{ */
int
update_ptr_down(ListView *view)
{
  int file_count =
    view->dir_entry[view->ptr_c]->Info->file_count;

  if(view->ptr_pos_c <
      (file_count-1 - view->scroll_c))
    view->ptr_pos_c++;

  if(view->ptr_c == (file_count-1) ) return 0;
  view->ptr_c++;

  if(view->ptr_c > (view->ptr_pos_c + view->scroll_c))
  {
    if(view->ptr_c != file_count)
      view->scroll_c++;
  }
  return 1;
} /* }}} */

/* update_ptr_up {{{ */
void
update_ptr_up(ListView *view)
{
  if(view->ptr_pos_c > 0)
    view->ptr_pos_c--;
  
  if(view->ptr_c > 0)
  {
    if(view->scroll_c != 0)
    {
      if(view->ptr_c == view->scroll_c)
      {
        view->scroll_c--;
      }
    }
    view->ptr_c--;
  }
} /* }}} */

/* update_page_dn {{{ */
int
update_page_dn(ListView *view, int scroll_step)
{
  int y = getmaxy(view->win);

  int file_count =
    view->dir_entry[view->ptr_c]->Info->file_count;

  if(file_count < y+1) return 0; //y-3

  view->curr_page_sum = (file_count - view->scroll_c);
  view->page_scroll_c = 0;

  if(view->curr_page_sum >= y+1)
  {
    while(view->page_scroll_c != ((y-1)/scroll_step)) // y-5
    {
      if(view->scroll_c != file_count -y) // y+4
        view->scroll_c++;

      view->page_scroll_c++;
    }
  } // y+4
  if(view->scroll_c == file_count -y &&
      view->page_scroll_c == 0)
    view->ptr_pos_c = y-1;

  view->ptr_c = (view->scroll_c + view->ptr_pos_c);
  view->update_list = UPDATE;
  return 1;
} /* }}} */

/* update_page_up {{{ */
int
update_page_up(ListView *view, int scroll_step)
{
  int y = getmaxy(view->win);

  int file_count =
    view->dir_entry[view->ptr_c]->Info->file_count;

  if(file_count < y+1 && view->ptr_c == 0) return 0;

  view->curr_page_sum = (file_count - view->scroll_c);
  view->page_scroll_c = 0;

  if(view->curr_page_sum != file_count)
  {
    while(view->page_scroll_c != ((y-1)/scroll_step))
    {
      if(view->scroll_c != 0)
        view->scroll_c--;
  
      view->page_scroll_c++;
    }
  }
  if(view->scroll_c == 0 && view->page_scroll_c == 0)
    view->ptr_pos_c = 0;
  
  view->ptr_c = (view->scroll_c + view->ptr_pos_c);
  view->update_list = UPDATE;
  return 1;
} /* }}} */

/* update_files {{{ */
void
update_files(ListView *view, cfg_t **cfg, char *file)
{
  int path_len = 0;
  int scrolled = 0;
  char *target_file;
  set_last_dir_pos(view);
  
  target_file = conv_filename(view->path);
  char last_dir[( strlen(target_file)+1 )];

  snprintf(last_dir,
      strlen(target_file)+1, "%s", target_file);

  if(file != NULL)
  {
    path_len = (strlen(view->path) + strlen(file)+2);
    if(path_len > view->path_size)
    {
      view->path = (char *)realloc(view->path, path_len * sizeof(char));
      view->path_size = path_len;
    }
  }
  add_path(file, view->path, view->path_size);
  
  get_files(&view->dir_entry, &view->entry_buffer_size,
      view->path, view->file_filter, view->sortoption, view->sortmode);

  if(file == NULL) view->ptr_c = search_dir_pos(view, last_dir);
  else view->ptr_c = ret_last_dir_pos(view, &scrolled);
 
  view->scroll_c = scrolled;
  view->ptr_pos_c = (view->ptr_c - view->scroll_c);

  view->update_list = UPDATE;
} /* }}} */

/* directory_up {{{ */
void
directory_up(ListView *view, cfg_t **cfg)
{
  if(strlen(view->path) < 2)
  {
    view->ptr_c = 0;
    view->ptr_pos_c = 0;
    view->scroll_c = 0;

    view->update_list = UPDATE;
  }
  else update_files(view, cfg, NULL);
} /* }}} */

/* directory_down {{{ */
int
directory_dn(ListView *view, cfg_t **cfg, int y, int x)
{
  int d_reload = 0, file_t = 0;
  int path_len = (strlen(view->path) +
      strlen(view->dir_entry[view->ptr_c]->name))+2;

  char file_buf[path_len];

  wattrset(st2_win, COLOR_PAIR(CMDBAR)|A_BOLD);

  if(ret_dot_type(
        view->dir_entry[view->ptr_c]->name) == CWD_DOT)
  {
    get_files(&view->dir_entry, &view->entry_buffer_size,
        view->path, view->file_filter, view->sortoption, view->sortmode);
    d_reload = 1;
  }
  if(view->ptr_c != 0 && d_reload == 0)
  {
    build_path(view->path, view->dir_entry[view->ptr_c]->name,
        file_buf, path_len);
  
    file_t = ret_filetype(file_buf);
    if(file_t == IS_DIRECTORY || file_t == IS_SYMDIR)
    {
      if(access(file_buf, 04) == -1) return -1;
      update_files(view, cfg, view->dir_entry[view->ptr_c]->name);
    }
    else return -2;
  }
  else if(d_reload == 1) return 0;
  else directory_up(view, cfg);
  return 1;
} /* }}} */

/* change_directory {{{ */
int
change_directory(ListView *view, cfg_t **cfg, char *path)
{
  int path_len = 0;
  int scrolled = 0;
  if(path == NULL || access(path, 00) == -1) return -1;
  else if(strcmp(view->path, path) == 0)
    return -2;
  set_last_dir_pos(view);
  path_len = strlen(path)+1;
  if(path_len > view->path_size)
  {
    view->path = (char *)realloc(view->path, path_len * sizeof(char));
    view->path_size = path_len;
  }
  snprintf(view->path, view->path_size, "%s", path);

  if(get_files(&view->dir_entry, &view->entry_buffer_size,
        view->path, view->file_filter, view->sortoption, view->sortmode) == 0)
    return 0;
  view->ptr_c = ret_last_dir_pos(view, &scrolled);
  view->scroll_c = scrolled;
  view->ptr_pos_c = (view->ptr_c - view->scroll_c);
  view->update_list = UPDATE;
  return 1;
} /* }}} */

/* check_limits {{{ */
// short hacks to regulate limits in functions
// becarefull here!!!
void
check_limits(ListView *view)
{
  int y = getmaxy(view->win);

  int file_count =
    view->dir_entry[view->ptr_c]->Info->file_count;

  /* if marker out of lastfile
   * when nothing is scrolled .
   * it is possible when first the dotfiles
   * are showed and to the next time are hidden */
  if(view->ptr_c > file_count-1)
  {
    view->ptr_c = file_count-1;
    view->ptr_pos_c = view->ptr_c;
  }
  /* when window is smaller than the current
   * market position than scroll automatic */
  if( (view->ptr_c - view->scroll_c) > y-1)
  {
    view->ptr_pos_c = y-1;
    view->scroll_c = (view->ptr_c - view->ptr_pos_c);
  }
  /* snap list when filesummary is longer than
   * screen size and scroll counter not zero */

  while(file_count - view->scroll_c < y && view->scroll_c != 0)
  {
    view->scroll_c--;
    view->ptr_pos_c++;
  }
} /* }}} */

/* toggle_dotfiles {{{ */
void
toggle_file_filter(ListView *view, cfg_t **cfg, int y)
{
  int respawn_limit = 0;

  if(view->file_filter == OFF)
  {
    respawn_limit = (view->dir_entry[view->ptr_c]->Info->file_count -
        view->dir_entry[view->ptr_c]->Info->files_filtered);

    if(view->scroll_c != 0 &&
        view->scroll_c > respawn_limit - view->page_scroll_c)
    {
      view->ptr_c = view->ptr_pos_c;
      view->scroll_c = 0;
    }
    view->file_filter = ON;
  }  
  else if(view->file_filter == ON)
    view->file_filter = OFF;

  get_files(&view->dir_entry, &view->entry_buffer_size,
      view->path, view->file_filter, view->sortoption, view->sortmode);

  view->update_list = UPDATE;
} /* }}} */

/* sort_toggle {{{ */
void
toggle_sortfiles(ListView *view, cfg_t **cfg)
{
  if(view->sortmode == D_SORT)
    view->sortmode = U_SORT;

  else if(view->sortmode == U_SORT)
    view->sortmode = D_SORT;

  inverse_sort(&view->dir_entry);
  view->update_list = UPDATE;
} /* }}} */

/* tag_file {{{ */
void
tag_file(ListView *view, int y)
{
  if(view->ptr_c != 0)
  {
    if(view->dir_entry[view->ptr_c]->is_selected == 1)
      view->dir_entry[view->ptr_c]->is_selected = 0;

    else view->dir_entry[view->ptr_c]->is_selected = 1;
  }
  if(view->ptr_c ==
      view->dir_entry[view->ptr_c]->Info->file_count-1)
    view->update_list = UPDATE;

  if(visual.line == 0)
    update_ptr_down(view);

  view->update_list = UPDATE;
} /* }}} */

/* print_borders {{{ */
void
print_borders(ListView *view, int master)
{
  int y = getmaxy(view->win);
  int x = getmaxx(view->win);
  int line_c;

  wattrset(view->win, COLOR_PAIR(BORDER));
  for(line_c = 0; line_c < y+1; line_c++)
  {
    if(master == 1)
      mvwprintw(view->win, line_c, 0, "%c", '\040');
    mvwprintw(view->win, line_c, x-1, "%c", '\040');
  }
} /* }}} */

/* show list {{{ */
void
show_list(ListView *view, Color *color, int master)
{
  int y = getmaxy(view->win);
  int x = getmaxx(view->win);

  int fc = 0, c = 0;

  // loutp = list output
  int max_loutp =
    (view->dir_entry[view->ptr_c]->Info->file_count - view->scroll_c);

  if(view->scroll_c != 0) fc += view->scroll_c;

  for(; c < y; c++)
  {
    if(c < max_loutp)
    {
      show_name_in_list(view, color, (c+fc), 1, x, master);
      show_fsize_in_list(view, color, (c+fc), 1, x);
    }
    else
    {
      if(view->ptr_c == (max_loutp + view->scroll_c)-1)
        wattrset(view->win, COLOR_PAIR(WIN));
      mvwaddstr(view->win, c+fc, 1, "~");
    }
  }
}/* }}} */

/* set_file_color {{{ */
void
set_file_color(ListView *view, Color *color, int file_c)
{
  int filetype = view->dir_entry[file_c]->file_t, val_col = -1;
  int valid_colors[] =
  {
    EXECUTABLE, SYMLINK, SYMLINK, DIRECTORY,
    BLK_DEVICE,DEVICE, SOCKET, NFILE
  };
  if(valid_colors[filetype] == NFILE
      && view->dir_entry[file_c]->executable == 1)

    wattrset(view->win, COLOR_PAIR(EXECUTABLE)|color[EXECUTABLE].attr);
  else
  {
    val_col = valid_colors[filetype];
    if(view->dir_entry[file_c]->is_directory == 1)
      wattrset(view->win, COLOR_PAIR(val_col)|color[val_col].attr);

    else wattrset(view->win, COLOR_PAIR(val_col)|color[val_col].attr);  
  }
} /* }}} */

/* show_name_in_list {{{ */
void
show_name_in_list(ListView *view, Color *color, int c, int curr_win, int x, int master)
{
  char file_buf[MAX_COLS];
  char trnc_show[]= "…";

  int truncate = 0, max_file_outp_len = 0, show_begin = 2;
  int mb_strlen = mbstowcs(NULL, view->dir_entry[c]->name, MB_CUR_MAX);
  int slash_pos = 0, col = 0;

  if(master == 0) show_begin--;

  max_file_outp_len = (x - F_SIZE_POS - show_begin);

  truncate = calc_utf8_truncate_size(view->dir_entry[c]->name,
      &mb_strlen, max_file_outp_len);

  slash_pos = mb_strlen+show_begin;

  if(view->ptr_c == c && curr_win == 1)
    wattrset(view->win, COLOR_PAIR(CURRENT)|color[CURRENT].attr);
  else if(view->dir_entry[c]->is_selected == 1)
    wattrset(view->win, COLOR_PAIR(SELECT)|color[SELECT].attr);
  else set_file_color(view, color, c);

  if(view->ptr_c == c) // if c == selected filename
  {
    for(col = master; col < x-1; col++)
      mvwaddstr(view->win, c - view->scroll_c, col, " ");
  }
  snprintf(file_buf, truncate, "%s", view->dir_entry[c]->name);
  mvwaddstr(view->win, c - view->scroll_c, show_begin, file_buf);

  if(max_file_outp_len <=
      ret_utf8_str_column_sum(view->dir_entry[c]->name, 255))
  {
    slash_pos = max_file_outp_len+show_begin;
    mvwaddstr(view->win, c - view->scroll_c, max_file_outp_len+master, trnc_show);
  }
  if(view->dir_entry[c]->is_directory == 1)
  {
    if(c != view->ptr_c) wattrset(view->win, COLOR_PAIR(FSLASH)|color[FSLASH].attr);
    mvwaddstr(view->win, c - view->scroll_c, slash_pos, "/");
  }
} /* }}} */

/* show_fsize_in_list {{{ */
void
show_fsize_in_list(ListView *view, Color *color, int c, int curr_win, int x)
{
  int size_len= 0;

  if(c == view->ptr_c && curr_win == 1)
    wattrset(view->win, COLOR_PAIR(CURRENT)|color[CURRENT].attr);
  else wattrset(view->win, COLOR_PAIR(WIN)|color[WIN].attr);
  // show not the size of ../
  if(c != 0)
  {
    // 5 is the max len from a sizeview
    size_len = strlen(view->dir_entry[c]->size_str);//- 5;

    mvwprintw(view->win, c - view->scroll_c,
        x - size_len - 2, "%s", view->dir_entry[c]->size_str);
  }
} /* }}} */

/* show_last_hbar_pos {{{ */
void
show_last_hbar_pos(ListView *view, Color *color, int master)
{
  int x = getmaxx(view->win);
  int curr_win = 0;

  show_name_in_list(view, color,
      view->ptr_c, curr_win, x, master);

  show_fsize_in_list(view, color,
      view->ptr_c, curr_win, x);

  wattrset(view->win, COLOR_PAIR(FSLASH)|color[FSLASH].attr);
  mvwprintw(view->win, view->ptr_c - view->scroll_c, master, ">");
} /* }}} */

/* show_message_bar {{{ */
void
show_file_bar(ListView *view, int y, int x, WINDOW *win, int deep_scan)
{
  double filesize_c = 0;
  int file_c, item_is_sel = 0;
  int path_len = (strlen(view->path) + MAX_FILELEN + 2); // 2 = / & \0
  char absol_p[path_len], size_buf[MAX_SIZE];

  for(file_c = 0; file_c < view->dir_entry[0]->Info->file_count; file_c++)
  {
    if(view->dir_entry[file_c]->is_selected == 1)
    {
      item_is_sel++;

      build_path(view->path, view->dir_entry[file_c]->name,
          absol_p, path_len);

      filesize_c += get_filesize(absol_p, deep_scan);
    }
  }
  if(filesize_c == 0)
  {
    build_path(view->path, view->dir_entry[view->ptr_c]->name,
        absol_p, path_len);

    filesize_c = get_filesize(absol_p, deep_scan);
  }
  convert_filesize(filesize_c, size_buf);

  if(deep_scan == DEEP_ON)
    filesize_c = 0;
    //show_full_file_properties(view, x, y, item_is_sel, filesize_c, win);
  else
  {
    wattrset(win, COLOR_PAIR(BORDER)|A_BOLD);

    for(file_c = 0; file_c < x; file_c++)
      mvwprintw(win, 0, file_c, "%c", '\040');

    show_file_properties(view, item_is_sel, size_buf, win);
    show_filtered_files(view->dir_entry[view->ptr_c]->Info->files_filtered, x, win);
  }
} /* }}} */

/* show_file_propertys {{{ */
int
show_file_properties(ListView *view, int sel_c, char *size, WINDOW *win)
{
  int x = (view->ptr_c);
  int path_len = (strlen(view->path) +
      strlen(view->dir_entry[view->ptr_c]->name) + 2);

  char *outp = NULL;
  char pathbuf[path_len];

  if(view->ptr_c == 0)
  {
    mvwaddstr(win, 0, 1, ".. up a dir");
    return 1;
  }
  build_path(view->path, view->dir_entry[x]->name, pathbuf, path_len);
  check_file_access(view->dir_entry[x]->show_bits, pathbuf);
  if(sel_c > 0)
  {
    if(sel_c > 1) outp = "files";
    else outp = "file";
    mvwprintw(win, 0, 1, "%d %s selected ( %s )", sel_c, outp, size);
    return 1;
  }
  get_owner_info(&view->dir_entry[x], pathbuf);
  if(view->dir_entry[x]->mtime != 0 &&
      view->dir_entry[x]->mtime_str[0] == '\0')
  {
    conv_time(view->dir_entry[x]->mtime,
        view->dir_entry[x]->mtime_str);
  }
  mvwprintw(win, 0, 1, "%s  %s:%s  %s  %d",
      view->dir_entry[x]->show_bits, view->dir_entry[x]->owner,
      view->dir_entry[x]->group, view->dir_entry[x]->mtime_str,
      view->dir_entry[x]->nlinks);
  return 1;
} /* }}} */

/* show_filtered_files {{{ */
void
show_filtered_files(int num_files_f, int x, WINDOW *win)
{
  char *out_p, out_buf[100];
  int outp_len;

  if(num_files_f > 0)
  {
    if(num_files_f > 1) out_p = "files";
    else out_p = "file";

    snprintf(out_buf, 100,
        "%d %s filtered", num_files_f, out_p);

    outp_len = strlen(out_buf)+1;
    panel.bot_filter_show_len = outp_len;

    mvwprintw(win, 0, x-outp_len, "%s", out_buf);
  }
  else panel.bot_filter_show_len = 0;
} /* }}} */

/* show_list_state {{{ */
int
show_list_state(ListView * view, int x, int y, WINDOW *win)
{
  char scrl_percent[4], list_inf[24];

  int scrolled = view->scroll_c;
  int inf_len;

  int scroll_limit =
    view->dir_entry[view->ptr_c]->Info->file_count-1;

  snprintf(list_inf, 24, "%d-%d\040",
      view->ptr_c, view->dir_entry[view->ptr_c]->Info->file_count-1);

  inf_len = strlen(list_inf);

  if(scrolled == 0) snprintf(scrl_percent, 4, "TOP");
  else
  {
    scroll_limit -= y-5;
    scrolled *= 100;
    scrolled /= scroll_limit;

    if(scrolled == 100) snprintf(scrl_percent, 4, "BOT");
    else snprintf(scrl_percent, 4, "%d%%", scrolled);
  }
  inf_len += strlen(scrl_percent)+1;

  mvwprintw(win, 1,
      x-inf_len, "%s%s", list_inf, scrl_percent);

  return inf_len;
} /* }}} */

/* show_shortcut_state {{{ */
void
show_shortcut_state(ListView *view, int max_outp, int x, WINDOW *win)
{
  char sc_outp_buf[20];

  if(k_scut.value != 0)
  {
    snprintf(sc_outp_buf, 20, "%d", k_scut.value);
    strncat(sc_outp_buf, k_scut.buf, 20);
  }
  else snprintf(sc_outp_buf, 20, "%s", k_scut.buf);

  max_outp += strlen(sc_outp_buf);
  mvwprintw(win, 1, x-max_outp, "%s", sc_outp_buf);
} /* }}} */

/* show_cmd_bar {{{ */
void
show_cmd_bar(ListView *view, cfg_t **cfg, int x, int y, WINDOW *win, char *error)
{
  int cols, max_sc_outp = 0;

  wattrset(win, COLOR_PAIR(CMDBAR)|A_BOLD);

  for(cols = 0; cols < x; cols++)
    mvwprintw(win, 1, cols, "%c", '\040');

  max_sc_outp = show_list_state(view, x, y, win)+2;
  show_shortcut_state(view, max_sc_outp, x, win);

  if(cmd_line.mode != 0)
    mvwprintw(win, 1, 0, "%s", cmd_line.buf);
  else if(error == NULL)
  {
    if(visual.line == 1) mvwprintw(win, 1, 1, "--VISUAL--");
    if( (*cfg)->reload == 1)
    {
      mvwprintw(win, 1, 1, "seoni was reloaded!");
      (*cfg)->reload = 0;
    }
  }
  else mvwprintw(win, 1, 1, "%s", error);
} /* }}} */

/* reload {{{ */
void
reload(ListView *view, cfg_t **cfg)
{
  int check_sort = (*cfg)->sort;

  init_config(cfg, RELOAD);

  if( (*cfg)->sort != check_sort)
  {
    get_files(&view->dir_entry, &view->entry_buffer_size,
        view->path, view->file_filter, view->sortoption, view->sortmode);
  }
  (*cfg)->reload = 1;
  view->update_list = UPDATE;
} /* }}} */
