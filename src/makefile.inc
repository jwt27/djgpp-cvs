# Copyright (C) 2000 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details
# -*- makefile -*-

include $(TOP)/../makefile.def

MAKEFLAGS := --no-print-directory

.SUFFIXES: .o .i .c .cc .s

# Set $(GCC{L}_OPT) here so they are defined before first use
#
ifeq ($(CROSS_BUILD),1)
GCC_OPT := $(shell cat $(TOP)/../gcc.opt)
GCCL_OPT := $(shell cat $(TOP)/../gcc-l.opt)
endif

# Set $(AS) before first use
#
AS := $(shell $(CROSS_GCC) $(GCC_OPT) -print-prog-name=as)
AS := $(subst \,/,$(AS))

# These were suggested by Tim Van Holder <tim.van.holder@pandora.be>
# but since $(CROSS_LD) is set in makefile.def and $(LD) is not used
# anywhere, they seem not very useful at this time.
#
#LD := $(shell $(CROSS_GCC) $(GCC_OPT) -print-prog-name=ld)
#LD := $(subst \,/,$(LD))

# Figure out GAS version for use in *.S and inline asm compile flags
#
GAS_VERSION    := $(subst ., ,$(shell $(AS) --version))
GAS_MAJOR      := $(word 3,$(GAS_VERSION))
GAS_MINOR      := $(word 4,$(GAS_VERSION))
GAS_MINORMINOR := $(word 5,$(GAS_VERSION))

# Defaults for GAS version numbers
ifeq ($(GAS_MAJOR),)
GAS_MAJOR := 0
endif
ifeq ($(GAS_MINOR),)
GAS_MINOR := 0
endif
ifeq ($(GAS_MINORMINOR),)
GAS_MINORMINOR := 0
endif

# Pass defines as compiler/assembler switches
CFLAGS += -DGAS_MAJOR=$(GAS_MAJOR)
CFLAGS += -DGAS_MINOR=$(GAS_MINOR)
CFLAGS += -DGAS_MINORMINOR=$(GAS_MINORMINOR)

ASFLAGS += -DGAS_MAJOR=$(GAS_MAJOR)
ASFLAGS += -DGAS_MINOR=$(GAS_MINOR)
ASFLAGS += -DGAS_MINORMINOR=$(GAS_MINORMINOR)

# Moved setting of $(GCC{L}_OPT) to top of file
#
ifneq ($(CROSS_BUILD),1)
XGCC = $(CROSS_GCC) @$(TOP)/../gcc.opt -I. -I- -isystem $(TOP)/../../include $(CFLAGS)
XLGCC = $(CROSS_GCC) -s @$(TOP)/../gcc-l.opt -I. -I- -isystem $(TOP)/../../include $(CFLAGS)
else
XGCC = $(CROSS_GCC) $(GCC_OPT) -I. -I- -isystem $(TOP)/../../include $(CFLAGS)
XLGCC = $(CROSS_GCC) $(GCCL_OPT) -I. -I- -isystem $(TOP)/../../include $(CFLAGS)
endif

MISC = $(TOP)/../misc.exe

%.o : %.c
	@$(MISC) echo - $(CROSS_GCC) '...' -c $<
	@$(XGCC) -c $<
%.o : %.cc
	@$(MISC) echo - $(CROSS_GCC) '...' -c -fno-exceptions $<
	@$(XGCC) -c -fno-exceptions $<
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
LIBGCCA := $(shell $(CROSS_GCC) $(GCC_OPT) -print-file-name=libgcc.a)
LIBGCCA := $(subst \,/,$(LIBGCCA))
export LIBGCCA
endif

ifeq ($(DJGPP_DJL),)
DJGPP_DJL := $(shell $(CROSS_GCC) $(GCC_OPT) -print-file-name=djgpp.djl)
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
