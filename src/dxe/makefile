# Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details
BIN=../../bin
CFLAGS=-O2 -nostdinc -I../../include
LIBRARY_PATH := ../../lib
PATH := $(PATH);$(BIN)
export

all :: \
	$(BIN)/dxegen.exe \
	$E

$(BIN)/% : %
	cp $< $@

dxegen.exe : dxegen
	strip dxegen
	stubify dxegen

clean ::
	-rm -f dxegen *.exe *.o
