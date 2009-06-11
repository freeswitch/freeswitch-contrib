MODNAME=mod_khomp
LOCAL_CFLAGS=-I./include -I./commons/include -lk3l -D_REENTRANT -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -DK3L_HOSTSYSTEM -DKHOMP_COMMONS_WITH_CHANNEL -lboost_thread -lboost_regex

LOCAL_OBJS= ./commons/k3lapi.o

include ../../../../build/modmake.rules
