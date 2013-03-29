/* config.h by lutherus 2013 */

#define MAX_CFG_SECTIONS 7
#define MAX_UI_CHAR 20
//#define MAX_UI_COLORS 32
#define MAX_CONFIG_LINES 50
//keys
#define MAX_SC 10
#define MAX_SC_LEN 10

#define FIRSTLOAD 0
#define RELOAD 1

/* envflag {{{ */
enum
envflag
{
  HOME,
  PWD
}; /* }}} */

/* reserved_keyws {{{ */
enum
reserved_keyws
{
  SET,
  MAP,
  HI
}; /* }}} */

/* reserved_keywords_id {{{ */
enum
reserved_keywords_id
{
  THEME,
  DEFAULT_BG,
  SORT_O,
  START_SPLIT,
  ALWAYS_SPLIT,
  REDRAW_DELAY,
  MAX_TABLEN
}; /* }}} */

/* colorflag {{{ */
enum
colorflag
{
  BLACK,
  RED,
  GREEN,
  YELLOW,
  BLUE,
  MAGENTA,
  CYAN,
  WHITE
}; /* }}} */

/* color_pair_sections {{{ */
enum
color_p_sections
{
  WIN = 1,
  CMDBAR,
  BORDER,
  DIRECTORY,
  FSLASH,
  SYMLINK,
  BLK_DEVICE,
  DEVICE,
  SOCKET,
  NFILE,
  SELECT,
  CURRENT,
  TABVIEW,
  TABVIEW_SEL,
  TABVIEW_BAR,
  EXECUTABLE,
  PATH,
  PATH_SEL,
  ERROR
}; /* }}} */

/* key_identifier_selection {{{ */
enum
key_identifiers
{
  MARKER_UP = 1,
  MARKER_DN,
  MARKER_TOP,
  MARKER_BOT,
  MARKER_TOP_WIN,
  MARKER_BOT_WIN,
  MARKER_CENTER_WIN,
  SCROLL_PAGE_UP,
  SCROLL_PAGE_DN,
  SCROLL_HALF_PAGE_UP,
  SCROLL_HALF_PAGE_DN,
  PREV_TAB,
  NEXT_TAB,
  OPEN_IN_NEW_TAB,
  DELETE_TAB,
  OPEN_IN_HSPLIT,
  CLOSE_VIEW,
  VISUAL_MODE,
  NORMAL_MODE,
  CMD_MODE,
  TAG_FILE,
  TOGGLE_FILEFILTER,
  TOGGLE_SORT,
  TOGGLE_WIN,
  DIR_DN,
  DIR_UP,
  REDRAW,
  HOMESWITCH
}; /* }}} */

/* keys {{{ */
typedef struct
{
  char **buf;
  char *cmd;

  int count;
  int s_count;
  int *special_case;

  int buf_size;
  int sc_size;

}keys_t;/* }}} */

/* color {{{ */
typedef struct _Color
{
  short int fg;
  short int bg;

  int attr;

} Color; /* }}} */

/* cfg {{{ */
typedef struct _cfg_t
{
  int reload;
  int default_bg;
  int sort;
  int start_split;
  int always_split;
  int redraw_delay;
  int max_tablen;

  char *config_path;
  char *colorschemes;
  char *themename;
  Color color[20];

  int user_k_size;
  keys_t **user_k;

///  int buf_size;

}cfg_t; /* }}} */

/* prototypes {{{ */

/* cfg_t */
keys_t *appen_map(int idx);
int init_cfg_t(cfg_t **cfg);
int expand_keys_t(cfg_t **cfg, int map_c);
void free_cfg_t(cfg_t *cfg);

/* parsing */
void close_gaps(char *line, char *dest);
int parse_from_c(char *config_line, char *output, int c);

/* colors */
void zero_colors(Color *color);
void init_default_colors(Color *color);
int set_pair_attr(Color *color, int p_idx, char *pair_buf);
int set_pair_color(Color *color, int p_idx, char *pair_buf);
int parse_color(Color *color, char *pair_line, int idx);
int parse_color_pair(Color *color, char *theme_line);
int read_themefile(Color *color, char *themepath);
int convert_col(char *colorname);
int create_default_theme(char *theme_path);
void init_themefile(Color *color, int default_bg);

/* config */
int parse_keyws(char *cfg_line, char *buf);
int read_config(cfg_t **cfg, char *config_path);
int parse_config(char *buf, char *user_input);
int ret_userinput_value(char **field, char *usr_input, int max_re_val);
void init_user_config(cfg_t **cfg, char *user_input, int ui_id);
int create_configfile(char *config_path);
void init_config(cfg_t **cfg, int load_bool);

/* keys */
int realloc_sc_size(keys_t ***user_k, int idx);
int search_sc(keys_t ***user_k, char *sc, int user_k_size);
int search_scase(keys_t ***user_k, int scase, int user_k_size);
int ret_ctrl_mod(char *lft_buf);
int ret_valid_scase(char *rglft_buf);
int init_map(keys_t ***user_k, char *lft_buf, int idx, int user_k_size);
int parse_map(keys_t ***user_k, char *line, int user_k_size);
int parse_keyws(char *cfg_line, char *buf);
void zero_maps(keys_t ***user_k, int user_k_size);
void init_default_maps(keys_t  ***user_k, int user_k_size);
int ret_valid_mapregion(keys_t ***userk_k, char *rgt_buf, int user_k_size);

/* env */
int get_env(char *envbuf, int envflag);
int check_cfg_directorys(cfg_t **cfg); /* }}} */

