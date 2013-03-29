/* config.c by lutherus 2013 */

/* includes {{{ */
#include "seoni.h"
#include <unistd.h>
#include <sys/stat.h>
#include <ctype.h>
#include "filedata.h"
#include "sort.h"
#include "utils.h"
#include "config.h" /* }}} */

#define NAME_SORT 0
#define SIZE_SORT 1
#define MTIME_SORT 2

/* config_sections {{{ */
char *config_section[] =
{
  "theme_name",
  "default_bg",
  "sort_option",
  "start_split",
  "always_split",
  "redraw_delay",
  "max_tablen"
}; /* }}} */

/* ui.color {{{ */
char *ui_color[] =
{
  "null",
  "win",
  "cmdbar",
  "border",
  "directory",
  "fslash",
  "symlink",
  "blk_device",
  "device",
  "socket",
  "nfile",
  "select",
  "current",
  "tabview",
  "tabview_sel",
  "tabview_bar",
  "executable",
  "path",
  "path_sel",
  "error_message"
}; /* }}} */

/* default_colors {{{ */
char *default_color[] =
{
  "null",
  "ctermfg=white    ctermbg=black",   // win
  "ctermfg=white    ctermbg=black     cterm=bold",    // cmdbar
  "ctermfg=black    ctermbg=white",   // border
  "ctermfg=yellow   ctermbg=black     cterm=bold",    // directory
  "ctermfg=cyan     ctermbg=black",   // fslash
  "ctermfg=red      ctermbg=black",   // symlink
  "ctermfg=magenta  ctermbg=black",   // blk_device
  "ctermfg=green    ctermbg=black",   // device
  "ctermfg=cyan     ctermbg=black",   // socket
  "ctermfg=white    ctermbg=black",   // nfile
  "ctermfg=black    ctermbg=red",     // select
  "ctermfg=white    ctermbg=blue      cterm=bold",    // current
  "ctermfg=blue     ctermbg=black",   // tabview
  "ctermfg=white    ctermbg=blue      cterm=bold",    // tabview-sel
  "ctermfg=white    ctermbg=black",   // tabview-bar
  "ctermfg=blue     ctermbg=black     cterm=bold",    // executable
  "ctermfg=black    ctermbg=white",   // path
  "ctermfg=white    ctermbg=white     cterm=bold",    // path-sel
  "ctermfg=red      ctermbg=black     cterm=bold",    // error_message
}; /* }}} */

/* default_config {{{ */
char *default_config[] =
{
  "\" The # or \" character at the beginning of a line comments out the line.",
  "\" Blank lines are ignored.",
  "\" Valid values are numbers, true or false and strings wrong values=0",
  "\" when no XDG_CONFIG_HOME exist in your env. than is ~/.seoni in use",
  "\" otherwise ~/.config/seoni",
  "\" colorthemes are in a folder named color in the config dir",
  "",
  "\"--!themefile!--",
  "\" theme name must be the name of the themefile that is placed in color dir",
  "set theme_name = default",
  "\" turn on/off transparency when your terminal support it looks like",
  "\" example: URxvt*transparent: on  in the ~/.Xdefaults file, when urxvt is the,",
  "\" terminalemulator that is in use",
  "set default_bg = false",
  "",
  "\"--!sort!--",
  "\" name = 0, size = 1, mtime = 2, atime = 3, ctime = 4",
  "set sort_option = name",
  "",
  "\"--!split!--",
  "\" first seoni-start runs in splitview",
  "set start_split = 1",
  "",
  "\" always start split windows in a new tab",
  "set always_split = 0",
  "",
  "\"--!redraw!--",
  "\" Valid delay values are 0-9 0=off",
  "set redraw_delay = 2",
  "",
  "\"--!tab!--",
  "\" shorten a tab to a given size if value=0 than only tabnumbers are shown max valid size=255",
  "set max_tablen = 20",
  "",
  "\"--!keyconfig!--",
  "\" map [lft] [rgt]   lft = the key you want to use; rgt = the place where lft associated from",
  "\" example: map k :marker_up   and than we can do    map <up> k  (up = k = :marker_up) ",
  "\" we can use a maximal sequence from 2 character gg tt ..etc",
  "\" when a sequence is active zb gg and we have push g than seoni is whaiting for a second",
  "\" character wheres is leading g, special chars are in <>  example: <C-l> or <pagedown>",
  "\" a list of special keys:",
  "\" <up> <down> <pageup> <pagedown> <esc> <return> <tab> <home> <end> <backspace> <F(1-12)>",
  ""
}; /* }}} */

