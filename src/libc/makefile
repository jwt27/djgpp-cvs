# Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details
# Note that makefile.oi, makefile.rf and makefile.sub are normally
# *generated* but they are included in the source distribution to ease
# cross-compiling.

TOP=.

LIBNAME=c

include $(TOP)/../makefile.lib

.PHONY: all info tex texinfo doc libc2.tex

MKDOC = $(HOSTBIN)/mkdoc.exe

all :: info

libc2.tex :
	$(MKDOC) . libc2.tex

info doc : libc.tex libc2.tex
	makeinfo --no-split libc.tex

dvi : libc.tex libc2.tex
	texi2dvi libc.tex

ps : libc.tex libc2.tex
	texi2ps -v libc.tex

clean ::
	@$(MISC) rm libc2.tex
	@$(MAKE) -C stubs $@
	@$(MAKE) -C crt0 $@
		@$(MISC) rm makefile.sub
