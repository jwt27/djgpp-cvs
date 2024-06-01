TOP ?= .
PREFIX ?= /usr/local
LIBDIR ?= $(PREFIX)/lib
INSTALL ?= install

VERSION = 0.1
DJLIBC = $(TOP)/lib/libc.a
DJ64LIB = $(TOP)/lib/libdj64.so.0.1
DJ64DEVL = $(TOP)/lib/libdj64.so
DJDEV64LIB = $(TOP)/lib/libdjdev64.so.0.1
DJDEV64DEVL = $(TOP)/lib/libdjdev64.so
DJSTUB64LIB = $(TOP)/lib/libdjstub64.so.0.1
DJSTUB64DEVL = $(TOP)/lib/libdjstub64.so

AS = $(CROSS_PREFIX)as
CROSS_PREFIX := i686-linux-gnu-
ifeq ($(shell $(AS) --version 2>/dev/null),)
CROSS_PREFIX := x86_64-linux-gnu-
endif
ifeq ($(shell $(AS) --version 2>/dev/null),)
ifeq ($(shell uname -m),x86_64)
CROSS_PREFIX :=
else
$(error cross-binutils not installed)
endif
endif

.PHONY: subs

all: dj64.pc dj64static.pc djdev64.pc djstub64.pc subs

subs:
	$(MAKE) -C src CROSS_PREFIX=$(CROSS_PREFIX)

install:
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/i386-pc-dj64/lib
	$(INSTALL) -m 0644 $(DJLIBC) $(DESTDIR)$(PREFIX)/i386-pc-dj64/lib
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/i386-pc-dj64/lib64
	$(INSTALL) $(DJ64LIB) $(DESTDIR)$(PREFIX)/i386-pc-dj64/lib64
	cp -fP $(DJ64DEVL) $(DESTDIR)$(PREFIX)/i386-pc-dj64/lib64
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/i386-pc-dj64/include
	cp -r $(TOP)/include $(DESTDIR)$(PREFIX)/i386-pc-dj64
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/include/djdev64
	cp -r $(TOP)/include/djdev64 $(DESTDIR)$(PREFIX)/include
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/i386-pc-dj64/bin
	$(INSTALL) -m 0755 ccwrp.sh $(DESTDIR)$(PREFIX)/i386-pc-dj64/bin
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/bin
	ln -sf ../i386-pc-dj64/bin/ccwrp.sh $(DESTDIR)$(PREFIX)/bin/dj64-gcc
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/share
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/share/pkgconfig
	$(INSTALL) -m 0644 dj64.pc $(DESTDIR)$(PREFIX)/share/pkgconfig
	$(INSTALL) -m 0644 dj64static.pc $(DESTDIR)$(PREFIX)/share/pkgconfig
	$(INSTALL) -m 0644 djdev64.pc $(DESTDIR)$(PREFIX)/share/pkgconfig
	$(INSTALL) -m 0644 djstub64.pc $(DESTDIR)$(PREFIX)/share/pkgconfig
	$(INSTALL) -d $(DESTDIR)$(LIBDIR)
	$(INSTALL) -m 0755 $(DJDEV64LIB) $(DESTDIR)$(LIBDIR)
	cp -fP $(DJDEV64DEVL) $(DESTDIR)$(LIBDIR)
	$(INSTALL) -m 0755 $(DJSTUB64LIB) $(DESTDIR)$(LIBDIR)
	cp -fP $(DJSTUB64DEVL) $(DESTDIR)$(LIBDIR)

uninstall:
	$(RM) -r $(DESTDIR)$(PREFIX)/bin/dj64-gcc
	$(RM) -r $(DESTDIR)$(PREFIX)/i386-pc-dj64
	$(RM) -r $(DESTDIR)$(PREFIX)/include/djdev64
	$(RM) $(DESTDIR)$(PREFIX)/share/pkgconfig/dj64.pc
	$(RM) $(DESTDIR)$(PREFIX)/share/pkgconfig/dj64static.pc
	$(RM) $(DESTDIR)$(PREFIX)/share/pkgconfig/djdev64.pc
	$(RM) $(DESTDIR)$(LIBDIR)/$(notdir $(DJDEV64DEVL))
	$(RM) $(DESTDIR)$(LIBDIR)/$(notdir $(DJDEV64LIB))

clean:
	$(MAKE) -C src clean
	$(RM) *.pc

deb:
	debuild -i -us -uc -b && $(MAKE) clean >/dev/null

rpm:
	make clean
	rpkg local && $(MAKE) clean >/dev/null

%.pc: %.pc.in makefile
	sed \
		-e 's!@PREFIX[@]!$(PREFIX)!g' \
		-e 's!@INCLUDEDIR[@]!$(INCLUDEDIR)!g' \
		-e 's!@LIBDIR[@]!$(LIBDIR)!g' \
		-e 's!@VERSION[@]!$(VERSION)!g' \
		$< >$@
