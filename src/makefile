# Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details

# Note: you may have to stubedit make to have a transfer buffer of at
# least 32k to pass the long lists of object files around.

MAKEFLAGS := --no-print-directory

DIRS = \
	../hostbin	\
	../bin		\
	../include	\
	../info		\
	../lib

all : misc.exe $(DIRS) makemake.exe subs

misc.exe : misc.c
	gcc -O2 misc.c -o misc.exe

$(DIRS) :
	./misc.exe mkdir $@

makemake.exe : makemake.c
	gcc -O2 makemake.c -o makemake.exe

copyrite.exe : copyrite.cc
	gcc -O2 copyrite.cc -o copyrite.exe

subs:
	$(MAKE) -C stub native
	$(MAKE) -C dxe native
	$(MAKE) -C mkdoc
	$(MAKE) -C libc
	$(MAKE) -C debug
	$(MAKE) -C stub
	$(MAKE) -C dxe
	$(MAKE) -C libemu
	$(MAKE) -C libm
	$(MAKE) -C utils
	$(MAKE) -C docs/kb
	$(MAKE) -C ../zoneinfo/src

.PHONY : clean
clean : misc.exe makemake.exe
	$(MAKE) -C libc clean
	$(MAKE) -C libm clean
	$(MAKE) -C debug clean
	$(MAKE) -C libemu clean
	$(MAKE) -C utils clean
	$(MAKE) -C stub clean
	$(MAKE) -C dxe clean
	$(MAKE) -C mkdoc clean
	$(MAKE) -C docs/kb clean
	$(MAKE) -C ../zoneinfo/src clean
		./misc.exe rm makemake.exe copyrite.exe misc.exe
