
.SUFFIXES: .o .i .c .cc .s

GCC = gcc @$(TOP)/../gcc.opt -I. -I- -I$(TOP)/../../include $(CFLAGS) -c $<

%.o : %.c
	$(GCC)
%.o : %.cc
	$(GCC)
%.o : %.S
	$(GCC)

%.i : %.c
	$(GCC) -E > $@
%.i : %.cc
	$(GCC) -E > $@
%.i : %.S
	$(GCC) -E > $@

%.ss : %.c
	$(GCC) -S > $@
%.ss : %.cc
	$(GCC) -S > $@

LIB = ${TOP}/../../lib
BIN = ${TOP}/../../bin

C = ${LIB}/crt0.o
L = ${LIB}/libc.a

EXES := $(addsuffix .exe, $(basename $(SRC)))
ERRS := $(addsuffix .err, $(basename $(SRC)))

LINK = ld $(LDFLAGS) $^ -o $@ -T ${TOP}/../../lib/djgpp.lnk
EXE = @${TOP}/../../bin/stubify.exe $@

.PRECIOUS: %.exe %.out %.err

%.exe : $C %.o ${EXTRA_LIBS} $L
	$(LINK)
	$(EXE)

%.out : %.exe
	-$^ > $@

%.err : %.exp %.out
	-diff $^ > $@
	cat $@

all :: $(EXES)
	@rem.com

tests :: $(ERRS)

testclean ::
	-rm -f *.out *.err

clean :: testclean
	-rm -f *.exe *.o *.d *.i

DEPS := $(wildcard *.d)
ifneq ($(DEPS),)
include $(DEPS)
endif
