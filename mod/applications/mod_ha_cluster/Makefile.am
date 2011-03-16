include $(top_srcdir)/build/modmake.rulesam
MODNAME=mod_ha_cluster

mod_LTLIBRARIES = mod_ha_cluster.la
mod_ha_cluster_la_SOURCES  = mod_ha_cluster.c ha_cluster_config.c ha_cluster_util.c \
			     ha_cluster_interfaces.c
mod_ha_cluster_la_CFLAGS   = $(AM_CFLAGS) -I.
mod_ha_cluster_la_LIBADD   = $(switch_builddir)/libfreeswitch.la
mod_ha_cluster_la_LDFLAGS  = -avoid-version -module -no-undefined -shared 

