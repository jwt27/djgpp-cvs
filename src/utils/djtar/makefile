# Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details
TOP = ..

LDFLAGS = -s

SRC += djtar.c
EXTRA_FILES += $(BIN)/djtarx.exe
EXTRA_FILES += $(BIN)/djtart.exe

include $(TOP)/../makefile.prg

$(BIN)/djtar.exe : oread.o unzip.o unlzw.o unlzh.o unpack.o inflate.o \
	crypt.o util.o zmethod.o

$(BIN)/djtarx.exe : $(BIN)/stubify.exe
	stubify -g $(BIN)/djtarx.exe
	stubedit $(BIN)/djtarx.exe runfile=djtar

$(BIN)/djtart.exe : $(BIN)/stubify.exe
	stubify -g $(BIN)/djtart.exe
	stubedit $(BIN)/djtart.exe runfile=djtar
