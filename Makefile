
TARGET=seoni
SOURCE = $(wildcard *.c)
HEADER = $(wildcard *.h)
OBJECT= $(patsubst %.c, %.o, $(SOURCE))

FLAGS += -Wall
FLAGS += -std=gnu99
FLAGS += -pedantic
FLAGS += -g
CFLAGS += $(shell getconf LFS_CFLAGS)
#CFLAGS += -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64

LIBS += -lncurses


all: $(TARGET)

%.o: %.c 
	$(CC)  $(FLAGS) $(CFLAGS) $(LIBS) -c -o $@ $<

$(TARGET): $(OBJECT) $(HEADER)
	$(CC) $(FLAGS) $(CFLAGS) $(LIBS) $(OBJECT) -o $(TARGET)

clean: 
	rm -f $(OBJECT) $(TARGET)

.PHONY: clean all

