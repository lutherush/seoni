/* search.c by lutherus 2012 */

#include "seoni.h"
#include "filedata.h"
#include "utils.h"
#include "config.h"
#include "sfbui.h"
#include "history.h"

/* free_history {{{ */
void
free_history(ListView *view)
{
  Dir *dir = NULL, *last_dir = NULL;
  int idx = 0;

  for(; idx < 100; idx++)
  {
    dir = (view->dir_hist[idx].dir);
    if(dir == NULL) continue;
    else dir = (dir->head);
    while(dir != NULL)
    {
      free(dir->filename);
      free(dir->path);
      dir->head = NULL;
      
      last_dir = dir;
      dir = (dir->next_dir);
      free(last_dir);
    }
  }
} /* }}} */

/* zero_history {{{ */
void
zero_history(ListView *view)
{
  Dir *dir = NULL;
  int idx = 0;

  for(; idx < 100; idx++)
  {
    dir = (view->dir_hist[idx].dir);
    if(dir == NULL) continue;
    else dir = (dir->head);
    while(dir != NULL)
    {
      dir->filename[0] = '\0';
      dir->pos = 0;
      dir->scrolled = 0;

      dir = (dir->next_dir);
    }
  }
} /* }}} */

/* append_dir_hist {{{ */
Dir
*append_dir_hist(ListView *view, int level)
{
  int pathsize = (strlen(view->path) * sizeof(char)+1);
  int filesize = (strlen(view->dir_entry[(view->ptr_c)]->name)
      * sizeof(char)+1);

  Dir *curr_dir = view->dir_hist[level].dir;

  Dir *n_dir = (Dir *)malloc(sizeof(Dir));
  if(n_dir == NULL) return NULL;

  n_dir->path = (char *)malloc(pathsize);
  if(n_dir->path == NULL)
  {
    free(n_dir);
    return NULL;
  }
  else snprintf(n_dir->path, pathsize, "%s", view->path);
  n_dir->filename = (char *)malloc(filesize);
  if(n_dir->filename == NULL)
  {
    free(n_dir->path);
    free(n_dir);
    return NULL;
  }
  else snprintf(n_dir->filename, filesize, "%s",
      view->dir_entry[(view->ptr_c)]->name);

  n_dir->pos = view->ptr_c;
  n_dir->scrolled = view->scroll_c;
  n_dir->next_dir = NULL;

  if(curr_dir == NULL)
    n_dir->head = n_dir;
  else
  {
    n_dir->head = curr_dir->head;
    curr_dir->next_dir = n_dir;
  }
  return n_dir;
} /* }}} */

/* reinit_dir_pos {{{ */
int reinit_dir_pos(Dir *curr_dir, ListView *view)
{
  char *curr_filename =
    (view->dir_entry[(view->ptr_c)]->name);

  int filesize =
    (strlen(curr_filename) * sizeof(char)+1);

  if(strcmp(curr_dir->filename, curr_filename) != 0)
  {
    curr_dir->filename = 
      (char *)realloc(curr_dir->filename, filesize);
    if(curr_dir->filename == NULL) return 0;

    snprintf(curr_dir->filename, filesize, "%s", curr_filename);
  }
  curr_dir->pos = view->ptr_c;
  curr_dir->scrolled = view->scroll_c;
  return 1;
} /* }}} */

/* set_last_dir_pos {{{ */
int
set_last_dir_pos(ListView *view)
{
  int level = ret_dir_level(view->path);
  if(level > 100) return 0;

  Dir *curr_dir = (view->dir_hist[level].dir);
  if(curr_dir == NULL)
  {
    view->dir_hist[level].dir = append_dir_hist(view, level);
    return 1;
  }
  Dir *same_dir = NULL, *search_dir = (curr_dir->head);

  while(search_dir != NULL)
  {
    if(strcmp(search_dir->path, view->path) == 0)
    {
      same_dir = search_dir;
      break;
    }
    search_dir = (search_dir->next_dir);
  }
  if(same_dir == NULL)
  {
    view->dir_hist[level].dir = append_dir_hist(view, level);
    return 1;
  }
  else reinit_dir_pos(same_dir, view);
  return 1;
} /* }}} */ 

/* ret_last_dir_pos {{{ */
int
ret_last_dir_pos(ListView *view, int *scrolled)
{
  int level = ret_dir_level(view->path);
  if(level > 100) return 0;
  Dir *dir = view->dir_hist[level].dir;

  if(dir == NULL) return 0;
  else if(dir != (dir->head)) dir = (dir->head);
  while(dir != NULL)
  {
    if(strcmp(dir->path, view->path) == 0)
      break;
    dir = (dir->next_dir);
  }
  if(dir == NULL) // has no history entry
  {
    *scrolled = 0;
    return 0;
  }
  else if(strcmp(view->dir_entry[(dir->pos)]->name,
        dir->filename) == 0)
  {
    *scrolled = dir->scrolled;
    return dir->pos;
  }
  else return search_dir_pos(view, dir->filename);
} /* }}} */

/* search_dir_pos {{{ */
int
search_dir_pos(ListView *view, char *target_file)
{
  int idx = 0, match = 0;

  for(; idx < view->dir_entry[0]->Info->file_count; idx++)
  {
    if(strcmp(target_file, view->dir_entry[idx]->name) == 0)
    {
      match = idx;
      break;
    }
  }
  return match;
} /* }}} */
