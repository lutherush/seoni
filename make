#!/bin/bash
# nerdfb makefile by lutherus 2012 //
#
echo -e "compile...

+-nerdfb.c    / seoni.h
+-config.c    / config.h
+-filedata.c  / filedata.h
+-sort.c      / sort.h
+-nfbui.c     / sfbui.h
+-tab.c       / tab.h
+-keys.c      / keys.h
+-mimetypes.c / mimetypes.h
+-utils.c     / utils.h
+-search.c    / search.h
----------------------------
"
#
#-compile-nerdfb-------------->
gcc -o seoni \
seoni.c \
filedata.c \
sort.c \
sfbui.c \
tab.c \
config.c \
keys.c \
mimetypes.c \
utils.c \
search.c \
-lncurses -g -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64
#-making workpath------------->







