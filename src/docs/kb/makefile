# Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details
TOP=..
include $(TOP)/../makefile.inc

all :: $(INFO)/kb.info

K = $(wildcard *.txi)

$(INFO)/kb.info : $K
	makeinfo --no-split kb.txi

html-dir: $K
	$(MISC) rm html/*
	$(MISC) rmdir html
	$(MISC) mkdir html
	makeinfo --html kb.txi -o html

html-single: $K
	makeinfo --html --no-split kb.txi -o html

clean ::
	$(MISC) rm $(INFO)/kb.info
	$(MISC) rm html/* kb.html
	$(MISC) rmdir html
