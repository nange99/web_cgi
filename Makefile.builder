include ../../common.mk

CFLAGS= -O2 -Wall -I. -I$(ROOTDIR)/include \
	-I$(ROOTDIR)/$(FSDIR)/include

LDFLAGS= -L$(ROOTDIR)/$(FSDIR)/lib -L$(ROOTDIR)/lib

all: config
	$(MAKE)

config: configure
	if [ ! -f Makefile ]; then \
		export CFLAGS="$(CFLAGS)"; \
		export LDFLAGS="$(LDFLAGS)"; \
		./configure \
		--host=powerpc-linux \
		--build='i386' \
		--prefix=$(ROOTDIR)/fs/wn \
		--bindir=$(ROOTDIR)/fs/wn/cgi \
		--datadir=$(ROOTDIR)/fs/wn/htdocs; \
	fi

configure:
	autoreconf -i

install:
	if [ -f Makefile ]; then \
		$(MAKE) install; \
	fi
	sudo chown 0:0 $(ROOTDIR)/$(FSDIR)/wn/cgi/web_digistar
	sudo chmod +s $(ROOTDIR)/$(FSDIR)/wn/cgi/web_digistar

clean:
	if [ -f Makefile ]; then \
		$(MAKE) clean; \
	fi

distclean:
	if [ -f Makefile ]; then \
		$(MAKE) distclean; \
	fi
