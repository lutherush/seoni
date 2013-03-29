/* sort.h by lutherus 2013 */
#ifndef __SORT_H__
#define __SORT_H__

/* macros {{{ */
#define TRUE 1
#define FALSE 0 /* }}} */

/* Sortoption {{{ */
enum Sortoption
{
  ASCENDING = 1,
  DESCENDING,
  NAME_SORT = 0,
  SIZE_SORT,
  MTIME_SORT,
  ATIME_SORT,
  CTIME_SORT
}; /* }}} */

/* prototypes {{{ */
int divide_dirs_and_nfiles(Dir_Entry ***dir_entry);
///////////////
void swap_pointer(Dir_Entry ***dir_entry, int left, int right);
void inverse_sort(Dir_Entry ***dir_entry);
void sort_files(Dir_Entry ***dir_entry, int so, int sm, char *path);
void q_sort(Dir_Entry ***dir_entry, int so, int sm, int l, int r); /* }}} */

#endif
