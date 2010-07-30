SHELL = /bin/sh
CC = @CC@

FLAGS= -W -Wmissing-prototypes -Wstrict-prototypes -fPIC -g3 -Wall $(CFLAGS)

all:


	$(CC) $(CFLAGS) \
	-shared -fPIC -o radcdr.so radcdr.c \
	-I../../ \
	-I@with_radius_includes@ \
	@with_radius_lib@ \
	@with_glib2_flags@ \
	-lconfig  -lglib-2.0 -lm -lz -lcrypt \
	../../stmtexpldr.o ../../stmtexp.o ../../log.o

install:
	cp radiuscdr.so $(libdir)/xmlcdrd/


clean:
	-rm -f *.o radcdr.so
