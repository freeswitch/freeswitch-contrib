AM_CFLAGS   = $(SWITCH_AM_CFLAGS)
AM_CPPFLAGS = $(SWITCH_AM_CXXFLAGS)
AM_LDFLAGS  = $(SWITCH_AM_LDFLAGS)
#we should set all these vars from configure, no reason to have these in each Makefile.am
LIBTOOL = echo "`link=\`echo $@|grep .la;echo $@|grep .so;echo $@|grep .dll\`;if test -n "$$link"; then echo Creating $@;fi`";`if test -z "$(VERBOSE)" ; \
then echo $(SHELL) $(switch_builddir)/quiet_libtool ;else echo $(SHELL) $(switch_builddir)/libtool;  fi`
AM_MAKEFLAGS=`test -n "$(VERBOSE)" || echo -s`
# Dirty trick to override the link output
LIBS+=> $(MODNAME).log || error="yes";if test -n "$(VERBOSE)" -o "$$error" = "yes";then cat $(MODNAME).log;fi;if test "$$error" = "yes";then exit 1;fi

moddir=$(prefix)/mod

MODNAME=mod_xml_radius
mod_LTLIBRARIES = mod_xml_radius.la
mod_xml_radius_la_SOURCES = mod_xml_radius.c
mod_xml_radius_la_CFLAGS = $(AM_CFLAGS) 
mod_xml_radius_la_CPPFLAGS = $(AM_CPPFLAGS)
mod_xml_radius_la_LIBADD=$(switch_builddir)/libfreeswitch.la
mod_xml_radius_la_LDFLAGS=-module -avoid-version -no-undefined

mod_xml_radius_la_LDFLAGS += -lfreeradius-client

#Override the linstall target so we just install the .so/.dylib

install-data-am: $(DESTDIR)$(PREFIX)/$(moddir)/$(MODNAME).$(DYNAMIC_LIB_EXTEN)

$(DESTDIR)$(PREFIX)/$(moddir)/$(MODNAME).$(DYNAMIC_LIB_EXTEN): $(MODNAME).la
	@echo installing $(MODNAME).$(DYNAMIC_LIB_EXTEN)
	@if [ -f .libs/$(MODNAME).$(DYNAMIC_LIB_EXTEN) ] ; then \
	  $(LIBTOOL) --mode=install $(INSTALL) .libs/$(MODNAME).$(DYNAMIC_LIB_EXTEN) $(DESTDIR)$(PREFIX)/$(moddir) >/dev/null  ; \
	else \
	  $(LIBTOOL) --mode=install $(INSTALL) $(MODNAME).$(DYNAMIC_LIB_EXTEN) $(DESTDIR)$(PREFIX)/$(moddir) >/dev/null  ; \
	fi
