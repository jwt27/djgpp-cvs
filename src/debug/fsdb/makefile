# Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details
TOP=..
CFLAGS = -DFULLSCR -O2

EOBJS += ed.o
EOBJS += expr.o
EOBJS += fullscr.o
EOBJS += screen.o
EOBJS += unassmbl.o

include $(TOP)/../makefile.inc

#relax error checking:
XGCC = $(XLGCC)

e : $(BIN)/fsdb.exe
	$(NOP)

$(BIN)/fsdb.exe : $C $(EOBJS) $(LIB)/libdbg.a $L
	$(LINK)
	$(EXE)
	
clean ::
	$(MISC) rm expr.c