/* default_maps {{{ */
char *default_maps[] =
{
  "dummy",
  "k           :marker_up",
  "j           :marker_dn",
  "gg          :marker_top",
  "G           :marker_bot",
  "H           :marker_top_win",
  "L           :marker_bot_win",
  "M           :marker_center_win",
  "<pageup>    :scroll_page_up",
  "<pagedown>  :scroll_page_dn",
  "<C-u>       :scroll_half_page_up",
  "<C-d>       :scroll_half_page_dn",
  "gT          :prev_tab",
  "gt          :next_tab",
  "tt          :open_in_new_tab",
  "tT          :delete_tab",
  "oh          :open_in_hsplit",
  "dv          :close_view",
  "V           :visual_mode",
  "<esc>       :normal_mode",
  ":           :cmd_mode",
  "<space>     :tag_file",
  "zr          :toggle_filefilter",
  "zs          :toggle_sort",
  "<tab>       :toggle_win",
  "l           :dir_dn",
  "h           :dir_up",
  "R           :redraw",
  "~           :home_switch"
}; /* }}} */

/* append_map {{{ */
keys_t
*append_map(int idx)
{
  int sc_idx = 0;
  keys_t *new_map = (keys_t *)malloc(sizeof(keys_t));
  if(new_map == NULL) return NULL;
  new_map->cmd = NULL;
  new_map->buf_size = 10;
  new_map->count = 0;
  new_map->s_count = 0;
  new_map->sc_size = 10;

  if( (new_map->buf = malloc_char_array(10, 3)) == NULL)
  {
    free(new_map);
    return NULL;
  }
  else if( (new_map->special_case = malloc(10 * sizeof(int)) ) == NULL)
  {
    free(new_map->buf);
    free(new_map);
    return NULL;
  }
  for(; sc_idx < 10; sc_idx++)
    new_map->special_case[sc_idx] = 0;
  return new_map;
} /* }}} */

/* init_cfg_t {{{ */
int
init_cfg_t(cfg_t **cfg)
{
  int idx = 0;
  int dms = (sizeof(default_maps) / sizeof(default_maps[0]));

  if(*cfg != NULL) return 0;
  if(( (*cfg) = (cfg_t *)malloc(sizeof(cfg_t)) ) == NULL)
    return -1;

  (*cfg)->reload = 0;
  (*cfg)->default_bg = 0;
  (*cfg)->sort = 0;
  (*cfg)->start_split = 0;
  (*cfg)->always_split = 0;
  (*cfg)->redraw_delay = 0;
  (*cfg)->max_tablen = 0;

  zero_colors( (*cfg)->color);

  (*cfg)->config_path = NULL;
  (*cfg)->colorschemes = NULL;
  (*cfg)->themename = NULL;

  (*cfg)->user_k_size = dms;

  if(( (*cfg)->user_k = (keys_t **)malloc(dms * (sizeof(keys_t))) ) == NULL)
    return -1;
  for(idx = 0; idx <= dms; idx++)
  {
    if(( (*cfg)->user_k[idx] = append_map(idx)) == NULL)
      return -1;
  }
  return 1;
} /* }}} */

/* expand_keys_t {{{ */
int
expand_keys_t(cfg_t **cfg, int map_c)
{
  if(map_c <= (*cfg)->user_k_size)
    return 0;
  (*cfg)->user_k_size = map_c;

  (*cfg)->user_k =
    (keys_t **)realloc((*cfg)->user_k, map_c * (sizeof(keys_t)));
  if( (*cfg)->user_k == NULL) return -1;
  else if(( (*cfg)->user_k[map_c] = append_map(map_c)) == NULL)
    return -1;
  return 1;
} /* }}} */

/*free_cfg_t {{{ */
void
free_cfg_t(cfg_t *cfg)
{
  int idx = cfg->user_k_size;

  if(cfg->user_k != NULL)
  {
    for(; idx >= 0; idx--)
    {
      free(cfg->user_k[idx]->special_case);
      free_char_array(cfg->user_k[idx]->buf,
          cfg->user_k[idx]->buf_size-1);

      if(cfg->user_k[idx]->cmd != NULL)
        free(cfg->user_k[idx]->cmd);

      free(cfg->user_k[idx]);
    }
    free(cfg->user_k);
  }
  free(cfg->colorschemes);
  free(cfg->config_path);
  free(cfg->themename);

  free(cfg);
} /* }}} */

