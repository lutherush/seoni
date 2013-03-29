/* sort.c by lutherus 2013  */

/* includes {{{ */
#include "seoni.h"
#include <ctype.h>
#include "filedata.h"
#include "sort.h"
#include "config.h" /* }}} */

/* change_list {{{ */
void
swap_pointer(Dir_Entry ***dir_entry, int left, int right)
{
  Dir_Entry *temp;
  temp = (*(*dir_entry+left));
  (*(*dir_entry+left)) = (*(*dir_entry+right));
  (*(*dir_entry+right)) = temp;
} /* }}} */

/* inverse_list {{{ */
/* good for quick ascending, descending */
void
inverse_sort(Dir_Entry ***dir_entry)
{
  int idx = 1, last_dir = 1, nfile_count = 0,
      file_count = (*(*dir_entry))->Info->file_count;
  while( (*(*dir_entry+idx))->is_directory)
  {
    last_dir++;
    idx++;
    if(idx == (file_count)) break;
  }
  idx = 1;
  while(idx <= (last_dir >> 1) &&
      (*(*dir_entry+idx))->is_directory != 0)
  {
    swap_pointer(dir_entry, idx, (last_dir-idx));
    idx++;
  }
  idx = 1;
  nfile_count = ( (file_count - last_dir) >> 1);
  for(; idx <= nfile_count; idx++, last_dir++)
    swap_pointer(dir_entry, last_dir, file_count-idx);
} /* }}} */

/* divide_dirs_and_nfiles {{{ */
int
divide_dirs_and_nfiles(Dir_Entry ***dir_entry)
{
  int dir_c = 1, idx = 1;
  for(; idx < (*(*dir_entry))->Info->file_count; idx++)
  {
    if( (*(*dir_entry+idx))->file_t == IS_DIRECTORY)
    {
      swap_pointer(dir_entry, dir_c, idx);
      dir_c++;
    }
  }
  return dir_c;
} /* }}} */

/* sort_files {{{ */
void
sort_files(Dir_Entry ***dir_entry, int so, int sm, char *path)
{
  int last_dir = 0;

  last_dir = divide_dirs_and_nfiles(dir_entry);

  q_sort(dir_entry, so, sm, 1, last_dir-1);
  if( (*(*dir_entry))->Info->file_count > last_dir)
    q_sort(dir_entry, so, sm, last_dir, (*(*dir_entry))->Info->file_count-1);
} /* }}} */

/* version_sort {{{ */
void
q_sort(Dir_Entry ***dir_entry, int so, int sm, int l, int r)
{
  int left = l, right = r;
  Dir_Entry *center = (*(*dir_entry+( (left + right) >> 1)));
  do
  {
    switch(so)
    {
      case NAME_SORT:
        if(sm == ASCENDING)
        {
          while(strcasecmp( (*(*dir_entry+left))->name, center->name)
              < 0 && left < r) left++;
          while(strcasecmp( (*(*dir_entry+right))->name, center->name)
              > 0 && right > l) right--;
        }
        else // DESCENDING
        {
          while(strcasecmp( (*(*dir_entry+left))->name, center->name)
              > 0 && left < r) left++;
          while(strcasecmp( (*(*dir_entry+right))->name, center->name)
              < 0 && right > l) right--;
        }
        break;

      case SIZE_SORT:
        if(sm == ASCENDING)
        {
          while((left < r) &&
              ( (*(*dir_entry+left))->size < center->size)) left++;
          while((right > l) &&
              ( (*(*dir_entry+right))->size > center->size)) right--;
        }
        else // DESCENDING
        {
          while((left < r) &&
              ( (*(*dir_entry+left))->size > center->size)) left++;
          while((right > l) &&
              ( (*(*dir_entry+right))->size < center->size)) right--;
        }
        break;

      case MTIME_SORT:
        if(sm == ASCENDING)
        {
          while((left < r) &&
              ( (*(*dir_entry+left))->mtime < center->mtime)) left++;
          while((right > l) &&
              ( (*(*dir_entry+right))->mtime > center->mtime)) right--;
        }
        else // DESCENDING
        {
          while((left < r) &&
              ( (*(*dir_entry+left))->mtime > center->mtime)) left++;
          while((right > l) &&
              ( (*(*dir_entry+right))->mtime < center->mtime)) right--;
        }
        break;
      
      case ATIME_SORT:
        if(sm == ASCENDING)
        {
          while((left < r) &&
              ( (*(*dir_entry+left))->atime < center->atime)) left++;
          while((right > l) &&
              ( (*(*dir_entry+right))->atime > center->atime)) right--;
        }
        else // DESCENDING
        {
          while((left < r) &&
              ( (*(*dir_entry+left))->atime > center->atime)) left++;
          while((right > l) &&
              ( (*(*dir_entry+right))->atime < center->atime)) right--;
        }
        break;
      
      case CTIME_SORT:
        if(sm == ASCENDING)
        {
          while((left < r) &&
              ( (*(*dir_entry+left))->ctime < center->ctime)) left++;
          while((right > l) &&
              ( (*(*dir_entry+right))->ctime > center->ctime)) right--;
        }
        else // DESCENDING
        {
          while((left < r) &&
              ( (*(*dir_entry+left))->ctime > center->ctime)) left++;
          while((right > l) &&
              ( (*(*dir_entry+right))->ctime < center->ctime)) right--;
        }
        break;
    }
    if(left <= right)
    {  
      swap_pointer(dir_entry, left, right);
      left++, right--;
    }
  }
  while(left <= right);
  if(l < right) q_sort(dir_entry, so, sm, l, right);
  if(r > left) q_sort(dir_entry, so, sm, left, r);
} /* }}} */
