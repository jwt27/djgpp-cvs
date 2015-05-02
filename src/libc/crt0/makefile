# Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 2012 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 2011 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details
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
SRC += rfinfo.c
SRC += dfinfo.c

EXTRA_OBJS += $(LIB)/crt0.o
EXTRA_OBJS += $(LIB)/gcrt0.o

include $(TOP)/../makefile.inc

exit16.ah : exit16.asm
	$(HOSTBIN)/djasm.exe exit16.asm exit16.ah

sbrk16.ah : sbrk16.asm
	$(HOSTBIN)/djasm.exe sbrk16.asm sbrk16.ah

$(LIB)/crt0.o : crt0.S exit16.ah sbrk16.ah
	$(XGCC) -MMD -c crt0.S
	$(MISC) cp crt0.o $@
	$(MISC) rm crt0.o
	sed 's@crt0.o@$(LIB)/crt0.o@' crt0.d > crt02.d

$(LIB)/gcrt0.o : gcrt0.S crt0.S exit16.ah sbrk16.ah
	$(XGCC) -MMD -c gcrt0.S
	$(MISC) cp gcrt0.o $@
	$(MISC) rm gcrt0.o
	sed 's@gcrt0.o@$(LIB)/gcrt0.o@' gcrt0.d > gcrt02.d

mcount.o : mcount.c
	$(XNOPGGCC) -c $<