/* set_cfg_folder {{{ */
int
check_cfg_directorys(cfg_t **cfg)
{
  int pathlen = 50;
  char *home = getenv("HOME");
  if(home == NULL)
    return -1;
  else pathlen += strlen(home);
  char cfg_path[pathlen];
  char colors_path[pathlen];

  // testing ~/.config/seoni
  snprintf(cfg_path, pathlen, "%s/.config", home);
  if(access(cfg_path, 00) != -1)
  {
    snprintf(cfg_path, pathlen, "%s/.config/seoni", home);
    if(access(cfg_path, 00) == -1 &&
        mkdir(cfg_path, 0711) == -1)
      return 0;
  }
  else // testing ~/.seoni
  {
    snprintf(cfg_path, pathlen, "%s/.seoni", home);
    if(access(cfg_path, 00) == -1 &&
        mkdir(cfg_path, 0711) == -1)
      return 0;
  }
  if( (*cfg)->config_path == NULL)
    (*cfg)->config_path = (char *)malloc(pathlen * sizeof(char));
  else
    (*cfg)->config_path =
      (char *)realloc( (*cfg)->config_path, pathlen * sizeof(char));

  // init cfgpath
  snprintf((*cfg)->config_path, pathlen, "%s", cfg_path);

  // testing colorschemes path
  snprintf(colors_path, pathlen, "%s/colors", cfg_path);
  if(access(colors_path, 00) == -1)
  {
    if(mkdir(colors_path, 0711) == -1)
      return 0;
    else
      create_default_theme(colors_path);
  }
  if( (*cfg)->colorschemes == NULL)
    (*cfg)->colorschemes = (char *)malloc(pathlen * sizeof(char));
  else
    (*cfg)->colorschemes =
      (char *)realloc( (*cfg)->colorschemes, pathlen * sizeof(char));

  snprintf((*cfg)->colorschemes, pathlen, "%s", colors_path);
  return 1;
} /* }}} */

/* clear_whitespaces {{{ */
void
close_gaps(char *line, char *dest)
{
  int match = 0, line_cc = 0;

  while(isblank(line[match]))
    match++;
  
  while(line[match] != '\0')
  {
    while(isblank(line[match])) match++;
    dest[line_cc] = line[match];

    match++; line_cc++;
  }
  dest[line_cc] = '\0';
} /* }}} */

/* realloc_sc_size {{{ */
int
realloc_sc_size(keys_t ***user_k, int idx)
{
  int sc_size_tmp = 0, *tmp = 0, c = 0;

  sc_size_tmp = (*(*user_k+idx))->sc_size;
  sc_size_tmp += 2;

  tmp = realloc( (*(*user_k+idx))->special_case,
      (sc_size_tmp * sizeof(int)) );

  if(tmp == NULL) return 0;
  
  (*(*user_k+idx))->special_case = tmp;

  for(c = (*(*user_k+idx))->sc_size; c < sc_size_tmp; c++)
    (*(*user_k+idx))->special_case[c] = 0;

  (*(*user_k+idx))->sc_size = sc_size_tmp;
  return 1;
} /* }}} */

/* search_sc {{{ */
int
search_sc(keys_t ***user_k, char *sc, int user_k_size)
{
  int idx = 1, c = 0, match = -1;

  for(; idx < user_k_size; idx++)
  {
    for(c = 0; c < (*(*user_k+idx))->count; c++)
    {
      if(strcmp(sc, (*(*user_k+idx))->buf[c]) == 0)
        match = idx;
    }
  }
  return match;
} /* }}} */

/* search_scase {{{ */
int
search_scase(keys_t ***user_k, int scase, int user_k_size)
{
  int idx = 1, c = 0, match = -1;

  for(; idx < user_k_size; idx++)
  {
    for(c = 0; c < (*(*user_k+idx))->s_count; c++)
    {
      if(scase == (*(*user_k+idx))->special_case[c])
        match = idx;
    }
  }
  return match;
} /* }}} */

/* ret_ctrl_mod {{{ */
int
ret_ctrl_mod(char *lft_buf)
{
  int idx = 0;
  int ctrl_lower = 96, ctrl_upper = 55;
  int head = 0, tail = 0, match_k = -1;

  while(lft_buf[idx] != '-' && lft_buf[idx] != '\0')
    idx++;
  head = idx;

  while(lft_buf[idx] != '>' && lft_buf[idx] != '\0')
    idx++;
  tail = idx;

  tail--;
  head++;

  if(head != tail || lft_buf[idx] != '>') return -1;

  match_k = lft_buf[head];
  if(isupper(lft_buf[head]))
    match_k -= ctrl_upper;
  else match_k -= ctrl_lower;

  return match_k;
} /* }}} */

