# Makefile for popt library.

ACLOCAL_AMFLAGS = -I m4
AM_CPPFLAGS = -I. -I$(top_srcdir)
AM_CFLAGS = $(POPT_CFLAGS)
AM_LDFLAGS = $(POPT_LDFLAGS)

EXTRA_DIST = lookup3.c autogen.sh CHANGES  \
	footer_no_timestamp.html libpopt.vers \
	$(TESTS) test-poptrc\
	popt.xcodeproj/project.pbxproj \
	popt.ps Doxyfile 	test3-data/01.answer \
				test3-data/01.input \
				test3-data/02.answer \
				test3-data/02.input \
				test3-data/03.answer \
				test3-data/03.input

SUBDIRS = po . auto


noinst_HEADERS = poptint.h system.h

check_PROGRAMS  = test1 test2 tdict test3

check_SCRIPTS	= $(TESTS)


test1_CPPFLAGS  = -I $(top_builddir) 
test2_CPPFLAGS  = -I $(top_builddir) 
tdict_CPPFLAGS  = -I $(top_builddir) 
test3_CPPFLAGS  = -I $(top_builddir) 
test1_CFLAGS  = $(AM_CFLAGS) 
test2_CFLAGS  = $(AM_CFLAGS) 
tdict_CFLAGS  = $(AM_CFLAGS) 
test3_CFLAGS  = $(AM_CFLAGS) 
test1_LDFLAGS  = $(AM_LDFLAGS) 
test2_LDFLAGS  = $(AM_LDFLAGS) 
tdict_LDFLAGS  = $(AM_LDFLAGS) 
test3_LDFLAGS  = $(AM_LDFLAGS) 
test1_LDADD  = $(top_builddir)/$(usrlib_LTLIBRARIES)
test2_LDADD  = $(top_builddir)/$(usrlib_LTLIBRARIES)
tdict_LDADD  = $(top_builddir)/$(usrlib_LTLIBRARIES)
test3_LDADD  = $(top_builddir)/$(usrlib_LTLIBRARIES)

TESTS_ENVIRONMENT = top_srcdir=$(top_srcdir) MUDFLAP_OPTIONS="$(MUDFLAP_OPTIONS)" testpoptrc="$(top_srcdir)/test-poptrc" PATH=.:../src:$$PATH \
                        $(VALGRIND_ENVIRONMENT) \
			$(SHELL)


TESTS = testit.sh

include_HEADERS = popt.h

usrlibdir = $(libdir)
usrlib_LTLIBRARIES = libpopt.la

libpopt_la_SOURCES = popt.c poptparse.c poptconfig.c popthelp.c poptint.c
libpopt_la_LDFLAGS = -no-undefined @LTLIBINTL@ @LTLIBICONV@ $(AM_LDFLAGS)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = popt.pc

if HAVE_LD_VERSION_SCRIPT
libpopt_la_LDFLAGS += -Wl,@LD_VERSION_SCRIPT_FLAG@,@top_srcdir@/libpopt.vers
endif

dist_man_MANS = popt.3

BUILT_SOURCES = popt.pc 

distclean-local:
	rm -rf .ccache 

clean-local:
	rm -rf doxygen *.gcda *.gcno lcov-result popt-valgrind-result 

if HAVE_SPLINT
lint:
	$(SPLINT) ${DEFS} ${INCLUDES} test1.c ${libpopt_la_SOURCES}

popt.lcd: Makefile.am ${libpopt_la_SOURCES} ${include_HEADERS} ${noinst_HEADERS}
	$(SPLINT) -dump $@ ${libpopt_la_SOURCES}
endif 

if HAVE_MCCABE
mccabe:
	$(MCCABE) $(libpopt_la_SOURCES) | sort -n -r | head -n 10

endif

if DOXYGEN_ENABLE
doxygen: Doxyfile
	rm -rf doxygen
	mkdir -p doxygen
	$(DOXYGEN)
endif 

if HAVE_LCOV
# run lcov from scratch, always
lcov-reset:
	make lcov-run
	make lcov-report

# run lcov from scratch if the dir is not there
lcov:
	make lcov-reset

# reset run coverage tests
lcov-run:
	@-rm -rf lcov
	find . -name "*.gcda" -exec rm {} \;
	make check

# generate report based on current coverage data
lcov-report:
	rm -rf lcov-result
	mkdir lcov-result
	$(LCOV) --directory . --capture --output-file lcov-result/lcov.info
	$(LCOV) -l lcov-result/lcov.info | grep -v "`cd $(top_srcdir) && pwd`" | cut -d: -f1 > lcov-result/remove
	$(LCOV) -r lcov-result/lcov.info `cat lcov-result/remove` > lcov-result/lcov.cleaned.info
	rm lcov-result/remove
	mv lcov-result/lcov.cleaned.info lcov-result/lcov.info
	genhtml -t "$(PACKAGE_STRING)" -o lcov lcov-result/lcov.info
endif

po_dir=./po
.PHONY:	updatepo
updatepo:
	rsync -Lrtvz  translationproject.org::tp/latest/popt/  po
	ls "$(po_dir)"/*.po 2>/dev/null | LANG=C sed 's|.*/||; s|\.po$$||' > "$(po_dir)/LINGUAS"

.PHONY:	sources
sources:
	@echo $(libpopt_la_SOURCES:%=popt/%)

