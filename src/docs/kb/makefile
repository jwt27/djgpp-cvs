# Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details
TOP=..
include $(TOP)/../makefile.inc

all :: $(INFO)/kb.inf

K = $(wildcard *.txi)

$(INFO)/kb.inf : $K
	makeinfo --no-split kb.txi
