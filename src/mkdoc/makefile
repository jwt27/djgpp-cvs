# Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details

TOP = .
include $(TOP)/../makefile.inc

all :: $(HOSTBIN)/mkdoc.exe

$(HOSTBIN)/mkdoc.exe : mkdoc.cc
	$(GCC) mkdoc.cc $(LIBSTDCXX) -o $@

clean ::
	@-$(MISC) rm $(HOSTBIN)/mkdoc.exe
