# Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details
TOP=../..

SRC += hugevalf.c
SRC += hugevall.c
SRC += nan.c
SRC += fpclassf.S
SRC += fpclassd.S
SRC += fpclassl.S

include $(TOP)/../makefile.inc

fpclassf.S: fp-asm.h

fpclassd.S: fp-asm.h

fpclassl.S: fp-asm.h

fp-asm.h: fp-asm.sed $(TOP)/../../include/math.h
	sed -n -f fp-asm.sed $(TOP)/../../include/math.h > fp-asm.h

clean::
	$(MISC) rm fp-asm.h