/* ret_valid_scase {{{ */
int
ret_valid_scase(char *rglft_buf)
{
  int idx = 0, match = -1;

  int scase_value[] =
  {
    KEY_F(1), KEY_F(2), KEY_F(3), KEY_F(4),
    KEY_F(5), KEY_F(6), KEY_F(7), KEY_F(8),
    KEY_F(9), KEY_F(10), KEY_F(11), KEY_F(12),
    KEY_UP, KEY_DOWN, KEY_RIGHT, KEY_LEFT,
    KEY_PPAGE, KEY_NPAGE, KEY_BACKSPACE,
    KEY_HOME, KEY_END,
    9, 0XA, 040, 27
  };
  char *scase[] =
  {
    "<F1>", "<F2>", "<F3>", "<F4>", "<F5>",
    "<F6>", "<F7>", "<F8>", "<F9>", "<F10>",
    "<F11>", "<F12>",
    "<up>", "<down>", "<right>", "<left>",
    "<pageup>", "<pagedown>", "<backspace>",
    "<home>", "<end>",
    "<tab>", "<return>", "<space>", "<esc>"
  };
  for(; idx < (sizeof(scase) / sizeof(scase[0])); idx++)
  {
    if(strcmp(rglft_buf, scase[idx]) == 0)
    {
      match = scase_value[idx];
      break;
    }
  }
  return match;
} /* }}} */

/* init_key {{{ */
int
init_map(keys_t ***user_k, char *lft_buf, int idx, int user_k_size)
{
  int match = -1;

  char **kbuf = ( (*(*user_k+idx))->buf );
  int k_count = ( (*(*user_k+idx))->count );
  int s_count = ( (*(*user_k+idx))->s_count );
  int sc_size = ( (*(*user_k+idx))->sc_size );
  int k_bufsize = ( (*(*user_k+idx))->buf_size );

  if(strlen(lft_buf) < 3)
  {
    // if same shortcut exist
    if(search_sc(user_k, lft_buf, user_k_size) != -1) return 0;

    snprintf( kbuf[k_count], MAX_SC_LEN, "%s", lft_buf);
    (*(*user_k+idx))->count++;
    k_count = (*(*user_k+idx))->count;

    if(k_count >= k_bufsize)
    {
      (*(*user_k+idx))->buf =
        realloc_char_array( (*(*user_k+idx))->buf,
            &(*(*user_k+idx))->buf_size, 3);
    }
    if( (*(*user_k+idx))->buf == NULL) return 0;
    return 1;
  }
  if(ret_suitable_string(lft_buf, "<C", '-') == 1)
    match = ret_ctrl_mod(lft_buf);
  else
    match = ret_valid_scase(lft_buf); 
  if(match == -1) return 0;
  else
  {
    if(search_scase(user_k, match, user_k_size) != -1) return 0;

    (*(*user_k+idx))->special_case[s_count] = match;
    (*(*user_k+idx))->s_count++;
  }
  if( (*(*user_k+idx))->s_count >= sc_size)
    realloc_sc_size(user_k, idx);
  return 1;
} /* }}} */

/* zero_maps {{{ */
void
zero_maps(keys_t ***user_k, int user_k_size)
{
  int idx = 1, c = 0;

  for(; idx < user_k_size; idx++)
  {
    for(c = 0; c < (*(*user_k+idx))->count; c++)
      (*(*user_k+idx))->buf[c][0] = '\0';
    (*(*user_k+idx))->count = 0;
    
    for(c = 0; c < (*(*user_k+idx))->s_count; c++)
      (*(*user_k+idx))->special_case[c] = 0;
    (*(*user_k+idx))->s_count = 0;
  }
} /* }}} */

/* init_default_maps {{{ */
void
init_default_maps(keys_t ***user_k, int user_k_size)
{
  int idx = 1;
  int def_maps_size = 
    (sizeof(default_maps) / sizeof(default_maps[0]));

  for(; idx < def_maps_size; idx++)
    parse_map(user_k, default_maps[idx], user_k_size);
} /* }}} */

/* ret_valid_mapregion {{{ */
int
ret_valid_mapregion(keys_t ***user_k, char *rgt_buf, int user_k_size)
{
  int match = -1;
  
  if(ret_suitable_string(rgt_buf, "<C", '-') == 1)
    match = ret_ctrl_mod(rgt_buf);
  else
    match = ret_valid_scase(rgt_buf);

  if(match != -1)
    return (search_scase(user_k, match, user_k_size));
  else
    return (search_sc(user_k, rgt_buf, user_k_size));
} /* }}} */

