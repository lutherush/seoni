/* utils.c by lutherus 2013 */
#define _XOPEN_SOURCE 650
#include <wchar.h>
#include <assert.h>
#include "seoni.h"
#include "filedata.h"
#include "config.h"
#include "utils.h"

/* add_path {{{ */
void
add_path(char *file, char *path_buf, int max_size)
{
  int level = 0;
  char *path_ptr;

  if(file != NULL)
  {
    if(strlen(path_buf) != 1)
      strncat(path_buf, "/", max_size);

    strncat(path_buf, file, max_size);
    path_ptr = path_buf;
    path_ptr[(strlen(path_buf))] = '\0';
  }
  else
  {
    if( (level =
          ret_dir_level(path_buf)) > 1)
    {
      path_ptr = strrchr(path_buf, '/');
      *path_ptr = '\0';
    }
    else snprintf(path_buf, max_size, "/");
  }
} /* }}} */

/* ret_dir_level {{{ */
int
ret_dir_level(char *path)
{
  int level = 0, path_cc;

  for(path_cc = 0; path_cc
      < strlen(path); path_cc++)
  {
    if(path[path_cc] == '/')
      level++;
  }
  return level;
} /* }}} */

/* build_path {{{ */
void
build_path(char *curr_path, char *entry_name, char *outp, int outp_s)
{
  char *slash_buf;

  if(strlen(curr_path) < 2)
    slash_buf = "";
  else slash_buf = "/";

  snprintf(outp, outp_s, "%s%s%s",
      curr_path, slash_buf, entry_name);
} /* }}} */

/* *conv_filename {{{ */
char
*conv_filename(char *filepath)
{
  char *filename_begin, *filename;
  int path_len;

  filename_begin = strrchr(filepath, '/');

  if( (path_len = strlen(filepath)) > 1)
    filename_begin++;

  filename = filename_begin;
  return filename;
} /* }}} */

/* ret_suitable_string {{{ */
int
ret_suitable_string(char *s1, const char *s2, const char endmark)
{
  int idx = 0, s1_len = 0, s2_len = strlen(s2);

  if(endmark == 0) s1_len = strlen(s1);
  else
  {
    while(s1[s1_len] != endmark)
    {
      if(s1[s1_len] != '\0') s1_len++;
      else break;
    }
  }
  for(idx = s1_len; idx > 0; idx--)
  {
    if(idx > s2_len) return 0;
    else if(strncmp(s1, s2, idx) != 0) s1_len--;
  }
  if(s1_len == s2_len) return 1;
  else return 0;
} /* }}} */

/* convert_cypher {{{ */
/* use 48 = 0 to become a true value 0-9 */
int
ret_cypher(int current_value, int new_value)
{
  char cypher_buf[20];

  snprintf(cypher_buf, 20, "%d%d",
      current_value, new_value-48);

  return atoi(cypher_buf);
} /* }}} */

/* wnstrlen {{{ */
int
wnstrlen(char *str, int max_column)
{
  if(str == NULL) return 0;
  else if(str[0] == '\0') return 0;
  else if(max_column == 0) return 0;

  // next byte
  int nb = 0;
  int size = 0;
  int width = 0;
  int idx = 0;
  int len = strlen(str);
  int mb_strlen = mbstowcs(NULL, str, MB_CUR_MAX);
  char *p = str;

  if(len == mb_strlen) return max_column;

  for(; idx < len; idx++)
  {
    nb = mblen(p, MB_CUR_MAX);
    if(nb != -1)
    {
      width = mbwidth(p);
      if( (max_column - width) >= 0)
      {
        max_column -= width;
        size += nb;
      }
      else break;
    }
    p++;
  } // +1 == '\0'
  return size+1;
} /* }}} */

/* wnstrunc {{{ */
int
wnstrunc(char *str, int trnc, int x)
{
  if(str == NULL) return 0;
  else if(str[0] == '\0') return 0;

  int wlen = 0;
  int idx = 0;
  int width = 0;
  int len = strlen(str);
  char *p = str;

  for(; idx < len; idx++)
  {
    if(trnc == 0) break;
    width = mblen(p, MB_CUR_MAX);
    if(width != -1)
    {
      wlen += width;
      trnc--;
    }
    if(ret_utf8_str_column_sum(p, len) < x)
      break;
    p++;
  }
  return wlen;
} /* }}} */

/* ret_utf8_mblen {{{ */
int
ret_utf8_str_column_sum(char *file, int max_len)
{
  char *p = file;
  int c = 0, file_len = strlen(file);
  int mb_s = 0, mb_len = 0, width = 0;
  int mc = max_len;

  for(c = 0; c < file_len; c++)
  {
    if(mc == 0) break;
    width = mbwidth(p);
    mb_s = mblen(p, MB_CUR_MAX);
    if(mb_s != -1)
    {
      mb_len += width;
      mc--;
    }
    p++;
  }
  return mb_len;
} /* }}} */

