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

native :: \
	$(HOSTBIN)/dxegen.exe \
	$E

.o.h:
	strip --strip-unneeded $<
	bin2h $< $(basename $<) $@

$(BIN)/dxe3gen.exe : $(C) dxe3gen.o $(L) 
	$(LINK)
	$(EXE)

$(BIN)/dxegen.exe : $(HOSTBIN)/stubify.exe $(BIN)/dxe3gen.exe
	$(HOSTBIN)/stubify.exe -g $@
	$(HOSTBIN)/stubedit.exe $@ runfile=dxe3gen

$(BIN)/dxe3res.exe : $(C) dxe3res.o $(L)
	$(LINK)
	$(EXE)

$(HOSTBIN)/dxegen.exe : dxe3gen.c init1.h init2.h init3.h fini1.h fini2.h fini3.h
	$(GCC) -I ../../include -DDXE_LD=\"$(CROSS_LD)\" dxe3gen.c -o $@

clean ::
	@-$(MISC) rm *.o *.h $(HOSTBIN)/dxegen.exe
