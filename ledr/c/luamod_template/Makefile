ENVIRONMENT_NAME= luamod
DRIVER_NAME= test

FS_SRCDIR = ../../../..

PREFIX= /usr/local
LUA_LIBDIR= $(PREFIX)/lib/lua/5.1
LUA_DIR= $(PREFIX)/share/lua/5.1
LUA_INC= $(FS_SRCDIR)/src/mod/languages/mod_lua/lua

LIB_OPTION= -shared #for Linux
#LIB_OPTION= -bundle -undefined dynamic_lookup #for MacOS X

LIBNAME= $(DRIVER_NAME).so

DRIVER_LIBS= -L/usr/lib64
DRIVER_INCS= -I/usr/include

WARN= -fPIC -Wall -Wmissing-prototypes -Wmissing-declarations -ansi -pedantic
INCS= -I$(LUA_INC)
CFLAGS= -O2 $(WARN) -I$(COMPAT_DIR) $(DRIVER_INCS) $(INCS) $(DEFS)
CC= gcc

OBJS= luamod.o driver.o
SRCS= luamod.h luamod.c $(DRIVER_NAME).c
AR= ar rcu
RANLIB= ranlib

#--

lib: $(LIBNAME)

$(LIBNAME): $(OBJS)
	export MACOSX_DEPLOYMENT_TARGET="10.3"; $(CC) $(CFLAGS) -o $@ $(LIB_OPTION) $(OBJS) $(DRIVER_LIBS)

install:
	mkdir -p $(LUA_LIBDIR)/$(ENVIRONMENT_NAME)
	cp src/$(LIBNAME) $(LUA_LIBDIR)/$(ENVIRONMENT_NAME)

clean:
	rm -f $(LIBNAME) *.o $(COMPAT_O)
