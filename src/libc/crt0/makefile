# Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details
TOP=..

SRC += _main.c
SRC += brk.c
SRC += c1args.c
SRC += c1loadef.c
SRC += c1pglob.c
SRC += crt1.c
SRC += mcount.c
SRC += memhandl.c

EXTRA_OBJS += $(LIB)/crt0.o
EXTRA_OBJS += $(LIB)/gcrt0.o

include $(TOP)/../makefile.inc

$(LIB)/crt0.o : crt0.S
	$(GCC) -o $@

$(LIB)/gcrt0.o : gcrt0.S
	$(GCC) -o $@
