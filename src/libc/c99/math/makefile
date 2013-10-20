# Copyright (C) 2013 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details
TOP=../..

SRC += errhandl.c
SRC += hugevalf.c
SRC += hugevall.c
SRC += lrintf.c
SRC += lrint.c
SRC += lrintl.c
SRC += llrintf.c
SRC += llrint.c
SRC += llrintl.c
SRC += nan_def.c
SRC += nan.c
SRC += nanf.c
SRC += nanl.c
SRC += fpclassf.S
SRC += fpclassd.S
SRC += fpclassl.S
SRC += sgnbitd.c
SRC += sgnbitf.c
SRC += sgnbitld.c

include $(TOP)/../makefile.inc

fpclassf.S: fp-asm.h

fpclassd.S: fp-asm.h

fpclassl.S: fp-asm.h

fp-asm.h: fp-asm.sed $(TOP)/../../include/math.h
	sed -n -f fp-asm.sed $(TOP)/../../include/math.h > fp-asm.h

clean::
	$(MISC) rm fp-asm.h