/* parse_map {{{ */
int
parse_map(keys_t ***user_k, char *line, int user_k_size)
{
  int line_len = strlen(line);
  int l_idx = 0, idx = 0, match = -1;
  char lft_buf[50], rgt_buf[50], *tmp = NULL;

  if(line_len < 3 || line_len >= 50) return 0;

  for(; !isblank(line[l_idx]) && line[l_idx] != '\0'; l_idx++, idx++)
    lft_buf[idx] = line[l_idx];

  lft_buf[idx] = '\0'; // terminate lft
  if(line[l_idx] == '\0' || !isblank(line[l_idx])) return 0; 

  while(isblank(line[l_idx])) l_idx++;
  if(line[l_idx] == '\0') return 0; 

  for(idx = 0; !isblank(line[l_idx]) && line[l_idx] != '\0'; l_idx++, idx++)
    rgt_buf[idx] = line[l_idx];
  
  rgt_buf[idx] = '\0';

  // we have a function
  if( strlen(rgt_buf) >= 3 && rgt_buf[0] == ':')
  {
    tmp = rgt_buf;
    tmp++;
    line_len = strlen(tmp)+1;
    for(idx = 1; idx <= user_k_size; idx++)
    {
      if( (*(*user_k+idx))->cmd == NULL)
      {
        (*(*user_k+idx))->cmd =
          (char *)malloc(line_len * sizeof(char));

        snprintf( (*(*user_k+idx))->cmd, line_len, "%s", tmp);
        match = idx;
        break;
      }
      else if(strcmp(tmp, (*(*user_k+idx))->cmd) == 0)
      {
        match = idx;
        break;
      }
    }
    if(match == -1) return 0;
    init_map(user_k, lft_buf, match, user_k_size);
  }
  else
  {
    match = ret_valid_mapregion(user_k, rgt_buf, user_k_size);
    if(match != -1)
      init_map(user_k, lft_buf, match, user_k_size);
  }
  return 1;
} /* }}} */

/* check_keyws {{{ */
int
parse_keyws(char *cfg_line, char *buf)
{
  int idx = 0, word_len = 0, match = -1;
  char *keyws[] = { "set", "map", "hi", "highlight" };
  char *tmp = NULL;

  while(isblank(cfg_line[idx])) cfg_line++;
  for(; idx < (sizeof(keyws) / sizeof(keyws[0])); idx++)
  {
    while(!isblank(cfg_line[word_len]))
    {
      if(cfg_line[word_len] == '\0') return -1;
      word_len++;
    }
    if(strncmp(cfg_line, keyws[idx], word_len) == 0)
    {
      if(! isblank(cfg_line[word_len])) return -1;
      else
      {
        while(isblank(cfg_line[word_len]))
          word_len++;
      }
      tmp = cfg_line;
      tmp += word_len;
      snprintf(buf, MAX_COLS, "%s", tmp);
      if(idx == 3) idx = 2;
      match = idx;
    }
  }
  return match;
} /* }}} */

/* get_configfiles {{{ */
int
read_config(cfg_t **cfg, char *config_path)
{
  int found = -1, map_c = 0;
  char buf[MAX_COLS], tmp[MAX_COLS];
  FILE *stream;

  if((stream = fopen(config_path, "r")) == NULL) return 0;
  while(fgets(buf, MAX_COLS, stream) != NULL)
  {
    if(buf[0] == '\n' || buf[0] == '#' || buf[0] == '"') continue;
    termstr(buf);
    found = parse_keyws(buf, tmp);
    switch(found)
    {
      case MAP:
        map_c++;
        if(expand_keys_t(cfg, map_c) == -1)
          return -1;
        parse_map( &(*cfg)->user_k, tmp, (*cfg)->user_k_size);
        break;

      case SET:
        close_gaps(tmp, buf);
        found = parse_config(buf, tmp);
        if(found != -1)
          init_user_config(cfg, tmp, found);
        break;
    }
    tmp[0] = '\0';
    found = -1;
  }
  fclose(stream);
  return 1;
} /* }}} */

/* scan_config_sections {{{ */
int
parse_config(char *buf, char *user_input)
{
  int idx = 0, match = -1;

  for(; idx < (sizeof(config_section) / 
        sizeof(config_section[0])); idx++)
  {
    if(ret_suitable_string(buf,
          config_section[idx], '=') == 1

        && parse_from_c(buf, user_input, '=') != 0)
    {
      match = idx;
      break;
    }
  }
  return match;
} /* }}} */

