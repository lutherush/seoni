/* filedata.c by lutherus 2012 */

/* includes {{{ */
#include "seoni.h"
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include "config.h"
#include "filedata.h"
#include "sort.h"
#include "utils.h"
#include <pwd.h>
#include <grp.h> /* }}} */

/* check_symdir {{{ */
int
check_symdir(char *file)
{
  int ret = 0;
  struct stat attrib;

  if(stat(file, &attrib) != -1)
  {
    if(attrib.st_mode & S_IFDIR)
      ret = IS_SYMDIR;
  }
  return ret;
} /* }}} */

/* check_file_type {{{ */
int
ret_filetype(char *file)
{
  struct stat attrib;

  if(lstat(file, &attrib) == -1)
    return 0;

  if(S_ISLNK(attrib.st_mode))
  {
    if(check_symdir(file) != 0)
      return IS_SYMDIR;
    else return IS_SYMLINK;
  }
  else if(S_ISDIR(attrib.st_mode))
    return IS_DIRECTORY;
  else if(S_ISBLK(attrib.st_mode))
    return IS_BLOCK_DEVICE;
  else if(S_ISCHR(attrib.st_mode))
    return IS_DEVICE;
  else if(S_ISSOCK(attrib.st_mode))
    return IS_SOCKET;
  else return IS_REG_FILE;
} /* }}} */

/* convert_filetype {{{ */
void
convert_filetype(int mode, int outp_s, char *outp)
{
  char *ft[] = { "Symlink", "Directory", "Block Device",
                 "Device", "Socket", "Reg. File"
  };
  int mode_c;
  for(mode_c = 0; mode_c < 6; mode_c++)
  {
    if(mode == mode_c)
    {
      snprintf(outp, outp_s, "%s", ft[mode_c]);
      break;
    }
  }
} /* }}} */

/* check_file_access {{{ */
int
check_file_access(char *show_bits, char *file)
{
  struct stat attr;
  int idx = 0;

  if(lstat(file, &attr) == -1)
    return 0;
  else
  {
    if(S_ISLNK(attr.st_mode)) show_bits[0] = 'l';
    else if(S_ISDIR(attr.st_mode)) show_bits[0] = 'd';
    else if(S_ISBLK(attr.st_mode)) show_bits[0] = 'b';
    else if(S_ISCHR(attr.st_mode)) show_bits[0] = 'c';
    else if(S_ISSOCK(attr.st_mode)) show_bits[0] = 's';
    else show_bits[0] = '-';

    idx++;
    /* user */
    show_bits[idx++] = attr.st_mode & S_IRUSR ? 'r' : '-';
    show_bits[idx++] = attr.st_mode & S_IWUSR ? 'w' : '-';

    if(attr.st_mode & S_ISUID)
      show_bits[idx++] = attr.st_mode & S_IXUSR ? 's' : 'S';
    else show_bits[idx++] = attr.st_mode & S_IXUSR ? 'x' : '-';

    /* group */
    show_bits[idx++] = attr.st_mode & S_IRGRP ? 'r' : '-';
    show_bits[idx++] = attr.st_mode & S_IWGRP ? 'w' : '-';

    if(attr.st_mode & S_ISGID)
      show_bits[idx++] = attr.st_mode & S_IXGRP ? 's' : 'S';
    else show_bits[idx++] = attr.st_mode & S_IXGRP ? 'x' : '-';

    /* other */
    show_bits[idx++] = attr.st_mode & S_IROTH ? 'r' : '-';
    show_bits[idx++] = attr.st_mode & S_IWOTH ? 'w' : '-';

    if(attr.st_mode & S_ISVTX)
      show_bits[idx++] = attr.st_mode & S_IXOTH ? 't' : 'T';
    else show_bits[idx++] = attr.st_mode & S_IXOTH ? 'x' : '-';

    show_bits[idx] = '\0';

    return 1;
  }
} /* }}} */

/* get_owner_info {{{ */
int
get_owner_info(Dir_Entry **dir_entry, char *file)
{
  struct group *grp_buf;
  struct passwd *pwd_buf;
  struct stat attr;

  if(stat(file, &attr) == -1) return 0;
  else
  {
    (*dir_entry)->uid = attr.st_uid;
    (*dir_entry)->gid = attr.st_gid;
  }
  if((pwd_buf = getpwuid((*dir_entry)->uid)) == NULL)
    return 0;
  else
    snprintf( (*dir_entry)->owner, 
        sizeof( (*dir_entry)->owner), "%s", pwd_buf->pw_name);

  if((grp_buf = getgrgid((*dir_entry)->gid)) == NULL)
    return 0;
  else
    snprintf( (*dir_entry)->group,
        sizeof( (*dir_entry)->group), "%s", grp_buf->gr_name);
  return 1;
} /* }}} */

