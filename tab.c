/* tab.c by lutherus 2013 */

/* includes {{{ */
#include "seoni.h"
#include "config.h"
#include "filedata.h"
#include "sfbui.h"
#include "history.h"
#include "utils.h"
#include "tab.h"
#include <unistd.h> /* }}} */

/* create_new_tab {{{ */
Tab
*create_new_tab(Tab *curr_tab, cfg_t **cfg, int y, int x, int split, int *error)
{
  int path_len = 0;
  char *pathbuf = NULL;
  ListView *curr_view = NULL;
  Tab *tab = curr_tab;
  Tab *n_tab = NULL;

  if(curr_tab != NULL)
  {
    curr_view = (curr_tab->sel_view);

    path_len += strlen(curr_view->path);
    path_len += strlen(curr_view->dir_entry[(curr_view->ptr_c)]->name) + 2;

    pathbuf = (char *)malloc(path_len * sizeof(char));
  }
  if (! (n_tab = (Tab *)malloc(sizeof(Tab)) ) )
  {
    *error = 1;
    if(pathbuf != NULL)
      free(pathbuf);
    return NULL;
  }
  n_tab->next_tab = NULL;
  if(curr_tab != NULL) // if curr_tab not the head?
  {
    if(curr_view->dir_entry[(curr_view->ptr_c)]->is_directory != 1)
    {
      free(n_tab);
      free(pathbuf);
      *error = 2;
      return NULL;
    }
    else if(curr_view->ptr_c == 0)
    {
      snprintf(pathbuf, path_len, "%s", curr_view->path);
      add_path(NULL, pathbuf, path_len);
    }
    else
    {
      build_path(curr_view->path,
          curr_view->dir_entry[curr_view->ptr_c]->name,
          pathbuf, path_len);
    }
    if(access(pathbuf, 04) == -1)
    {
      free(n_tab);
      free(pathbuf);
      *error = 3;
      return NULL;
    }
    n_tab->l_win = curr_tab->l_win;
    n_tab->r_win = curr_tab->r_win;
    while(tab->next_tab != NULL) tab = (tab->next_tab);
    tab->next_tab = n_tab;
    n_tab->head = curr_tab->head;
  }
  else // if curr_tab the head
  {
    n_tab->head = n_tab;
    n_tab->l_win = newwin(y-4, x, 2, 0);
    n_tab->r_win = newwin(y-4, x, 2, 0);
  }
  n_tab->l_view = append_view(n_tab->l_win, cfg, pathbuf);
  if(split == 1) n_tab->r_view =
    append_view(n_tab->r_win, cfg, pathbuf);
  else n_tab->r_view = NULL;
  n_tab->sel_view = n_tab->l_view;
  n_tab->query = NULL;
  if(pathbuf != NULL) free(pathbuf);
  return n_tab;
} /* }}} */

/* free_tab_view {{{ */
void
free_tab_view(ListView *view)
{
  free_history(view);
  free_dir_entry(view->dir_entry,
      view->entry_buffer_size);
  free(view->path);
  free(view);
} /* }}} */

/* free_tabs {{{ */
void
free_tabs(Tab *curr_tab)
{
  Tab *tab = curr_tab->head,
      *head = curr_tab->head;

  while(tab->next_tab != NULL)
  {
    tab->sel_view = NULL;
    tab->head = NULL;
    free_tab_view(tab->l_view);
    if(tab->r_view != NULL)
      free_tab_view(tab->r_view);
    else if(tab->query != NULL)
      free_tab_view(tab->query);
    tab = (tab->next_tab);
    free(head);
    head = tab;
  }
  free_tab_view(tab->l_view);
  if(tab->r_view != NULL)
    free_tab_view(tab->r_view);
  else if(tab->query != NULL)
    free_tab_view(tab->query);
  free(tab);
} /* }}} */

/* close_view {{{ */
int
close_view(Tab **tab, cfg_t **cfg, int y, int x)
{
  if( (*tab)->sel_view == (*tab)->r_view)
  {
    (*tab)->sel_view = (*tab)->l_view;
    (*tab)->query = (*tab)->r_view;
    (*tab)->r_view = NULL;
  }
  else if( (*tab)->sel_view == (*tab)->l_view)
  {
    if( (*tab)->r_view == NULL)
    {
      kill_sel_tab(tab, cfg, y, x);
      return 0;
    }
    change_directory((*tab)->l_view,
        cfg, (*tab)->r_view->path);

    (*tab)->query = (*tab)->r_view;
    (*tab)->r_view = NULL;
    (*tab)->sel_view = (*tab)->l_view;
  }
  update_tab_views((*tab), cfg, y, x, W_RESIZE);
  return 1;
} /* }}} */

