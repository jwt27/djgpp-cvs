# Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details

TOP=.
include $(TOP)/../makefile.inc

all :: native \
	$(BIN)/dxegen.exe \
	$E

native :: \
	$(HOSTBIN)/dxegen.exe \
	$E

$(BIN)/dxegen.exe : dxegen.c $(L)
	$(XLGCC) -s dxegen.c -o $@

$(HOSTBIN)/dxegen.exe : dxegen.c
	$(GCC) -DDXE_LD=\"$(CROSS_LD)\" dxegen.c -o $@

clean ::
	@-$(MISC) rm *.o $(HOSTBIN)/dxegen.exe
