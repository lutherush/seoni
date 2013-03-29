/* mimetypes.h by lutherus 2012 */

/* BINARYS {{{ */
enum
Binary
{
  ELF = 1
}; /* }}} */

/* typedefs {{{ */
typedef struct __mime
{
  char *catalog;
  char type[MAX_COLS];

} _mime;
_mime mime; /* }}} */

/* prototypes {{{ */
int read_mimetype(char *filepath, char *type_out);
int read_binary_type(char *filepath);
int compare_extension(const char *mfexts, const char *ext);
/////////////////////////////////////////////////
char *ret_mime_file_exts(char *mimebuf); /* }}} */
