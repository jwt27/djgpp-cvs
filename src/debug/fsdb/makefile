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

GCC = gcc -I. -I- -I$(TOP)/../../include $(CFLAGS) -c $<

e : $(BIN)/fsdb.exe

$(BIN)/fsdb.exe : $C $(EOBJS) $(LIB)/libdbg.a $L
	ld -o $@ $^ -T$(LIB)/djgpp.lnk
	$(BIN)/stubify -v $@
