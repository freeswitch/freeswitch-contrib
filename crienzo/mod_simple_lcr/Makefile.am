include $(top_srcdir)/build/modmake.rulesam
MODNAME=mod_simple_lcr

mod_LTLIBRARIES = mod_simple_lcr.la
mod_simple_lcr_la_SOURCES  = mod_simple_lcr.c route_trie.c
mod_simple_lcr_la_CFLAGS   = $(AM_CFLAGS)
mod_simple_lcr_la_LIBADD   = $(switch_builddir)/libfreeswitch.la
mod_simple_lcr_la_LDFLAGS  = -avoid-version -module -no-undefined -shared

