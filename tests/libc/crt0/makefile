TOP=..

SRC += args.c
SRC += brk.c
SRC += envtest.c
SRC += frstfile.c
SRC += globtest.c
SRC += loadaddr.c
SRC += minsize.c
SRC += ms.c
SRC += mspn2.c
SRC += multispn.c
SRC += nop.c
SRC += pagetest.c
SRC += sections.c
SRC += stub1.c
SRC += stub2.c

include $(TOP)/../makefile.inc

stub2.exe : $(BIN)/stubify.exe $(BIN)/stubedit.exe
	$(BIN)/stubify -g stub2.exe
	$(BIN)/stubedit stub2.exe runfile=stub1
