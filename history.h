/* search.h by lutherus 2012  */

#ifndef __SEARCH_H__
#define __SEARCH_H__

/* prototypes {{{ */
Dir *append_dir_hist(ListView *view, int level);
void free_history(ListView *view);
void zero_history(ListView *view);
int reinit_dir_pos(Dir *dir, ListView *view);
int set_last_dir_pos(ListView *view);
int ret_last_dir_pos(ListView *view, int *scrolled);
int search_dir_pos(ListView *view, char *target_file);

#endif
