/* test.h */

#define MAX_SIZE 100

// sel_file
#define NEXT 1
#define PREV 0

#define ON 1
#define OFF 0
#define DEEPON 1
#define DEEPOFF 0

#define ENVBUF 100
#define F_ENTRY 0

#define KB 1000
#define MB 1000000
#define GB 1000000000

#ifndef __TEST_H__
#define __TEST_H__

/*enum FileType {{{ */
enum
FileType
{
  IS_SYMDIR = 1,
  IS_SYMLINK,
  IS_DIRECTORY,
  IS_BLOCK_DEVICE,
  IS_DEVICE,
  IS_SOCKET,
  IS_REG_FILE
}; /* }}} */

/*type info {{{ */
typedef struct _INFO
{
  int file_count;
  int files_filtered;

}INFO; /* }}} */

/* type file {{{ */
typedef struct _Dir_Entry
{
  char *name;
  int nlinks;
  int file_t;
  int is_directory;
  int is_selected;
  int executable;

  double size;
  time_t mtime;
  time_t atime;
  time_t ctime;
  
  char size_str[100];
  char mtime_str[100];
  char atime_str[100];
  char ctime_str[100];

  char show_bits[11];
  char owner[50];
  char group[50];

  uid_t uid;
  gid_t gid;

  INFO *Info;

}Dir_Entry; /* }}} */

/* prototypes {{{ */
///////////////////////////////////////
int init_file_pointer(Dir_Entry ***dir_entry, int *entry_size, int idx);
int init_file(Dir_Entry ***dir_entry, char *entry_name, int *buff_size,  char *path, int idx);
int init_file_info(Dir_Entry ***dir_entry, char *entry_name, int idx, char *path);
int check_symdir(char *file);
int ret_filetype(char *file);
int check_dir_item(char *file);
int check_file_access(char *show_bits, char *file);
int get_owner_info(Dir_Entry **dir_entry, char *file);
int file_filter(char *dname, int f_filter, int *filtered);
int get_file_times(char *file, time_t *mt, time_t *at, time_t *ct);
int conv_time(time_t my_time, char *out_conv_time);
int get_files(Dir_Entry ***dir_entry, int *entry_size, char *path, int f_filter, int sort_o, int sort_m);
int get_number_of_links(char *file);
//////////////////////////////////////////////
double scan_recursive(char *path);
double get_filesize(char *file, int deepscan);
void convert_filesize(double file_size, char *out_format_size);
///////////////////////////////////
void free_dir_entry(Dir_Entry **dir_entry, int buffer_size);
void convert_filetype(int mode, int outp_s, char *outp);
void convert_files(char *path); /* }}} */

#endif




