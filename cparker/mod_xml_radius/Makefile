RADCLIENT_VERSION=1.1.6
RADCLIENT=freeradius-client-$(RADCLIENT_VERSION)
RADCLIENT_DIR=$(switch_srcdir)/libs/$(RADCLIENT)
RADCLIENT_LIBDIR=$(RADCLIENT_DIR)/lib
RADCLIENT_LA=${RADCLIENT_LIBDIR}/libfreeradius-client.la

LOCAL_CFLAGS=-I$(RADCLIENT_DIR)/include
LOCAL_LIBADD=$(RADCLIENT_LA)

BASE=../../..
include $(BASE)/build/modmake.rules

$(RADCLIENT_DIR):
	$(GETLIB) $(RADCLIENT).tar.gz

$(RADCLIENT_DIR)/Makefile: $(RADCLIENT_DIR)
	cd $(RADCLIENT_DIR) && ./configure --disable-shared --with-pic
	$(TOUCH_TARGET)

$(RADCLIENT_LA): $(RADCLIENT_DIR)/Makefile
	cd $(RADCLIENT_DIR) && CFLAGS=$(CFLAGS) $(MAKE)
	$(TOUCH_TARGET)


