# Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details
TOP=.
DXE_LD_LIBRARY_PATH=$(TOP)/../../lib
export DXE_LD_LIBRARY_PATH

LIBNAME=emu

include $(TOP)/../makefile.lib

all ::	$(BIN)/emu387.dxe

$(BIN)/emu387.dxe : src/emu387.o src/emudummy.o
	$(HOSTBIN)/dxegen.exe $(BIN)/emu387.dxe __emu_entry src/emu387.o id_emu.o src/emudummy.o -L../../lib $(LIBGCCA) -lc $(LIBGCCA)
	
clean ::
	$(MISC) rm makefile.sub
