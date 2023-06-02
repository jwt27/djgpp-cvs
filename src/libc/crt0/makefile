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
#SRC += mcount.c
SRC += memhandl.c
SRC += rfinfo.c
SRC += dfinfo.c

include $(TOP)/../makefile.inc

exit16.ah : exit16.asm
	$(HOSTBIN)/djasm.exe exit16.asm exit16.ah

sbrk16.ah : sbrk16.asm
	$(HOSTBIN)/djasm.exe sbrk16.asm sbrk16.ah

_crt0.o : crt0.S exit16.ah sbrk16.ah
	$(XGCC) -MMD -m32 -c -o $@ $<

$(LIB)/crt0.o : crt0.S plt.S ../plt.inc ../plt_asms.inc
	$(XGCC) -m32 -DWITH_PLT $< -c -o $@

$(LIB)/gcrt0.o : gcrt0.S crt0.S exit16.ah sbrk16.ah
	$(XGCC) -MMD -m32 -c gcrt0.S
	$(MISC) cp gcrt0.o $@
	$(MISC) rm gcrt0.o
	sed 's@gcrt0.o@$(LIB)/gcrt0.o@' gcrt0.d > gcrt02.d

final : $(LIB)/crt0.o $(LIB)/gcrt0.o

mcount.o : mcount.c
	$(XNOPGGCC) -c $<
