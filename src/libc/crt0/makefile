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

exit16.ah : exit16.asm
	djasm exit16.asm exit16.ah

sbrk16.ah : sbrk16.asm
	djasm sbrk16.asm sbrk16.ah

$(LIB)/crt0.o : crt0.S exit16.ah sbrk16.ah
	$(GCC) -o $@
	sed 's@crt0.o@$(LIB)/crt0.o@' crt0.d > crt02.d

$(LIB)/gcrt0.o : gcrt0.S exit16.ah sbrk16.ah
	$(GCC) -o $@
	sed 's@gcrt0.o@$(LIB)/gcrt0.o@' gcrt0.d > gcrt02.d
