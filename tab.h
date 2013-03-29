/* tab.h by lutherus 2013 */

#ifndef __TAB_H__
#define __TAB_H__

#define W_RESIZE true
#define W_REDRAW false

/* tab {{{ */
typedef struct __tab
{
  WINDOW *l_win;
  WINDOW *r_win;

  ListView *l_view;
  ListView *r_view;

  ListView *sel_view;
  ListView *query;

  struct __tab *next_tab;
  struct __tab *head;

} Tab; /* }}} */

/* prototypes {{{ */

/* tabs */
Tab *create_new_tab(Tab *curr_tab, cfg_t **cfg, int y, int x, int split, int *error);
void free_tab_view(ListView *view);
void free_tabs(Tab *curr_tab);
void switch_to_tab(Tab *tab, Tab *old_tab, cfg_t **cfg, int y, int x);
void prev_tab(Tab **tab, cfg_t **cfg, int y, int x);
void next_tab(Tab **tab, cfg_t **cfg, int y, int x);
void kill_sel_tab(Tab **curr_tab, cfg_t **cfg, int y, int x);

/* views */
int close_view(Tab **tab, cfg_t **cfg, int y, int x);
int open_new_view(Tab **curr_tab, cfg_t **cfg, int y, int x);
void toggle_view(Tab *tab, cfg_t **cfg, int y, int x);

/* resizing, organisation, output */
void update_tab_bar(Tab *curr_tab, cfg_t *cfg, int y, int x, WINDOW *tab_win);
void update_path_bar(Tab *curr_tab, Color *color, WINDOW *win);
void first_start(Tab *tab, cfg_t **cfg, int scr_y, int scr_x);
void resize_windows(Tab *tab, int scr_y, int scr_x);
void update_tab_views(Tab *tab, cfg_t **cfg, int y, int x, bool resize); /* }}} */

#endif
