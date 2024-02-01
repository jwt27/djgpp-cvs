TOP = ../..
GCC = gcc
PKG_CONFIG ?= pkg-config
SOURCES = djdev64.c elf.c
OBJECTS = $(SOURCES:.c=.o)
LIBELF := $(shell $(PKG_CONFIG) --libs libelf)
ifeq ($(LIBELF),)
$(error libelf not installed)
endif

%.o: %.c
	$(GCC) $(CFLAGS) -iquote ../../include -fpic -ggdb3 -Wall -c -o $@ $<

LIBDIR = $(TOP)/lib
LIBN = libdjdev64.so.0.1
LIB = $(LIBDIR)/$(LIBN)

all: $(LIBDIR)/libdjdev64.so

clean:
	$(RM) *.o $(LIBDIR)/libdjdev64.so*

$(LIBDIR):
	mkdir $@

$(LIB): $(OBJECTS) | $(LIBDIR)
	$(GCC) -shared -Wl,-soname=$(LIBN) -o $@ $^ $(LIBELF)

$(LIBDIR)/libdjdev64.so: $(LIB)
	ln -sf $(LIBN) $@
