# Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details
TOP=.
include $(TOP)/../makefile.inc

all :: $(INFO)/dir
	$(MAKE) -C kb

$(INFO)/dir : $(INFO)/dir.txi
	makeinfo --no-split -o $(INFO)/dir $(INFO)/dir.txi

clean ::
	$(MAKE) -C kb clean
	$(MISC) rm $(INFO)/dir
