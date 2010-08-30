ENVIRONMENT_NAME= luamod
DRIVER_NAME= template

PREFIX= /usr/local

FS_ROOT = ../../../..

LUA_LIBDIR= $(PREFIX)/lib/lua/5.1
LUA_DIR= $(PREFIX)/share/lua/5.1

LIB_OPTION= -shared #for Linux
#LIB_OPTION= -bundle -undefined dynamic_lookup #for MacOS X

LIBNAME= $(DRIVER_NAME).so

DRIVER_LIBS= -L/usr/lib64

INCS= -I/usr/include -I$(FS_ROOT)/src/mod/languages/mod_lua/lua -I $(FS_ROOT)/src/include -I$(FS_ROOT)/libs/libteletone/src
CFLAGS= -O2 -fPIC -Wall -ansi -pedantic $(INCS)
CPPFLAGS= -O2 -x c++ -fPIC -Wall -ansi -pedantic $(INCS)
CC= gcc

OBJS= luamod.o $(DRIVER_NAME).o
SRCS= luamod.h luamod.c $(DRIVER_NAME).c
AR= ar rcu
RANLIB= ranlib

#--

#lib: $(LIBNAME)

$(LIBNAME): $(OBJS)
	$(CC) $(CPPFLAGS) -o $@ $(LIB_OPTION) $(OBJS) $(DRIVER_LIBS)

$(LIBNAME):
	$(CC) $(CFLAGS) -o $@ $(LIB_OPTION) $(OBJS) $(DRIVER_LIBS)

install:
	mkdir -p $(LUA_LIBDIR)/$(ENVIRONMENT_NAME)
	cp src/$(LIBNAME) $(LUA_LIBDIR)/$(ENVIRONMENT_NAME)

clean:
	rm -f $(LIBNAME) *.o $(COMPAT_O)