/* calc_truncate_size {{{ */
int
calc_utf8_truncate_size(char *file, int *mb_strlen, int max_str_outp)
{
  char *p = file;

  int c = 0, need_size = 0, mb_s = 0, width = 0;
  int file_len = strlen(file);

  int match = 0;

  if(file_len == *mb_strlen) return max_str_outp;
  else if(file_len > *mb_strlen)
  {
    for(c = 0; c < file_len; c++)
    {
      width = mbwidth(p);
      mb_s = mblen(p, MB_CUR_MAX);
      if(mb_s != -1)
      {
        if((match + width) >= max_str_outp)
          break;
        else match += width;
        if(mb_s > 0) need_size += mb_s;
        else if(mb_s == 0) need_size++;
      }
      p++;
    }
    *mb_strlen = match;
  }
  if(match > 0) return need_size+1;
  else return max_str_outp;
} /* }}} */

/* mbwidth {{{ */
int
mbwidth(const char *c)
{
  assert(c != NULL);

  wchar_t wc, bad_wchar = 0xFFFD;
  int width;

  if(mbtowc(&wc, c, MB_CUR_MAX) < 0)
  {
    mbtowc(NULL, NULL, 0);
    wc = bad_wchar;
  }
  width = wcwidth(wc);

  if(width == -1)
  {
    wc = bad_wchar;
    width = wcwidth(wc);
  }
  return width;
} /* }}} */

/* path_shorten if p > home_dir or > given size {{{ */
int
path_shorten(char *path, char *outp, int outp_len, int x)
{
  if(path == NULL) return 0;
  else if(path[0] == '\0' || x == 0) return 0;

  int diff = 0;
  int wlen = 0;
  int home_len = 0;
  int path_len = strlen(path);
  char *home = getenv("HOME");
  char *p = NULL;

  if(home == NULL) return 0;
  else home_len = strlen(home);

  if(path[path_len] != '\0') return 0;
  p = path;

  if(path_len > home_len &&
      strncmp(path, home, home_len) == 0)
  {
    p += home_len;
    wlen = ret_utf8_str_column_sum(p, strlen(p));
    if(wlen >= x)
    {
      diff = (wlen - x);
      wlen = wnstrunc(p, (diff+3), x-2);
      if(wlen == 0) return 0;
      p += wlen;
      snprintf(outp, outp_len, "...%s", p);
    }
    else snprintf(outp, outp_len, "~%s", p);
    return 1;
  }
  wlen = ret_utf8_str_column_sum(p, strlen(p));
  if(wlen >= x)
  {
    diff = (wlen - x);
    wlen = wnstrunc(p, (diff+3), x-2);
    if(wlen == 0) return 0;
    p += wlen;
    snprintf(outp, outp_len, "...%s", p);
  }
  else snprintf(outp, outp_len, "%s", p);
  return 1;
} /* }}} */

/* free_char_array {{{ */
int
free_char_array(char **field, int raw)
{
  int idx = raw;

  if(field == NULL) return 0;
  for(; idx >= 0; idx--)
    free(field[idx]);
  free(field);
  return 1;
} /* }}} */

/* malloc_char_array {{{ */
char
**malloc_char_array(int raw, int col)
{
  int idx = 0;
  char **new = NULL;

  new = (char **)malloc(raw * sizeof(*new));
  if(new != NULL)
  {
    for(; idx < raw; idx++)
    {
      new[idx] = (char *)malloc(col * sizeof(*new[idx]));
      if(new[idx] == NULL)
      {
        free_char_array(new, idx-1);
        break;
      }
      else new[idx][0] = '\0';
    }
    if(idx == raw) return new;
    else return NULL;
  }
  else return NULL;
} /* }}} */

/* realloc_char_array {{{ */
char
**realloc_char_array(char **ptr, int *raw, int col)
{
  int idx = 0, raw_tmp = (*raw);

  raw_tmp += 10;
  if(ptr == NULL) return NULL;
  ptr = (char **)realloc(ptr, raw_tmp * sizeof(*ptr));
  if(ptr == NULL) return NULL;
  for(idx = (*raw); idx < raw_tmp; idx++)
  {
    ptr[idx] = (char *)malloc(col * sizeof(*ptr[idx]));
    if(ptr[idx] == NULL)
    {
      free_char_array(ptr, idx-1);
      break;
    }
    else ptr[idx][0] = '\0';
  }
  if(idx == raw_tmp)
  {
    (*raw) = raw_tmp;
    return ptr;
  }
  else return NULL;
} /* }}} */

/* termstr {{{ */
void
termstr(char *str)
{
  int strl = strlen(str);
  str[strl-1] = '\0';
} /* }}} */

/* ret_dot_type {{{ */
int
ret_dot_type(char *dname)
{
  int file_len = strlen(dname);

  if(dname[0] == '.')
  {
    if(file_len == 2 && dname[1] == '.')
      return DIRUP;
    else if(file_len == 1) return CWD_DOT;
    else return REGDOT;
  }
  else return NODOT;
} /* }}} */
