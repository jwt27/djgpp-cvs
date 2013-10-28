# Copyright (C) 2013 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details

include $(TOP)/../makefile.def

.SUFFIXES: .o .i .c .cc .s

# Find GCC own include directory and add it to CFLAGS
GCC_INC_DIR := $(shell $(CROSS_GCC) -print-file-name=include)
CFLAGS += -I$(GCC_INC_DIR)

GCC = gcc @$(TOP)/../gcc.opt -I. -I$(TOP)/../../include $(CFLAGS) -c $<
GPP = gpp @$(TOP)/../gpp.opt -I. -I$(TOP)/../../include $(CFLAGS) -c $<

%.o : %.c
	$(GCC)
%.o : %.cc
	$(GPP)
%.o : %.S
	$(GCC)

%.i : %.c
	$(GCC) -E > $@
%.i : %.cc
	$(GPP) -E > $@
%.i : %.S
	$(GCC) -E > $@

%.ss : %.c
	$(GCC) -S > $@
%.ss : %.cc
	$(GPP) -S > $@

LIB = ${TOP}/../../lib
BIN = ${TOP}/../../bin

C = ${LIB}/crt0.o
L = ${LIB}/libc.a

EXES := $(addsuffix .exe, $(basename $(SRC)))
ERRS := $(addsuffix .err, $(basename $(SRC)))

ifeq ($(LIBGCCA),)
LIBGCCA := $(shell $(CROSS_GCC) $(GCC_OPT) -print-file-name=libgcc.a)
LIBGCCA := $(subst \,/,$(LIBGCCA))
export LIBGCCA
endif

ifeq ($(DJGPP_DJL),)
DJGPP_DJL := $(shell $(CROSS_GCC) $(GCC_OPT) -print-file-name=djgpp-x.djl)
ifeq ($(DJGPP_DJL),djgpp-x.djl)
DJGPP_DJL := $(shell $(CROSS_GCC) $(GCC_OPT) -print-file-name=djgpp.djl)
endif
DJGPP_DJL := $(subst \,/,$(DJGPP_DJL))
export DJGPP_DJL
endif

LINK = ld $(LDFLAGS) $^ -o $@ $(LIBGCCA) -T $(DJGPP_DJL)
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

all :: config $(EXES)
	@rem.com

tests :: $(ERRS)

testclean ::
	-rm -f *.out *.err

clean :: testclean
	-rm -f *.exe *.o *.d *.i

config:	$(TOP)/../gcc.opt $(TOP)/../gpp.opt

$(TOP)/../gcc.opt: $(TOP)/../makefile.inc
	@$(TOP)/../../src/misc.exe echo - "-MD" >$(TOP)/../gcc.opt
	@$(TOP)/../../src/misc.exe echo - "-O3" >>$(TOP)/../gcc.opt
	@$(TOP)/../../src/misc.exe echo - "-g" >>$(TOP)/../gcc.opt
	@$(TOP)/../../src/misc.exe echo - "-march=i386" >>$(TOP)/../gcc.opt
	@$(TOP)/../../src/misc.exe echo - "-Wall" >>$(TOP)/../gcc.opt
	@$(TOP)/../../src/misc.exe echo - "-Wbad-function-cast" >>$(TOP)/../gcc.opt
	@$(TOP)/../../src/misc.exe echo - "-Wcast-qual" >>$(TOP)/../gcc.opt
	@$(TOP)/../../src/misc.exe echo - "-Werror" >>$(TOP)/../gcc.opt
	@$(TOP)/../../src/misc.exe echo - "-Wpointer-arith" >>$(TOP)/../gcc.opt
	@$(TOP)/../../src/misc.exe echo - "-Wshadow" >>$(TOP)/../gcc.opt
	@$(TOP)/../../src/misc.exe echo - "-Wstrict-prototypes" >>$(TOP)/../gcc.opt
	@$(TOP)/../../src/misc.exe echo - "-Wwrite-strings" >>$(TOP)/../gcc.opt
	@$(TOP)/../../src/misc.exe echo - "-Wundef" >>$(TOP)/../gcc.opt
	@$(TOP)/../../src/misc.exe echo - "-Wcast-align" >>$(TOP)/../gcc.opt
	@$(TOP)/../../src/misc.exe echo - "-Wsign-compare" >>$(TOP)/../gcc.opt
	@$(TOP)/../../src/misc.exe echo - "-nostdinc" >>$(TOP)/../gcc.opt
	@$(TOP)/../../src/misc.exe echo - "$(IQUOTE)" >>$(TOP)/../gcc.opt

$(TOP)/../gpp.opt: $(TOP)/../makefile.inc
	@$(TOP)/../../src/misc.exe echo - "-MD" >$@
	@$(TOP)/../../src/misc.exe echo - "-O3" >>$@
	@$(TOP)/../../src/misc.exe echo - "-g" >>$@
	@$(TOP)/../../src/misc.exe echo - "-march=i386" >>$@
	@$(TOP)/../../src/misc.exe echo - "-Wall" >>$@
	@$(TOP)/../../src/misc.exe echo - "-Wcast-qual" >>$@
	@$(TOP)/../../src/misc.exe echo - "-Werror" >>$@
	@$(TOP)/../../src/misc.exe echo - "-Wpointer-arith" >>$@
	@$(TOP)/../../src/misc.exe echo - "-Wshadow" >>$@
	@$(TOP)/../../src/misc.exe echo - "-Wwrite-strings" >>$@
	@$(TOP)/../../src/misc.exe echo - "-Wundef" >>$@
	@$(TOP)/../../src/misc.exe echo - "-Wcast-align" >>$@
	@$(TOP)/../../src/misc.exe echo - "-Wsign-compare" >>$@
	@$(TOP)/../../src/misc.exe echo - "-nostdinc" >>$@
	@$(TOP)/../../src/misc.exe echo - "$(IQUOTE)" >>$@

DEPS := $(wildcard *.d)
ifneq ($(DEPS),)
include $(DEPS)
endif
