/* utils.h by lutherus 2013 */

#ifndef __UTILS_H__
#define __UTILS_H__

#define DIRUP 0
#define REGDOT 1
#define NODOT 2
#define CWD_DOT 3

/* prototypes {{{ */
void add_path(char *file, char *path_buf, int max_size);
void build_path(char *curr_path, char *entry_name, char *outp, int outp_s);
////////////////////////////////////////////////////////
char *conv_filename(char *filepath);
char **malloc_char_array(int raw, int col);
char **realloc_char_array(char **ptr, int *raw, int col);
///////////////////////////////////
int free_char_array(char **field, int raw);
int ret_suitable_string(char *s1, const char *s2, const char endmark);
int ret_cypher(int current_value, int new_value);
int wnstrlen(char *str, int max_column);
int wnstrunc(char *str, int trnc, int x);
int ret_utf8_str_column_sum(char *file, int max_len);
int calc_utf8_truncate_size(char *file, int *mb_strlen, int max_str_outp);
int mbwidth(const char *c);
int path_shorten(char *path, char *outp, int outp_len, int x);
int ret_dir_level(char *path);
void termstr(char *str);
int ret_dot_type(char *dname); /* }}} */

#endif
