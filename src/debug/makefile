# Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details
TOP=.

LIBNAME=dbg

include $(TOP)/../makefile.lib

all ::
	$(MAKE) -f makefile.sub SUBARGS="e"

e:
	@rem

clean ::
	$(MAKE) -C fsdb clean
	$(MAKE) -C edebug clean
	$(MAKE) -C common clean
		$(MISC) rm makefile.sub
