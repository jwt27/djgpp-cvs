# Copyright (C) 2013 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details

TOP=.
include $(TOP)/../makefile.inc

all :: native \
	$(BIN)/dxe3gen.exe \
	$(BIN)/dxegen.exe \
	$(BIN)/dxe3res.exe \
	$E

native :: $(HOSTBIN)/dxegen.exe
	$(NOP)

.o.h:
	$(CROSS_STRIP) --strip-unneeded $<
	$(HOSTBIN)/bin2h.exe $< $(basename $<) $@

$(BIN)/dxe3gen.exe : $(C) dxe3gen.o $(L) 
	$(LINK)
	$(EXE)

$(BIN)/dxegen.exe : $(HOSTBIN)/stubify.exe $(BIN)/dxe3gen.exe
	$(HOSTBIN)/stubify.exe -g $@
	$(HOSTBIN)/stubedit.exe $@ runfile=dxe3gen

$(BIN)/dxe3res.exe : $(C) dxe3res.o $(L)
	$(LINK)
	$(EXE)

CROSS_CC = $(word 1,$(CROSS_GCC))
$(HOSTBIN)/dxegen.exe : dxe3gen.c init1.h init2.h init3.h init4.h init5.h fini1.h fini2.h fini3.h fini4.h fini5.h
	$(GCC) -DDXE_LD=\"$(CROSS_LD)\" -DDXE_CC=\"$(CROSS_CC)\" -DDXE_AR=\"$(CROSS_AR)\" -DDXE_AS=\"$(CROSS_AS)\" dxe3gen.c -o $@

clean ::
	@-$(MISC) rm *.o *.h $(HOSTBIN)/dxegen.exe
