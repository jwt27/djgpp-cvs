# Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details
BIN=../../bin
INC=../../include
CFLAGS=-O3 -MD
STUBIFY=../../bin/stubify.exe
C_INCLUDE_PATH := ../../include
LIBRARY_PATH := ../../lib
export
PATH := $(PATH);$(BIN)

all :: \
	$(BIN)/djasm.exe \
	$(BIN)/go32-v2.exe \
	$(BIN)/stubedit.exe \
	$(BIN)/stubify.exe \
	$(BIN)/exe2coff.exe \
	$(INC)/stubinfo.h \
	$E

$(BIN)/% : %
	cp $< $@

sbrk16.ah : sbrk16.asm djasm.exe
	../bin/djasm.exe sbrk16.asm sbrk16.ah

stubify.exe : stubify
	$(STUBIFY) stubify

exe2coff.exe : exe2coff
	$(STUBIFY) exe2coff

stubify.o : stub.h

$(INC)/stubinfo.h : stub.asm stub.map stub2inc.exe
	stub2inc stub.map stub.asm $(INC)/stubinfo.h

stub.h stub.map : stub.asm $(BIN)/djasm.exe
	$(BIN)/djasm.exe stub.asm stub.h stub.map

djasm.exe : djasm
	$(STUBIFY) djasm

$(BIN)/go32-v2.exe : go32-v2.c
	gcc -s $(CFLAGS) go32-v2.c -o $(BIN)/go32-v2.exe -ldbg

stub2inc.exe : stub2inc
	$(STUBIFY) stub2inc

stubedit.exe : stubedit $(INC)/stubinfo.h
	$(STUBIFY) stubedit

clean ::
	-rm -f *.exe djasm stub.h stub.map stubedit stubify *.o stub2inc *.d

DEPS := $(wildcard *.d)
ifneq ($(DEPS),)
include $(DEPS)
endif
