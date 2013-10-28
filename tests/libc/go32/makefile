TOP=..

SRC += ctrlc.c
SRC += fault.c
SRC += fpu.c
SRC += hang.c
SRC += infoblk.c
SRC += raise.c
SRC += signals.c

signals.exe:  signals.o
	$(filter-out -c,$(GCC)) -pg -o $@  

signals.o: CFLAGS += -pg

include $(TOP)/../makefile.inc
