# Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details
TOP = ..
include $(TOP)/../makefile.inc

STUBH = $(TOP)/../../include/libc/stubs.h

all :: makefil2
	$(MAKE) -f makefil2

makefil2 : mkstubs.exe  $(STUBH) makefile
	./mkstubs.exe $(STUBH)

mkstubs.exe : mkstubs.c
	$(GCC) mkstubs.c -o mkstubs.exe

clean ::
	@-$(MISC) rm *.S makefil2 mkstubs.exe
