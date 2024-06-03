TOP = ../../..
GCC = gcc
SOURCES = coff.c elf.c util.c stub.c dpmiwrp.c dos.c
OBJECTS = $(SOURCES:.c=.o)

%.o: %.c
	$(GCC) $(CFLAGS) -iquote ../include -fpic -ggdb3 -Wall -c -o $@ $<

LIBDIR = $(TOP)/lib
LIBN = libdjstub64.so.0.2
LIB = $(LIBDIR)/$(LIBN)

all: $(LIBDIR)/libdjstub64.so

clean:
	$(RM) *.o $(LIBDIR)/libdjstub64.so*

$(LIBDIR):
	mkdir $@

$(LIB): $(OBJECTS) | $(LIBDIR)
	$(GCC) -shared -Wl,-Bsymbolic -Wl,-soname=$(LIBN) \
		-Wl,--version-script=libdjstub.version -o $@ $^

$(LIBDIR)/libdjstub64.so: $(LIB)
	ln -sf $(LIBN) $@
