SHELL = /bin/sh
CC = @CC@

FLAGS= -W -Wmissing-prototypes -Wstrict-prototypes -g3 -Wall $(CFLAGS)

all:
	$(CC) $(CFLAGS) \
	-shared -fPIC -o luaexec.so luaexec.c \
	-I../../ \
        -I@with_lua_includes@ \
	@with_glib2_flags@ \
	@with_lua_lib@ -ldl -lglib-2.0 \
	-lconfig \
	../../stmtexpldr.o ../../stmtexp.o ../../log.o

install:
	-cp luaexec.so  $(libdir)/xmlcdrd/

clean:
	-rm -f *.o luaexec.so
