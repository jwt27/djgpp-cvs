# Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details

# Note: you may have to stubedit make to have a transfer buffer of at
# least 32k to pass the long lists of object files around.

# You don't have to build the stuff in ../zoneinfo/src, but if you do
# unzip djtzsNNN.zip, you will need a Unixy shell to build it.
# If you don't have ../zoneinfo/src, Make will report (and ignore) an
# error when it tries to build files there; disregard it.

MAKEFLAGS := --no-print-directory

DIRS = \
	../hostbin	\
	../bin		\
	../include	\
	../info		\
	../lib

all : misc.exe config $(DIRS) makemake.exe subs

misc.exe : misc.c
	gcc -O2 -Wall misc.c -o misc.exe

$(DIRS) :
	./misc.exe mkdir $@

makemake.exe : makemake.c
	gcc -O2 -Wall makemake.c -o makemake.exe

copyrite.exe : copyrite.cc
	gcc -O2 -Wall copyrite.cc -o copyrite.exe

subs:
	$(MAKE) -C djasm native
	$(MAKE) -C utils native
	$(MAKE) -C dxe native
	$(MAKE) -C djdev64
	$(MAKE) -C mkdoc
	$(MAKE) -C libc

.PHONY : clean
clean : misc.exe makemake.exe
	$(MAKE) -C libc clean
	$(MAKE) -C libm clean
	$(MAKE) -C debug clean
	$(MAKE) -C libemu clean
	$(MAKE) -C utils clean
	$(MAKE) -C dxe clean
	$(MAKE) -C djdev64 clean
	$(MAKE) -C mkdoc clean
	$(MAKE) -C docs clean
	$(MAKE) -C djasm clean
	-$(MAKE) -C ../zoneinfo/src clean
	./misc.exe rm gpp.opt
	./misc.exe rm gcc.opt gcc-l.opt
	./misc.exe rm makemake.exe copyrite.exe misc.exe

config: misc.exe
	$(MAKE) -f makefile.cfg

rmake.exe: rmake.c
	gcc -O2 -Wall rmake.c -o rmake.exe
