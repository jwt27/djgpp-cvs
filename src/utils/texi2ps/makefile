# Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details
CFLAGS = -O2 -Wimplicit
LDFLAGS = -s

OBJS = \
	fileio.o \
	ps.o \
	screenio.o \
	texi2ps.o \
	word.o \
	ifset.o \
	$E

../../../bin/texi2ps.exe: $(OBJS)
	gcc $(LDFLAGS) -o $@ $(OBJS)

%.o: %.c
	gcc -MMD -c $(CFLAGS) $<

clean:
	-rm -f *.o *.exe *.d

DEPS := $(wildcard *.d)
ifneq ($(DEPS),)
include $(DEPS)
endif