/* scan_recursive {{{ */
double
scan_recursive(char *path)
{
  DIR *dir;
  double file_size;
  struct dirent *dirptr;
  struct stat attr;

  if((dir = opendir(path)) == NULL)
    return 0;
  else
  {
    chdir(path);
    while((dirptr = readdir(dir)) != NULL)
    {
      lstat(dirptr->d_name, &attr);
      if(S_ISDIR(attr.st_mode))
      {
        if(strcmp(".", dirptr->d_name) == 0 ||
            strcmp("..", dirptr->d_name) == 0)
        {
          continue;
        }
        file_size += attr.st_size;
        scan_recursive(dirptr->d_name);
      }
      else
      {
        //do it with files
        file_size += attr.st_size;
      }
    }
    chdir("..");
    closedir(dir);
    return file_size;
  }
} /* }}} */

/* get_file_times {{{ */
int
get_file_times(char *file, time_t *mt, time_t *at, time_t *ct)
{
  struct stat attr;

  if(lstat(file, &attr) > 0)
    return 0;
  else
  {
    *mt = attr.st_mtime;
    *at = attr.st_atime;
    *ct = attr.st_ctime;
  }
  return 1;
} /* }}} */

/* conv_mtime {{{ */
int
conv_time(time_t my_time, char *out_conv_time)
{
  time_t *mt = &my_time;
  struct tm *timeptr;

  timeptr = localtime(mt);
 
  if(timeptr != NULL
      && (strftime(out_conv_time, 
          MAX_SIZE, "%Y-%m-%d %H:%M", timeptr) != 0))
    return 1;
  else return 0;
} /* }}} */

/*  get_file_s_inbyte {{{ */
double
get_filesize(char *file, int deepscan)
{
  double file_size;
  struct stat attr;

  if(stat(file, &attr) == -1)
    return -1;
  else
  {
    if(ret_filetype(file) == IS_DIRECTORY)
    {
      file_size = attr.st_size;

      if(deepscan == ON)
        file_size = scan_recursive(file);
    }
    else file_size = attr.st_size;
  }
  return file_size;
} /* }}} */

/* get_number_of_links * {{{ */
int
get_number_of_links(char *file)
{
  int num;
  struct stat attr;

  if(lstat(file, &attr) == -1)
    return 0;
  else num = attr.st_nlink;
  return num;
} /* }}} */

/* conv_filesize {{{ */
void
convert_filesize(double file_size, char *out_format_size)
{
  int s_dec = 0;
  double n_fsize;
  char *declar[] = { " B", "KB", "MB", "GB" };

  if(file_size >= KB && file_size < MB)
  {
    n_fsize = (file_size / KB);
    s_dec = 1;
  }
  else if(file_size >= MB && file_size < GB)
  {
    n_fsize = (file_size / MB);
    s_dec = 2;
  }
  else if(file_size > MB)
  {
    n_fsize = (file_size / GB);
    s_dec = 3;
  }
  else n_fsize = file_size;

  snprintf(out_format_size, MAX_SIZE,
      "%.1f %s", n_fsize, declar[s_dec]);

} /* }}} */

/* init_file_pointer {{{ */
int
init_file_pointer(Dir_Entry ***dir_entry, int *buff, int entry_count)
{
  int count = 0, old_buff = (*buff);
  if( (*dir_entry) == NULL)
  {
    if(( (*dir_entry) =
          (Dir_Entry **)malloc((*buff) * (sizeof(Dir_Entry))) ) == NULL)
      return 0;
    for(; count <= (*buff); count++)
      (*(*dir_entry+count)) = NULL;
    return 1;
  }
  else if( (entry_count+1) == (*buff))
  {
    (*buff) += (entry_count + 10);

    if(( (*dir_entry) = (Dir_Entry **)realloc( (*dir_entry),
            (*buff) * sizeof(Dir_Entry)) ) == NULL)
      return 0;
    for(; count <= (*buff); count++)
    {
      if(count >= old_buff)
        (*(*dir_entry+count)) = NULL;
    }
    return 1;
  }
  else return 1;
} /* }}} */

