
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

LINK = ld $(LDFLAGS) $(filter %.o,$^) $(filter-out %.o,$^) -o $@ ${LIB}/libgcc.a -T ${TOP}/../../lib/djgpp.djl
EXE = ${TOP}/../../bin/stubify.exe $@

LIB=$(TOP)/../../lib
BIN=$(TOP)/../../bin
INC=$(TOP)/../../include
INFO=$(TOP)/../../info

C = $(LIB)/crt0.o
L = $(LIB)/libc.a

OBJS := $(addsuffix .o, $(basename $(SRC)))
EXES := $(addprefix $(BIN)/,$(addsuffix .exe, $(basename $(SRC))))

.PRECIOUS: %.exe %.out %.err

$(INFO)/%.inf : %.tex
	makeinfo --no-split $<

$(BIN)/%.exe : $C %.o $L
	$(LINK)
	$(EXE)

all :: $(OBJS) $(EXTRA_OBJS) makefile.oh
	@rem.com

makefile.oh : makefile
	@echo.exe -o makefile.oh $(addprefix &/,$(OBJS))

clean ::
	-rm -f *.o *.d *.i makefile.oh

DEPS := $(wildcard *.d)
ifneq ($(DEPS),)
include $(DEPS)
endif
