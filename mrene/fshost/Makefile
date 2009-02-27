CC=gcc
FS_PATH=../../../../
FS_INCLUDES=-I$(FS_PATH)/src/include -I$(FS_PATH)/libs/apr/include -I$(FS_PATH)/libs/libteletone/src
FS_LDFLAGS = -L$(FS_PATH)/.libs -lfreeswitch
CFLAGS=-ggdb3 $(FS_INCLUDES) -Wall -Werror
LIBS=$(FS_LDFLAGS)

all: fshost

clean:
	rm -rf fshost fshost.dSYM

fshost:  main.c
	$(CC) -o fshost main.c $(CFLAGS) $(LIBS)
