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
AS_SRC += fpclassf.S
AS_SRC += fpclassd.S
AS_SRC += fpclassl.S
SRC += sgnbitd.c
SRC += sgnbitf.c
SRC += sgnbitld.c

include $(TOP)/../makefile.inc

fpclassf.o: fp-asm.h

fpclassd.o: fp-asm.h

fpclassl.o: fp-asm.h

fp-asm.h: fp-asm.sed $(TOP)/../../include/math.h
	sed -n -f fp-asm.sed $(TOP)/../../include/math.h > fp-asm.h

clean::
	$(MISC) rm fp-asm.h

nan.o: EXTRA_CFLAGS += -fno-builtin-nan
nanf.o: EXTRA_CFLAGS += -fno-builtin-nanf
nanl.o: EXTRA_CFLAGS += -fno-builtin-nanl