/* ret_userinput_value {{{ */
int
ret_userinput_value(char **field, char *usr_input, int max_ret_val)
{
  int idx = 0, num = -1, usrl = strlen(usr_input);

  if(isdigit(usr_input[0]) != 0 && usrl <= 3)
    num = atoi(usr_input);
  if(num <= max_ret_val && num != -1) return num;
  else if(field != NULL)
  {
    for(; idx <= max_ret_val; idx++)
      if(strncmp(usr_input, field[idx], usrl) == 0)
        return idx;
  }
  return 0;
} /* }}} */

/* init_user_config {{{ */
void
init_user_config(cfg_t **cfg, char *user_input, int ui_id)
{
  int scheme_len = 0;
  char *b[] = {"false", "true"};
  char *sort_val[] = {"name", "size", "mtime", "atime", "ctime"};

  switch(ui_id)
  {
    case THEME:
      scheme_len = strlen(user_input)+1;
      if( (*cfg)->themename == NULL)
        (*cfg)->themename = (char *)malloc(scheme_len * sizeof(char));
      else
        (*cfg)->themename =
          (char *)realloc( (*cfg)->themename, scheme_len * sizeof(char));
      
      snprintf((*cfg)->themename, scheme_len, "%s", user_input);
      break;

    case DEFAULT_BG:
      (*cfg)->default_bg =
        ret_userinput_value(b, user_input, 1);
      break;

    case SORT_O:
      (*cfg)->sort =
        ret_userinput_value(sort_val, user_input, 4);
      break;

    case START_SPLIT:
      (*cfg)->start_split =
        ret_userinput_value(b, user_input, 1);
      break;

    case ALWAYS_SPLIT:
      (*cfg)->always_split =
        ret_userinput_value(b, user_input, 1);
      break;

    case REDRAW_DELAY:
      if( (strlen(user_input)) < 2)
      {
        // delay 0-9
        (*cfg)->redraw_delay =
          ret_userinput_value(NULL, user_input, 10);
      }
      else (*cfg)->redraw_delay = 0;
      break;

    case MAX_TABLEN:
      (*cfg)->max_tablen =
        ret_userinput_value(NULL, user_input, 255);
      break;
  }
} /* }}} */

/* parse_from_c {{{ */
int
parse_from_c(char *config_line, char *output, int c)
{
  int line_len = strlen(config_line);

  if(line_len >= MAX_COLS) return 0;

// config_line[line_len-1] = '\0';
  for(; *config_line != c; config_line++)
  {
    if(*config_line == c
        || *config_line == '\0') break;
  }
  if(*config_line == '\0') return 0;
  else config_line++;
  while(isblank(*config_line))
  {
    if(*config_line == '\0') return 0;
    config_line++;
  }
  snprintf(output, MAX_COLS, "%s", config_line);
  return 1;
} /* }}} */

/* zero_colors {{{ */
void
zero_colors(Color *color)
{
  int color_size = 0, idx = 0;

  color_size = (sizeof(ui_color) / sizeof(ui_color[0]));

  for(idx = 0; idx < color_size; idx ++)
  {
     color[idx].fg = 0;
     color[idx].bg = 0;
     color[idx].attr = 0;
  }
} /* }}} */

/* init_default_colors {{{ */
void
init_default_colors(Color *color)
{
  int idx = 1;
  int max_colors = (sizeof(ui_color) /
      sizeof(ui_color[0]));

  for(; idx < max_colors; idx++)
    parse_color(color, default_color[idx], idx);
} /* }}} */

/* set_pair_attr {{{ */
int
set_pair_attr(Color *color, int p_idx, char *pair_buf)
{
  int idx = 0, match = -1, attr = -1;
  char *buf = pair_buf;
  char *termvar[] = { "cterm", "term" };
  char *termattr[] = { "bold", "underline", "blink", "dim",
                       "reverse" };

  for(; idx < 2; idx++)
  {
    if(ret_suitable_string(pair_buf, termvar[idx], '=') != 1)
      continue;
    else match = idx;
  }
  if(match == -1) return 0;
  buf += strlen(termvar[match])+1;
  match = 0;
  while(*buf != '\0')
  {
    while(buf[match] != ',' && buf[match] != '\0')
      match++;

    if(buf[match] == ',')
      match--;

    for(idx = 0; idx < 5; idx++)
      if(strncasecmp(buf, termattr[idx], match) == 0)
        attr = idx;
    if(attr == -1) return 0;
    switch(attr)
    {
      case 0: // attr is a bitmask
        color[p_idx].attr += A_BOLD;
        break;

      case 1:
        color[p_idx].attr += A_UNDERLINE;
        break;

      case 2:
        color[p_idx].attr += A_BLINK;
        break;

      case 3:
        color[p_idx].attr += A_DIM;
        break;

      case 4:
        color[p_idx].attr += A_REVERSE;
        break;
    }
    if(buf[match] != '\0') match++;
    else return 0;
    if(buf[match] == ',')
      match++;
    buf += match;
    match = 0;
    attr = -1;
  }
  return 1;
} /* }}} */

