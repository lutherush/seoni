/* mimetypes.c by lutherus 2012 */

/* includes {{{ */
#include "seoni.h"
#include "filedata.h"
#include "config.h"
#include "utils.h"
#include "mimetypes.h" /* }}} */

#define MATCH 1

/* default_mimetype {{{ */
/* we don't need to read the
 * mime_catalog, thats mean
 * this way is faster for
 * standart mime.types */
char
*default_mimetypes[] =
{
  "inode/chardevice",
  "inode/blockdevice",
  "inode/directory-locked", //whait for bitmask is implemented
  "inode/directory",
  "inode/fifo",
  "inode/socked",
}; /* }}} */

/* *ret_dotnames {{{ */
char
*ret_mime_file_exts(char *mimebuf)
{
  char *mod;
  mod = strchr(mimebuf, '\t');
  snprintf(mimebuf, MAX_COLS, "%s", mod);
  mod = strtok(mimebuf, "\t");
  snprintf(mimebuf, MAX_COLS, "%s", mod);

  return mimebuf;
} /* }}} */

/* compare_extension {{{ */
int
compare_extension(const char *mfexts, const char *ext)
{
  /* the -1 represent the '\n'
   * that is written by fgets */

  int c = 0, space_c = 0;
  char mfext_tmp[MAX_COLS];
  size_t mfexts_len, ext_len;

  if( (mfexts_len = strlen(mfexts)-1) == (ext_len = strlen(ext)) )
  {
    if(mfexts[0] == ext[0])
      return MATCH;
  }
  else
  {
    int c2, last_c = 0, match_c = 0, n = 0;
    char tmp[MAX_COLS];

    snprintf(tmp, MAX_COLS, "%s", mfexts);
    while(tmp[c] != '\n')
    {
      if(tmp[c] == 040 || tmp[c+1] == '\n')
      {
        if(tmp[c+1] == '\n')
          n = 1;
        for(c2 = last_c; c2 < c+n; c2++)
          snprintf(&mfext_tmp[c2-last_c], MAX_COLS, "%c", tmp[c2]);

        if( (mfexts_len = strlen(mfext_tmp)) == (ext_len = strlen(ext)) )
        {
          for(c2 = 0; c2 < ext_len; c2++)
          {
            if(mfext_tmp[c2] == ext[c2])
            {
              match_c++;
            }
          }
          if(match_c == ext_len)
          {
            return MATCH;
            break;
          }
          match_c = 0;
        }
        last_c = c+1; // +1 goes one position forward to 040
        space_c++;
      }
      c++;
    }
  }
} /* }}} */

/* read_binary_type {{{ */
int
read_binary_type(char *filepath)
{
  FILE *filep;
  char binbuf[6], *bin;
  int c, pos = 0, match = 0;

  if( (filep = fopen(filepath, "r")) != NULL)
  {
    while( (c = fgetc(filep)) != EOF)
    {
      if(c == 69)
        binbuf[0] = c;
      else if(c == 76)
        binbuf[1] = c;
      else if(c == 70)
        binbuf[2] = c;
      else
      {
        if(pos > 4)
          break;
      }
      pos++;
    }
    binbuf[3] = '\0';
    bin = "ELF";
    for(pos = 0; pos < 3; pos++)
    {
      if(binbuf[pos] == bin[pos])
        match++;
    }
    if(match == strlen(bin))
      return ELF;
    else return 0;
  }
  else
  {
    //exeption
    return 0;
  }
  fclose(filep);
} /* }}} */

/* read_mimetype {{{ */
int
read_mimetype(char *filepath, char *type_out)
{
  mime.catalog = "/etc/mime.types";

  char *searched, *ext, *mfexts, *filename;
  char mime_tbuf[MAX_COLS];
  char type_buf[MAX_COLS];
  char tmp[MAX_COLS];

  int dotpos;
  
  FILE *mime_cat;

  if( (mime_cat = fopen(mime.catalog, "r")) == NULL)
  {
    fprintf(stdout, "/etc/mime.types not found\n");
    return 0;
  }
  if(ret_filetype(filepath) == IS_REG_FILE )
  {
    filename = conv_filename(filepath);
    dotpos = strcspn(filename, ".");
    if(dotpos == 0)
    {
       searched = "home/.dotfile";
       snprintf(type_out, 50, "%s", searched);
    }
    else
    {
      if((ext = strrchr(filename, '.')) != NULL)
      {
        ext++;
        snprintf(type_buf, MAX_COLS, "%s", ext);
        while(fgets(mime_tbuf, MAX_COLS, mime_cat) != NULL)
        {
          if(mime_tbuf[0] != ('#' | 040))
          {
            strncpy(tmp, mime_tbuf, MAX_COLS);
            mfexts = ret_mime_file_exts(tmp);
            snprintf(type_out, MAX_COLS, "%s", mfexts);
            if(strstr(type_out, type_buf) != NULL)
            {
              if(compare_extension(type_out, type_buf) == MATCH)
                break;
            }
            else
            {
              strncpy(mime_tbuf, "text/plain", MAX_COLS);
            }
          }
        }
        searched = strtok(mime_tbuf, "\t");
        snprintf(type_out, MAX_COLS, "%s", searched);
      }
      else
      {
        if(read_binary_type(filepath) == ELF)
          searched = "application/x-executable";
        else searched = "text/plain";
        snprintf(type_out, 50, "%s", searched);
      }
    }
  }
  if(ret_filetype(filepath) == IS_DIRECTORY)
    snprintf(type_out, 50, "%s", default_mimetypes[3]);
  fclose(mime_cat);
} /* }}} */
