# Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details

TOP=..

EOBJS = \
	fileio.o \
	ps.o \
	screenio.o \
	texi2ps.o \
	word.o \
	ifset.o \
	$E

include $(TOP)/../makefile.inc
XGCC = $(XLGCC)

all :: $(BIN)/texi2ps.exe

$(BIN)/texi2ps.exe: $(C) $(EOBJS) $(L)
	$(LINK)
	$(EXE)