/* open_new_view {{{ */
int
open_new_view(Tab **curr_tab, cfg_t **cfg, int y, int x)
{
  ListView *view = NULL;
  ListView *curr_view = (*curr_tab)->sel_view;
  int path_len = (strlen(curr_view->path) +
      strlen(curr_view->dir_entry[curr_view->ptr_c]->name) + 2);

  char pathbuf[path_len];

  if(curr_view->dir_entry[curr_view->ptr_c]->is_directory != 1) return -1;
  else if(curr_view->ptr_c == 0)
  {
    snprintf(pathbuf, path_len, "%s", curr_view->path);
    add_path(NULL, pathbuf, path_len);
  }
  else
  {
    build_path(curr_view->path,
        curr_view->dir_entry[curr_view->ptr_c]->name,
        pathbuf, path_len);
  }
  if(access(pathbuf, 04) == -1) return 0;
  else if( (*curr_tab)->r_view != NULL)
  {
    if( (*curr_tab)->sel_view == (*curr_tab)->l_view)
      view = (*curr_tab)->r_view;
    else view = (*curr_tab)->l_view;
    toggle_view( (*curr_tab), cfg, y, x);

    if(change_directory(view, cfg, pathbuf) != -2)
      update_tab_views((*curr_tab), cfg, y, x, W_REDRAW);
  }
  else // if r_view == NULL
  {
    if( (*curr_tab)->query != NULL)
    {
      (*curr_tab)->r_view = prepend_view((*curr_tab)->query, cfg, pathbuf);
      (*curr_tab)->query = NULL;
    }
    else (*curr_tab)->r_view = append_view((*curr_tab)->r_win, cfg, pathbuf);
    toggle_view( (*curr_tab), cfg, y, x);
    update_tab_views( (*curr_tab), cfg, y, x, W_RESIZE);
  }
  return 1;
} /* }}} */

/* switch_to_tab {{{ */
void
switch_to_tab(Tab *tab, Tab *old_tab, cfg_t **cfg,  int y, int x)
{
  int draw_mode = W_REDRAW;

  if( (old_tab != NULL && old_tab->r_view == NULL &&
      tab->r_view != NULL) ||
      (old_tab != NULL && old_tab->r_view != NULL &&
      tab->r_view == NULL) )
    draw_mode = W_RESIZE;

  if(tab->r_view != NULL)
  {
    toggle_view(tab, cfg, y, x);
    toggle_view(tab, cfg, y, x);
  }
  update_tab_views(tab, cfg, y, x, draw_mode);
} /* }}} */

/* switch_to_view {{{ */
void
toggle_view(Tab *tab, cfg_t **cfg, int y, int x)
{
  int master = 0;
  if(tab->sel_view == tab->l_view)
    master = 1;
  show_last_hbar_pos(tab->sel_view, (*cfg)->color, master);

  if(tab->sel_view == tab->l_view)
    tab->sel_view = tab->r_view;

  else tab->sel_view = tab->l_view;

  update_tab_views(tab, cfg, y, x, W_REDRAW);
} /* }}} */

/* prev_tab {{{ */
void
prev_tab(Tab **curr_tab, cfg_t **cfg, int y, int x)
{
  Tab *tab = *curr_tab, *old_tab = *curr_tab;
  if(tab->head->next_tab != NULL)
  {
    if(*curr_tab == ( (*curr_tab)->head))
    {
      while(tab->next_tab != NULL)
        tab = (tab->next_tab);
      *curr_tab = tab;
    }
    else
    {
      tab = ( (*curr_tab)->head);
      while(tab->next_tab != (*curr_tab))
        tab = (tab->next_tab);
      *curr_tab = tab;
    }
    switch_to_tab(*curr_tab, old_tab, cfg, y, x);
  }
} /* }}} */

/* next_tab {{{ */
void
next_tab(Tab **curr_tab, cfg_t **cfg, int y, int x)
{
  Tab *tab = *curr_tab, *old_tab = *curr_tab;
  if(tab->head->next_tab != NULL)
  {
    if(tab->next_tab != NULL)
      *curr_tab = (tab->next_tab);
    else
    {
      if(*curr_tab != ( (*curr_tab)->head))
        *curr_tab = ( (*curr_tab)->head);
    }
    switch_to_tab(*curr_tab, old_tab, cfg, y, x);
  }
} /* }}} */

