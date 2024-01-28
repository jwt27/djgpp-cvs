TOP = ../..
GCC = gcc

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

$(LIB): djdev64.o | $(LIBDIR)
	$(GCC) -shared -Wl,-soname=$(LIBN) -o $@ $^

$(LIBDIR)/libdjdev64.so: $(LIB)
	ln -sf $(LIBN) $@
