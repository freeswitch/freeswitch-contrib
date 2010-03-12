SHELL = /bin/sh
VPATH = @srcdir@

confdir = "/etc/xmlcdrd"
subdirs = @subdirs@
top_srcdir = @top_srcdir@
srcdir = @srcdir@
prefix = @prefix@
exec_prefix = @exec_prefix@
bindir = $(exec_prefix)/bin
infodir = $(prefix)/info
libdir = $(prefix)/lib
includedir = $(prefix)/include
mandir = $(prefix)/man/man1
snapshot = $(srcdir)/../`date +xmlcdr_%Y%m%d_%H%M`

CC = @CC@
CPPFLAGS = @CPPFLAGS@
CFLAGS = $(CPPFLAGS) -fPIC -Wall @CFLAGS@
LDFLAGS = @LDFLAGS@
LIBS = @LIBS@
INCS = @INCS@
INSTALL = @INSTALL@

all:
	$(CC) $(CFLAGS) @with_glib2_flags@ -c stmtexpldr.c
	$(CC) $(CFLAGS) @with_glib2_flags@ -lconfig -c stmtexp.c
	$(CC) $(CFLAGS) @with_glib2_flags@ -c plug.c
	$(CC) $(CFLAGS) @with_glib2_flags@ -c plugldr.c
	$(CC) $(CFLAGS) -c log.c
	$(CC) $(CFLAGS) `@with_apreq2_config@ --libs --includes` \
`@with_apr_config@ --libs --includes  --cppflags` @with_glib2_flags@ -c urldecode.c

if BUILDMYSQLCDR
	$(MAKE) -C plugins/mysqlcdr
endif
	
if BUILDLUAEXEC
	$(MAKE) -C plugins/luaexec
endif

	$(CC) $(CFLAGS) \
 `@with_apreq2_config@  --libs --includes` \
 `@with_apr_config@ --libs --includes  --cppflags --link-ld ` \
  @with_glib2_flags@ -lglib-2.0 \
 `@with_xml2_config@ --cflags --libs` \
  $(LIBS) \
 stmtexpldr.o stmtexp.o log.o urldecode.o \
 plugldr.o plug.o \
-o xmlcdr.fcgi main.c


snapshot:
	-tar --exclude-vcs -zcf $(snapshot).tgz -R $(srcdir) ; fi 

install:
	-cp xmlcdr.fcgi $(bindir)/
	-if [ ! -d $(confdir) ]; then mkdir $(confdir); fi
	-cp conf/* $(confdir)/

if BUILDMYSQLCDR
	$(MAKE) -C plugins/mysqlcdr install
endif

if BUILDLUAEXEC
	$(MAKE) -C plugins/luaexec install
endif


clean:
	-rm -f *.o xmlcdr.fcgi

if BUILDMYSQLCDR
	$(MAKE) -C plugins/mysqlcdr clean
endif

if BUILDLUAEXEC
	$(MAKE) -C plugins/luaexec clean
endif