/* kill_sel_tab {{{ */
void
kill_sel_tab(Tab **curr_tab, cfg_t **cfg, int y, int x)
{
  int success = 0;
  Tab *target_tab = *curr_tab,
      *tab = (*curr_tab)->head;

  Tab *old_tab = (*curr_tab);

  if( (*curr_tab)->head == *curr_tab)
  {
    if( (*curr_tab)->next_tab != NULL)
    {
      *curr_tab = (*curr_tab)->next_tab;
      (*curr_tab)->head = *curr_tab;
      
      tab = (*curr_tab)->head;
      while(tab->next_tab != NULL)
      {
        tab->head = *curr_tab;
        tab = (tab->next_tab);
        if(tab->next_tab == NULL)
          tab->head = *curr_tab;
      }
      success = 1;
    }
  }
  else
  {
    while(tab->next_tab != *curr_tab)
      tab = (tab->next_tab);

    if(target_tab->next_tab != NULL)
      tab->next_tab = target_tab->next_tab;
    else tab->next_tab = NULL;

    if(tab->next_tab != NULL)
      *curr_tab = tab->next_tab;
    else *curr_tab = tab;

    success = 1;
  }
  if(success != 0)
  {
    switch_to_tab(*curr_tab, old_tab, cfg, y, x);

    free_tab_view(target_tab->l_view);
    if(target_tab->r_view != NULL)
      free_tab_view(target_tab->r_view);
    else if(target_tab->query != NULL)
      free_tab_view(target_tab->query);
    free(target_tab);
  }
} /* }}} */

/* update_tab_bar {{{ */
void
update_tab_bar(Tab *curr_tab, cfg_t *cfg, int y, int x, WINDOW *tab_win)
{
  int exit = 0;
  int bar_len;
  char *dirname, dnamebuf[255], *sep = ":";
  size_t dname_len;

  int tab_c = 1, x_pos = 0;

  Tab *t_head = curr_tab->head, *t_sel = curr_tab;

  wattrset(tab_win, COLOR_PAIR(TABVIEW_BAR)|cfg->color[TABVIEW_BAR].attr);
  for(bar_len = 0; bar_len < x; bar_len++)
    mvwprintw(tab_win, 0, bar_len, "%c", '\040');

  while(!exit)
  {
    dirname = conv_filename(t_head->sel_view->path);
    dname_len = wnstrlen(dirname, cfg->max_tablen);
//      ret_utf8_str_column_sum(dirname, cfg->max_tablen);

    if(dname_len == 0) sep = "";
    snprintf(dnamebuf, dname_len+strlen(sep), "%s%s", sep, dirname);

    if(t_head == t_sel)
      wattrset(tab_win, COLOR_PAIR(TABVIEW_SEL)|cfg->color[TABVIEW_SEL].attr);
    else wattrset(tab_win, COLOR_PAIR(TABVIEW)|cfg->color[TABVIEW].attr);

    mvwprintw(tab_win, 0, x_pos, " %d%s ", tab_c, dnamebuf);
    if(t_head->next_tab == NULL) exit = 1;

    if( (tab_c) >= 10) x_pos++;

    x_pos += (ret_utf8_str_column_sum(dnamebuf, dname_len))+2+strlen(sep);
//    x_pos += dname_len+3+strlen(sep);
    t_head = t_head->next_tab;
    tab_c++;
  }
} /* }}} */

