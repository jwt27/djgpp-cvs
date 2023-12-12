TOP ?= .
PREFIX ?= /usr/local
INSTALL ?= install

VERSION = 0.1
DJLIBC = $(TOP)/lib/libc.a
DJ64LIB = $(TOP)/lib/libdj64.so.0.1
DJ64DEVL = $(TOP)/lib/libdj64.so
DJDEV64LIB = $(TOP)/lib/libdjdev64.so.0.1
DJDEV64DEVL = $(TOP)/lib/libdjdev64.so

all: dj64.pc
	$(MAKE) -C src

install:
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/i386-pc-dj64/lib
	$(INSTALL) -m 0644 $(DJLIBC) $(DESTDIR)$(PREFIX)/i386-pc-dj64/lib
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/i386-pc-dj64/lib64
	$(INSTALL) $(DJ64LIB) $(DESTDIR)$(PREFIX)/i386-pc-dj64/lib64
	cp -fP $(DJ64DEVL) $(DESTDIR)$(PREFIX)/i386-pc-dj64/lib64
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/i386-pc-dj64/include
	cp -r $(TOP)/include $(DESTDIR)$(PREFIX)/i386-pc-dj64
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/lib
	$(INSTALL) $(DJDEV64LIB) $(DESTDIR)$(PREFIX)/lib
	cp -fP $(DJDEV64DEVL) $(DESTDIR)$(PREFIX)/lib
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/include/djdev64
	$(INSTALL) -m 0644 $(TOP)/src/djdev64/djdev64.h $(DESTDIR)$(PREFIX)/include/djdev64
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/lib/pkgconfig
	$(INSTALL) -m 0644 dj64.pc $(DESTDIR)$(PREFIX)/lib/pkgconfig

uninstall:
	$(RM) -r $(DESTDIR)$(PREFIX)/i386-pc-dj64
	$(RM) -r $(DESTDIR)$(PREFIX)/include/djdev64
	$(RM) $(DESTDIR)$(PREFIX)/lib/$(notdir $(DJDEV64LIB))

clean:
	$(MAKE) -C src clean

deb:
	debuild -i -us -uc -b

%.pc: %.pc.in makefile
	sed \
		-e 's!@PREFIX[@]!$(PREFIX)!g' \
		-e 's!@INCLUDEDIR[@]!$(INCLUDEDIR)!g' \
		-e 's!@LIBDIR[@]!$(LIBDIR)!g' \
		-e 's!@VERSION[@]!$(VERSION)!g' \
		$< >$@
