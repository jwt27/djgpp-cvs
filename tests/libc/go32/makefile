TOP=..

SRC += ctrlc.c
SRC += fault.c
SRC += fpu.c
SRC += hang.c
SRC += infoblk.c
SRC += raise.c
SRC += signals.c

# This requires Make 3.77 or later.
signals.exe: LDFLAGS += -pg
signals.o: CFLAGS += -pg

include $(TOP)/../makefile.inc
