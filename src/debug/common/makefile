# Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details
TOP=..

SRC += dbgcom.c
SRC += syms.c
SRC += v2load.c
SRC += wild.c
SRC += dbgredir.c

include $(TOP)/../makefile.inc

#relax error checking
XGCC=$(XLGCC)

e : $(BIN)/symify.exe
	$(NOP)

$(BIN)/symify.exe : $C symify.o $(LIB)/libdbg.a $L
	$(LINK)
	$(EXE)
