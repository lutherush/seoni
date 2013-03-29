/* seoni.c by lutherus 2012 */

/* includes {{{ */
#include "seoni.h"
#include "filedata.h"
#include "config.h"
#include "sfbui.h"
#include "tab.h"
#include "sort.h"
#include "keys.h"
#include "search.h"
#include "utils.h"
#include <locale.h>
#include<termios.h> /* struct winsize */
#include<sys/ioctl.h> /* }}} */

/* init_sfbgui {{{ */
void init_sfbgui()
{
  setlocale(LC_ALL, "");
  initscr();
  clear();
  noecho();
  curs_set(0);
  cbreak();
  nl();
  keypad(stdscr, TRUE);
  if(has_colors() == FALSE)
  {
    endwin();
    printf("Your terminal does not support color!\n");
    system("clear");
    exit(1);
  }
  start_color();
} /* }}} */

/* free_all_data {{{ */
void
free_all_data(Tab *curr_tab, cfg_t *cfg, bool error)
{
  delwin(st_win);
  delwin(st2_win);
  delwin(curr_tab->head->l_win);
  delwin(curr_tab->head->r_win);
  endwin();

  free_tabs(curr_tab);

  free_cfg_t(cfg);
  system("clear");

  if(error == true)
    printf("terminal is to small\n");
}
/* }}} */

/* refresh_all {{{ */
void
refresh_all(Tab *tab)
{
  refresh();
  if(tab->r_view != NULL)
    wnoutrefresh(tab->r_view->win);
  if(tab->l_view != NULL)
    wnoutrefresh(tab->l_view->win);
  wnoutrefresh(st_win);
  wnoutrefresh(st2_win);

  doupdate();
} /* }}} */
    
/* main {{{ */
int
main(void)
{
  int quit = 0, error = 0;
  int scr_y = 0, scr_x = 0, split_x = 0;
  struct winsize ws;
  cfg_t *cfg = NULL;

  init_sfbgui();
  if( (init_cfg_t(&cfg)) == -1)
  {
    printf("not enouth space available\n");
    return EXIT_SUCCESS;
  }
  init_config(&cfg, FIRSTLOAD);

  scr_x = getmaxx(stdscr);
  scr_y = getmaxy(stdscr);
  if(cfg->start_split == 1)
    split_x = (scr_x >> 1);
  else split_x = scr_x;

  Tab *tab = NULL;
  tab = create_new_tab(tab,
      &cfg, scr_y, split_x, cfg->start_split, &error);

  bkgd(COLOR_PAIR(WIN));
  init_state_win(scr_y, scr_x);
  wbkgd(st_win, COLOR_PAIR(WIN));

  first_start(tab, &cfg, scr_y, scr_x);

  mouseinterval(0);
  mousemask(ALL_MOUSE_EVENTS, NULL);

  cmd_line.mode = 0;
  cmd_line.count = 0;
  k_scut.value = 0;

  refresh_all(tab);

  chtype new_key;
  //MEVENT mouse;
  while(!quit)
  {
    new_key = getch();
    if(new_key == KEY_RESIZE && 
        is_term_resized(scr_y, scr_x))
    {
      if(ioctl(0, TIOCGWINSZ, &ws) == 0)    
        resize_term(ws.ws_row, ws.ws_col);
      else break;
      getmaxyx(stdscr, scr_y, scr_x);

      if(scr_x < 30 || scr_y < 6)
      {
        free_all_data(tab, cfg, RESIZE_ERROR);
        return EXIT_SUCCESS;
      }
      update_tab_views(tab, &cfg, scr_y, scr_x, W_RESIZE);
    }
    if(new_key != KEY_RESIZE)
      quit = filter_keystroke(&tab, &cfg,
          scr_x, scr_y, new_key);

    if(tab->sel_view->update_list == UPDATE)
      update_tab_views(tab, &cfg, scr_y, scr_x, W_REDRAW);

    refresh_all(tab);
  }
  free_all_data(tab, cfg, CLEARLY);
  return EXIT_SUCCESS;
} /* }}} */