/* set_pair_color {{{ */
int
set_pair_color(Color *color, int p_idx, char *pair_buf)
{
  int idx = 0;
  short int col = -1;
  char *buf = pair_buf;
  char *termvar[] = { "fg", "ctermfg", "bg", "ctermbg" };
  
  for(; idx < 4; idx++)
  {
    if(ret_suitable_string(pair_buf, termvar[idx], '=') != 1)
      continue;

    buf += strlen(termvar[idx])+1;
    col = convert_col(buf);
    if(col != -1)
    {
      if(idx == 0 || idx == 1)
        color[p_idx].fg = col;
      else color[p_idx].bg = col;

      return 1;
    }
    else return 0;
  }
  return 0;
} /* }}} */

/* parse_color_pair {{{ */
int
parse_color(Color *color, char *pair_line, int idx)
{
  int pos = 0, head = 0, tail = 0, buf_c = 0, equal = 0;
  char pair_buf[50];

  for(; pair_line[pos] != '\0'; pos++)
  {
    if(isblank(pair_line[pos])) continue;
    head = pos;
    while(!isblank(pair_line[pos])
        && pair_line[pos] != '\0')
    {
      if(pair_line[pos] == '=')
        equal = 1;
      pos++;
    }
    if(!equal) return 0;
    else equal = 0;
    tail = pos;
    buf_c = 0;
    for(; head < tail; head++)
    {
      pair_buf[buf_c] = pair_line[head];
      if(tail == 1) break;
      buf_c++;
    }
    pair_buf[(buf_c)] = '\0';

    if(set_pair_color(color, idx, pair_buf) == 0)
      set_pair_attr(color, idx, pair_buf);

    if(pair_line[pos] == '\0')
      return 0;
  }
  return 1;
} /* }}} */

/* grap_colorname {{{ */
int
parse_color_pair(Color *color, char *theme_line)
{
  int idx = 0, pos = 0;
  int max_colors = (sizeof(ui_color) /
      sizeof(ui_color[0]));

  char pair_line[MAX_COLS];
  // we need this to compare the userinput
  // from theme_line.
  while(!isblank(theme_line[pos]))
    pos++;

  for(; idx < max_colors; idx++)
  {
    if(strncmp(theme_line, ui_color[idx], pos) != 0)
      continue;
    if(parse_from_c(theme_line, pair_line, 040) == 1)
    {
      color[idx].attr = 0;
      parse_color(color, pair_line, idx);
      return 1;
    }
  }
  return 0;
} /* }}} */

/* read_themefile {{{ */
int
read_themefile(Color *color, char *themepath)
{
  int read = 0, idx = 0, color_size = 0, found = -1;
  char buf[MAX_COLS], tmp[MAX_COLS];
  char *b[] = {"false", "true"};
  FILE *stream;
  
  color_size = (sizeof(ui_color) / sizeof(ui_color[0]));
  for(; idx < color_size; idx++)
    color[idx].attr = 0;

  if((stream = fopen(themepath, "r")) == NULL) return 0;
  while(fgets(buf, MAX_COLS, stream) != NULL)
  {
    if(buf[0] == '\n' || buf[0] == '#' || buf[0] == '"') continue;
    termstr(buf);
    found = parse_keyws(buf, tmp);
    switch(found)
    {
      case SET:
        close_gaps(tmp, buf);
        if(ret_suitable_string(buf, "use", '=') == 1
            && parse_from_c(buf, tmp, '=') != 0)

          read = ret_userinput_value(b, tmp, 1);
        break;

      case HI:
        if(read != 1) continue;
        parse_color_pair(color, tmp);
        break;
    }
  }
  fclose(stream);
  return 1;
} /* }}} */

