# Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details
# Note that makefile.oi, makefile.rf, and ar.in are normally *generated* but
# they are included in the source distribution to ease cross-compiling.

TOP=.

LIBNAME=c

include $(TOP)/../makefile.lib

.PHONY: all info tex texinfo doc libc2.tex

MKDOC = ../../bin/mkdoc

all : info

libc2.tex :
	$(MKDOC) . libc2.tex

info doc : libc.tex libc2.tex
	makeinfo --no-split libc.tex

dvi : libc.tex libc2.tex
	texi2dvi libc.tex

ps : libc.tex libc2.tex
	texi2ps -v libc.tex
