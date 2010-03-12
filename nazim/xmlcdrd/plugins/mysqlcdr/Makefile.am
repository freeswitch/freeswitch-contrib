SHELL = /bin/sh
CC = @CC@

FLAGS= -W -Wmissing-prototypes -Wstrict-prototypes -fPIC -g3 -Wall $(CFLAGS)

all:
	$(CC) $(CFLAGS) -fPIC -W -Wall -I../../ `@with_mysql_config@ --include` -c dbcommon.c

	$(CC) $(CFLAGS) \
	-shared -fPIC -o mysqlcdr.so mysqlcdr.c \
	-I../../ \
	`@with_mysql_config@ --libs --include` \
	@with_glib2_flags@ \
	-lconfig  -lglib-2.0 \
	dbcommon.o \
	../../stmtexpldr.o ../../stmtexp.o ../../log.o

install:
	cp mysqlcdr.so $(libdir)/xmlcdrd/


clean:
	-rm -f *.o mysqlcdr.so
