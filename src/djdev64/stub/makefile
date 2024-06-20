TOP = ../../..
PREFIX ?= /usr/local
SOURCES = coff.c elf.c util.c stub.c dpmiwrp.c dos.c
OBJECTS = $(SOURCES:.c=.o)
CFLAGS += -iquote ../include -fpic -ggdb3 -Wall -Og \
  -DCRT0=\"$(PREFIX)/i386-pc-dj64/lib/crt0.elf\"

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

LIBDIR = $(TOP)/lib
LIBN = libdjstub64.so.0.2
LIB = $(LIBDIR)/$(LIBN)

all: $(LIBDIR)/libdjstub64.so

clean:
	$(RM) *.o $(LIBDIR)/libdjstub64.so*

$(LIBDIR):
	mkdir $@

$(LIB): $(OBJECTS) | $(LIBDIR)
	$(CC) -shared -Wl,-Bsymbolic -Wl,-soname=$(LIBN) \
		-Wl,--version-script=libdjstub.version -o $@ $^

$(LIBDIR)/libdjstub64.so: $(LIB)
	ln -sf $(LIBN) $@
