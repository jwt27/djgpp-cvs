# Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details
.PHONY : none all

TOP = ..

all : makefile.oh
	@rem.com

makefile.oh : $(TOP)/../../include/libc/stubs.h gen.exe makefile
	gen.exe $<

gen.exe : gen.c
	gcc -s gen.c -o gen.exe

clean :
	-rm -f *.o makefile.oh
