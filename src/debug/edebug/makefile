# Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details
TOP=..

EOBJS += debug.o
EOBJS += ed.o
EOBJS += unassmbl.o

include $(TOP)/../makefile.inc

e : $(BIN)/edebug32.exe

$(BIN)/edebug32.exe : $C $(EOBJS) $(LIB)/libdbg.a $L
	ld -s -o $@ $^ -T$(LIB)/djgpp.lnk
	$(BIN)/stubify -v $@
