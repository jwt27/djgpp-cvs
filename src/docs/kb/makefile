# Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details
TOP=..
include $(TOP)/../makefile.inc

all :: $(INFO)/kb.info

K = $(wildcard *.txi)

$(INFO)/kb.info : $K
	makeinfo --no-split kb.txi

clean ::
	$(MISC) rm $(INFO)/kb.info