/* convert_col {{{ */
int
convert_col(char *colorname)
{
  int col_c, ret_col = -1;

  if(isdigit(colorname[0]) != 0)
  {
    if(can_change_color() == TRUE)
      ret_col = atoi(colorname);
  }
  else
  {
    char *colors[] =
    {
      "black", "red", "green", "yellow", "blue",
      "magenta", "cyan", "white", "brightblack",
      "brightred", "brightgreen", "brightyellow",
      "brightblue", "brightmagenta", "brightcyan",
      "brightwhite"
    };
    for(col_c = 0; col_c
        < (sizeof(colors) / sizeof(colors[0])); col_c++)
    {
      if(strcasecmp(colorname, colors[col_c]) == 0)
      {
        ret_col = col_c;
        break;
      }
    }
  }
  if(ret_col == -1 || ret_col > 255) return -1;
  else return ret_col;
} /* }}} */

/* create_default_theme {{{ */
int
create_default_theme(char *theme_path)
{
  char def_theme[50];
  int idx = 0;
  int max_colors = (sizeof(ui_color) /
      sizeof(ui_color[0]));

  FILE *themefile;
  build_path(theme_path, "default",
      def_theme, 50);

  if((themefile = fopen(def_theme, "w")) == NULL)
    return 0;
  for(; idx < max_colors; idx++)
  {
    if(idx == 0)
      fprintf(themefile, "set use = 1\n");
    else
    {
      fprintf(themefile, "hi %s   %s\n",
          ui_color[idx], default_color[idx]);
    }
  }
  fclose(themefile);
  return 1;
} /* }}} */

/* create_configfile {{{ */
int
create_configfile(char *config_path)
{
  int cfg_size = (sizeof(default_config) /
      sizeof(default_config[0]));

  int key_cfg_size = (sizeof(default_maps) /
      sizeof(default_maps[0]));

  int idx = 0;
  FILE *configfile;
  
  if((configfile = fopen(config_path, "w")) == NULL)
    return 0;
  else
  {
    while(idx != cfg_size)
    {
      fprintf(configfile, "%s\n",
          default_config[idx]);
      idx++;
    }
    idx = 1;
    while(idx != key_cfg_size)
    {
      fprintf(configfile, "map %s\n",
          default_maps[idx]);
      idx++;
    }
  }
  fclose(configfile);
  return 1;
} /* }}} */

/*init _config {{{ */
void
init_config(cfg_t **cfg, int load_bool)
{
  int scheme_len = 10;
  char cfg_file_path[MAX_COLS];

  if(check_cfg_directorys(cfg) != 0)
  {
    char cfg_filename[7] = "/nfbrc";
    snprintf(cfg_file_path, MAX_COLS, "%s%s",
        (*cfg)->config_path, cfg_filename);
    
    if(access(cfg_file_path, 00) == -1)
      create_configfile(cfg_file_path);
  }
  if(load_bool == RELOAD)
  {
    if((*cfg)->themename != NULL)
      (*cfg)->themename[0] = '\0';
    (*cfg)->default_bg = 0;
    (*cfg)->reload = 0;
    (*cfg)->sort = 0;
    (*cfg)->start_split = 0;
    (*cfg)->always_split = 0;
    (*cfg)->redraw_delay = 0;
    (*cfg)->max_tablen = 0;

    zero_colors( (*cfg)->color);
    zero_maps( &(*cfg)->user_k, (*cfg)->user_k_size);

  }
  init_default_maps( &(*cfg)->user_k,
      (*cfg)->user_k_size);

  init_default_colors( (*cfg)->color);
  if(read_config(cfg, cfg_file_path) != 0)
  {
    if((*cfg)->themename != NULL)
    {
      scheme_len += strlen((*cfg)->config_path);
      scheme_len += strlen((*cfg)->themename);
      char scheme_path[scheme_len];

      snprintf(scheme_path, scheme_len, "%s/%s",
          (*cfg)->colorschemes, (*cfg)->themename);
     
      read_themefile((*cfg)->color, scheme_path);
    }
  }
  init_themefile( (*cfg)->color, (*cfg)->default_bg);
} /* }}} */

/* init_themefile {{{ */
void
init_themefile(Color *color, int default_bg)
{
  short idx = 0;
  int max_colors = (sizeof(ui_color) / sizeof(ui_color[0]));

  for(; idx < max_colors; idx++)
  {
    if(default_bg == FALSE)
    {
      init_pair(idx, color[idx].fg, color[idx].bg);
      continue;
    }
    if(idx == 1)
    {
      use_default_colors(); // from ncurses
      init_pair(idx, color[idx].fg, -1);
    }
    else if(idx == CURRENT || idx == TABVIEW || idx == TABVIEW_SEL ||
        idx == TABVIEW_BAR || idx == BORDER || PATH_SEL || idx == CMDBAR)
    {
      init_pair(idx, color[idx].fg, color[idx].bg);
    }
    else init_pair(idx, color[idx].fg, -1);
  }
} /* }}} */