/* init_file {{{ */
int
init_file(Dir_Entry ***dir_entry, char *entry_name, int *buff_size, char *path, int idx)
{
  int entry_len = (strlen(entry_name));

  if(init_file_pointer(dir_entry,
        buff_size, idx) == 0) return 0;

  if( (*(*dir_entry+idx)) != NULL)
  {
    if(( (*(*dir_entry+idx))->name =
          (char *)realloc( (*(*dir_entry+idx))->name,
            entry_len * sizeof(char)+1) ) == NULL)
      return 0;
  }
  else
  {
    if(( (*(*dir_entry+idx)) = malloc(sizeof(Dir_Entry)) ) == NULL)
      return 0;
    else
    {
      if(( (*(*dir_entry+idx))->name =
            (char *)malloc(entry_len * sizeof(char)+1) ) == NULL)
        return 0;
      (*(*dir_entry+idx))->nlinks = 0;
      (*(*dir_entry+idx))->file_t = 0;
      (*(*dir_entry+idx))->is_directory = 0;
      (*(*dir_entry+idx))->is_selected = 0;
      (*(*dir_entry+idx))->executable = 0;
      (*(*dir_entry+idx))->size = 0;
      (*(*dir_entry+idx))->mtime = 0;
      (*(*dir_entry+idx))->atime = 0;
      (*(*dir_entry+idx))->ctime = 0;
      (*(*dir_entry+idx))->uid = 0;
      (*(*dir_entry+idx))->gid = 0;
      (*(*dir_entry+idx))->owner[0] = '\0';
      (*(*dir_entry+idx))->group[0] = '\0';
      (*(*dir_entry+idx))->size_str[0] = '\0';
      (*(*dir_entry+idx))->mtime_str[0] = '\0';
      (*(*dir_entry+idx))->atime_str[0] = '\0';
      (*(*dir_entry+idx))->ctime_str[0] = '\0';
      (*(*dir_entry+idx))->show_bits[0] = '\0';

      if(idx == 0) (*(*dir_entry+idx))->Info =
        (INFO *)malloc(sizeof(INFO));
      else (*(*dir_entry+idx))->Info = (*(*dir_entry))->Info;
    }
  }
  strncpy( (*(*dir_entry+idx))->name,
      entry_name, entry_len * sizeof(char)+1);

  if(init_file_info(dir_entry, entry_name, idx, path) == 0) return 0;
  return 1;
} /* }}} */

/* init_file_info {{{ */
int
init_file_info(Dir_Entry ***dir_entry, char *entry_name, int idx, char *path)
{
  int file_t = 0, path_len = (strlen(entry_name) + strlen(path)+2);
  char absol_p[path_len];

  build_path(path, entry_name, absol_p, path_len);

  get_file_times(absol_p, &(*(*dir_entry+idx))->mtime,
      &(*(*dir_entry+idx))->atime, &(*(*dir_entry+idx))->ctime);

  (*(*dir_entry+idx))->nlinks =
    get_number_of_links(absol_p);

  (*(*dir_entry+idx))->size = get_filesize(absol_p, DEEPOFF);
  convert_filesize( (*(*dir_entry+idx))->size,
      (*(*dir_entry+idx))->size_str);

  file_t = ret_filetype(absol_p);
  if(file_t != 0) (*(*dir_entry+idx))->file_t = file_t;
  else return 0;

  if(file_t == IS_DIRECTORY || file_t == IS_SYMDIR)
    (*(*dir_entry+idx))->is_directory = 1;
  else (*(*dir_entry+idx))->is_directory = 0;

  if(access(absol_p, 01) != -1)
    (*(*dir_entry+idx))->executable = 1;
  else (*(*dir_entry+idx))->executable = 0;

  (*(*dir_entry+idx))->is_selected = 0;

  return 1;
} /* }}} */

/*free_dir_entry {{{ */
void
free_dir_entry(Dir_Entry **dir_entry, int buffer_size)
{
  int idx = buffer_size;

  for(; idx >= 0; idx--)
  {
    if(dir_entry[idx] != NULL)
    {
      free(dir_entry[idx]->name);
      if(idx == 0) free(dir_entry[idx]->Info);
      free(dir_entry[idx]);
    }
  }
  free(dir_entry);
} /* }}} */

/* file_filter {{{ */
int
file_filter(char *dname, int f_filter, int *filtered)
{
  if(ret_dot_type(dname) == CWD_DOT ||
      ret_dot_type(dname) == DIRUP) return 0;
  else if(f_filter == ON)
  {
    // any other filter rules can later define here
    if(ret_dot_type(dname) != REGDOT) return 1;
    else
    {
      *filtered += 1;
      return 0;
    }
  }
  else return 1;
} /* }}} */

/* get_files {{{ */
int
get_files(Dir_Entry ***dir_entry, int *entry_size, char *path, int f_filter, int sort_o, int sort_m)
{
  DIR *dir;
  struct dirent *dirptr;
  int visible = 0, filtered = 0;

  if(init_file(dir_entry, "..",
        entry_size, path, visible) == 0) return 0;
  else visible++;
  if( (dir = opendir(path)) == NULL) return 0;
  while((dirptr = readdir(dir)) != NULL)
  {
    if(file_filter(dirptr->d_name, f_filter, &filtered) != 0)
    {
      if(init_file(dir_entry, dirptr->d_name,
            entry_size, path, visible) == 0) return 0;
      else visible++;
    }
    else continue;
  }
  closedir(dir);
  (*(*dir_entry))->Info->file_count = visible;
  (*(*dir_entry))->Info->files_filtered = filtered;
  // if folder empty
  if( (*(*dir_entry))->Info->file_count == 1)
  {
    if(init_file(dir_entry, ".",
          entry_size, path, 1) == 0) return 0;
    else (*(*dir_entry))->Info->file_count++;
  }
  else sort_files(dir_entry, sort_o, sort_m, path);
  return 1;
} /* }}} */
