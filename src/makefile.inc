# Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details
# -*- makefile -*-

include $(TOP)/../makefile.def

MAKEFLAGS := --no-print-directory

.SUFFIXES: .o .i .c .cc .s

ifneq ($(CROSS_BUILD),1)
XGCC = $(CROSS_GCC) @$(TOP)/../gcc.opt -I. -I- -I$(TOP)/../../include $(CFLAGS)
XLGCC = $(CROSS_GCC) -s @$(TOP)/../gcc-l.opt -I. -I- -I$(TOP)/../../include $(CFLAGS)
else
GCC_OPT := $(shell cat $(TOP)/../gcc.opt)
GCCL_OPT := $(shell cat $(TOP)/../gcc-l.opt)
XGCC = $(CROSS_GCC) $(GCC_OPT) -I. -I- -I$(TOP)/../../include $(CFLAGS)
XLGCC = $(CROSS_GCC) $(GCCL_OPT) -I. -I- -I$(TOP)/../../include $(CFLAGS)
endif

MISC = $(TOP)/../misc.exe

%.o : %.c
	@$(MISC) echo - $(CROSS_GCC) '...' -c $<
	@$(XGCC) -c $<
%.o : %.cc
	@$(MISC) echo - $(CROSS_GCC) '...' -c $<
	@$(XGCC) -c $<
%.o : %.S
	@$(MISC) echo - $(CROSS_GCC) '...' -c $<
	@$(XGCC) -c $<
%.o : %.s
	@$(MISC) echo - $(CROSS_GCC) '...' -c $<
	@$(XGCC) -x assembler-with-cpp -c $<

%.i : %.c
	$(XGCC) -c $< -E > $@
%.i : %.cc
	$(XGCC) -c $< -E > $@
%.i : %.S
	$(XGCC) -c $< -E > $@

%.ss : %.c
	$(XGCC) -c $< -S > $@
%.ss : %.cc
	$(XGCC) -c $< -S > $@

%.c : %.y
	bison $*.y -o $*.c

LIB=$(TOP)/../../lib
BIN=$(TOP)/../../bin
INC=$(TOP)/../../include
INFO=$(TOP)/../../info

# Where programs we *run* to build djgpp are
HOSTBIN = $(TOP)/../../hostbin

C = $(LIB)/crt0.o
L = $(LIB)/libc.a

ifeq ($(LIBGCCA),)
LIBGCCA := $(shell $(CROSS_GCC) -print-file-name=libgcc.a)
LIBGCCA := $(subst \,/,$(LIBGCCA))
export LIBGCCA
endif

ifeq ($(DJGPP_DJL),)
DJGPP_DJL := $(shell $(CROSS_GCC) -print-file-name=djgpp.djl)
DJGPP_DJL := $(subst \,/,$(DJGPP_DJL))
export DJGPP_DJL
endif

LINK = $(CROSS_LD) -s $(LDFLAGS) $(filter %.o,$^) $(filter-out %.o,$^) -o $@ $(LIBGCCA) -T $(DJGPP_DJL)
EXE = $(HOSTBIN)/stubify.exe $@

OBJS := $(addsuffix .o, $(basename $(SRC)))
EXES := $(addprefix $(BIN)/,$(addsuffix .exe, $(basename $(SRC))))

NOP = @$(MISC)

.PRECIOUS: %.exe %.out %.err

$(INFO)/%.inf : %.tex
	makeinfo --no-split $<

$(BIN)/%.exe : $C %.o $L
	$(LINK)
	$(EXE)

$(HOSTBIN)/%.exe : %
	$(GCC) -o $@ $^

all :: $(OBJS) $(EXTRA_OBJS) $(EXTRA_FILES)
	$(NOP)

ifneq ($(MAKEFILE_LIB),1)
all :: makefile.oh
makefile.oh : makefile
	@$(MISC) echo - building new response file
	@$(MISC) echo makefile.oh $(addprefix \&/,$(OBJS))
endif

clean ::
	-$(MISC) rm *.o *.d *.i makefile.oh

DEPS := $(wildcard *.d)
ifneq ($(DEPS),)
include $(DEPS)
endif
