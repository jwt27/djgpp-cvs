# Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 2000 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details

TOP = ..

EOBJS = \
	djtar.o \
	oread.o \
	epunzip.o \
	untar.o \
	unzip.o \
	unlzw.o \
	unlzh.o \
	unpack.o \
	inflate.o \
	crypt.o \
	util.o \
	zmethod.o \
	unbzip2.o \
	bzlib.o \
	decompress.o \
	huffman.o \
	randtable.o \
	crctable.o \
	$E

EXTRA_FILES += $(BIN)/djtarx.exe
EXTRA_FILES += $(BIN)/djtart.exe

include $(TOP)/../makefile.inc
XGCC = $(XLGCC)

all :: $(BIN)/djtar.exe

$(BIN)/djtar.exe : $(C) $(EOBJS) $(L)
	$(LINK)
	$(EXE)

$(BIN)/djtarx.exe : $(HOSTBIN)/stubify.exe
	$(HOSTBIN)/stubify.exe -g $@
	$(HOSTBIN)/stubedit.exe $@ runfile=djtar

$(BIN)/djtart.exe : $(HOSTBIN)/stubify.exe
	$(HOSTBIN)/stubify.exe -g $@
	$(HOSTBIN)/stubedit.exe $@ runfile=djtar
