MODNAME=mod_khomp
VERBOSE=1
LOCAL_CFLAGS=-I./include -I./commons -D_REENTRANT -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -DK3L_HOSTSYSTEM -g -ggdb
LOCAL_LDFLAGS=-lk3l
LOCAL_OBJS= ./commons/k3lapi.o ./commons/config_options.o ./commons/format.o ./commons/strings.o
LOCAL_OBJS+= ./src/globals.o ./src/opt.o ./src/khomp_pvt.o

include ../../../../build/modmake.rules