/* update_path_bar {{{ */
void
update_path_bar(Tab *curr_tab, Color *color, WINDOW *win)
{
  int path_len = 0;
  int cols = 0;
  int x = getmaxx(win);
  int maxx = x;
  int diff = 0;

  wattrset(win, COLOR_PAIR(BORDER));
  for(; cols < x; cols++)
    mvwprintw(win, 1, cols, "%c", '\040');
  
  if(curr_tab->r_view != NULL)
  {
    x >>= 1;
    if( (x << 1) == maxx)
      diff = 1;
    if(curr_tab->r_view == curr_tab->sel_view)
      wattrset(win, COLOR_PAIR(PATH_SEL)|color[PATH_SEL].attr);
    else wattrset(win, COLOR_PAIR(PATH)|color[PATH].attr);

    path_len = strlen(curr_tab->r_view->path)+1; // +1 '\0' !
    char rpath[path_len];

    if(path_shorten(curr_tab->r_view->path, rpath, path_len, x-1) != 0)
      mvwprintw(win, 1, x+1-diff, "%s", rpath);
    else mvwprintw(win, 1, x+1-diff, "NULL");
  }
  else diff++;
  if(curr_tab->l_view == curr_tab->sel_view)
    wattrset(win, COLOR_PAIR(PATH_SEL)|color[PATH_SEL].attr);
  else wattrset(win, COLOR_PAIR(PATH)|color[PATH].attr);

  path_len = strlen(curr_tab->l_view->path)+1;
  char lpath[path_len];

  if(path_shorten(curr_tab->l_view->path, lpath, path_len,  x-diff-1) != 0)
    mvwprintw(win, 1, 1, "%s", lpath);
  else mvwprintw(win, 1, 1, "NULL");

  wattroff(win, COLOR_PAIR(BORDER));
} /* }}} */

/* first_start {{{ */
void
first_start(Tab *tab, cfg_t **cfg, int scr_y, int scr_x)
{
  int diff = 0;
  int split_x = scr_x;
  if( (*cfg)->start_split == 1)
  {
    split_x = (scr_x >> 1);
    if( (split_x << 1) == (scr_x -1) )
      diff = 1;

    if(diff != 0)
      wresize(tab->l_view->win, scr_y-4, split_x+diff);
    mvwin(tab->r_view->win, 2, split_x+diff);

    wbkgd(tab->r_view->win, COLOR_PAIR(WIN));
    show_list(tab->r_view, (*cfg)->color, 0);
    print_borders(tab->r_view, 0);
    show_last_hbar_pos(tab->r_view, (*cfg)->color, 0);
    (*cfg)->start_split = 0;
  }
  wbkgd(tab->l_view->win, COLOR_PAIR(WIN));
  show_list(tab->l_view, (*cfg)->color, 1);
  print_borders(tab->l_view, 1);

  update_tab_bar(tab, *cfg, scr_y, scr_x, st_win);
  update_path_bar(tab, (*cfg)->color, st_win);
  show_file_bar(tab->l_view, scr_y, scr_x, st2_win, DEEP_OFF);
  show_cmd_bar(tab->l_view, cfg, scr_x, scr_y, st2_win, NULL);
} /* }}} */

/* resize_windows {{{ */
void
resize_windows(Tab *tab, int scr_y, int scr_x)
{
  int diff = 0;
  int split_x = scr_x;

  werase(stdscr);
  werase(st_win);
  werase(st2_win);
  if(tab->r_view != NULL)
    werase(tab->r_view->win);
  werase(tab->l_view->win);

  wresize(st_win, 2, scr_x);
  wresize(st2_win, 2, scr_x);
  mvwin(st2_win, scr_y-2, 0);

  if(tab->r_view != NULL)
  {
    split_x = (scr_x >> 1);
    if( (split_x << 1) == (scr_x -1) )
      diff = 1;

    wresize(tab->r_view->win, scr_y-4, split_x);
    mvwin(tab->r_view->win, 2, split_x+diff);
  }
  wresize(tab->l_view->win, scr_y-4, split_x+diff);
  mvwin(tab->l_view->win, 2, 0);

} /* }}} */

/* update_windows {{{ */
// um = update_mode
void
update_tab_views(Tab *tab, cfg_t **cfg, int scr_y, int scr_x, bool resize)
{
  int master = 0;

  if(resize == true)
  {
    resize_windows(tab, scr_y, scr_x);

    if(tab->r_view != NULL)
    {
      toggle_view(tab, cfg, scr_y, scr_x);
      toggle_view(tab, cfg, scr_y, scr_x);
    }
  }
  else werase(tab->sel_view->win);
  werase(st_win);
  werase(st2_win);

  check_limits(tab->sel_view);
  
  if(tab->sel_view == tab->l_view)
    master = 1;

  update_tab_bar(tab, *cfg, scr_y, scr_x, st_win);
  update_path_bar(tab, (*cfg)->color, st_win);

  show_list(tab->sel_view, (*cfg)->color, master);
  print_borders(tab->sel_view, master);

  show_file_bar(tab->sel_view, scr_y, scr_x, st2_win, DEEP_OFF); 
  show_cmd_bar(tab->sel_view, cfg, scr_x, scr_y, st2_win, NULL);

  tab->sel_view->update_list = DONE;
} /* }}} */
