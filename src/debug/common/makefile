# Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details
TOP=..

SRC += dbgcom.c
SRC += syms.c
SRC += v2load.c
SRC += wild.c

include $(TOP)/../makefile.inc


e : $(BIN)/symify.exe

$(BIN)/symify.exe : $C symify.o $(LIB)/libdbg.a $L
	gcc -s -o $@ symify.o $(LIB)/libdbg.a 
