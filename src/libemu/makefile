# Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details
LIB=../../lib
BIN=../../bin
CFLAGS=-O2

all :: \
	$(LIB)/libemu.a \
	$(BIN)/emu387.dxe \
	$E

$(LIB)/% : %
	cp $< $@

emu387.o: emu387.cc
	gcc $(CFLAGS) -c $< -o $@

emudummy.o: emudummy.c
	gcc $(CFLAGS) -c $< -o $@

libemu.a : emu387.o npxsetup.o ../ident.c
	gcc -c -DLIB=libemu ../ident.c -o id_emu.o
	ar rs libemu.a emu387.o npxsetup.o id_emu.o

npxsetup.o : ../libc/emu387/npxsetup.c
	gcc $(CFLAGS) -c -DIMBED_EMU387 ../libc/emu387/npxsetup.c -o npxsetup.o

emu387.dxe : emu387.o emudummy.o
	../../bin/dxegen emu387.dxe __emu_entry emu387.o id_emu.o emudummy.o -L../../lib -lgcc -lc

clean ::
	-rm -f emu387.dxe *.o libemu.a
