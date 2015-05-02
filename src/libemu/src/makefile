# Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 2010 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details
TOP=..

SRC += emu387.cc
SRC += npxsetup.o

EXTRA_OBJS += emudummy.o

include $(TOP)/../makefile.inc

# FIXME: fix emu387.cc instead to avoid need for -fno-strict-aliasing

%.o : %.cc
	$(XNOPGGPP) -c -fno-exceptions -fno-strict-aliasing $<

%.o : %.c
	$(XNOPGGCC) -c $<
