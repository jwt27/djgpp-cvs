# Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details
TOP=..

EOBJS += debug.o
EOBJS += ed.o
EOBJS += unassmbl.o

include $(TOP)/../makefile.inc

#relax error checking:
XGCC=$(XLGCC)

e : $(BIN)/edebug32.exe
	$(NOP)

$(BIN)/edebug32.exe : $C $(EOBJS) $(LIB)/libdbg.a $L
	$(LINK)
	$(EXE)
