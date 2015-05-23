# Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details
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
GPP_OPT := $(shell cat $(TOP)/../gpp.opt)
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
GAS_VERSION_STR:= $(shell $(AS) --version | sed 1q | sed -e 's:^.*[ 	]\([0-9]\.[0-9][\.0-9]*\).*$$:\1:')
GAS_VERSION    := $(subst ., ,$(GAS_VERSION_STR))
GAS_MAJOR      := $(word 1,$(GAS_VERSION))
GAS_MINOR      := $(word 2,$(GAS_VERSION))
GAS_MINORMINOR := $(word 3,$(GAS_VERSION))

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

# Find GCC own include directory and add it to CFLAGS
GCC_INC_DIR := $(shell $(CROSS_GCC) -print-file-name=include)
CFLAGS += -I$(GCC_INC_DIR)

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
XGCC = $(CROSS_GCC) @$(TOP)/../gcc.opt -isystem $(TOP)/../../include $(CFLAGS)
XGPP = $(CROSS_GCC) @$(TOP)/../gpp.opt -isystem $(TOP)/../../include $(CFLAGS)
XLGCC = $(CROSS_GCC) -s @$(TOP)/../gcc-l.opt -isystem $(TOP)/../../include $(CFLAGS)
else
XGCC = $(CROSS_GCC) $(GCC_OPT) -isystem $(TOP)/../../include $(CFLAGS)
XGPP = $(CROSS_GCC) $(GPP_OPT) -isystem $(TOP)/../../include $(CFLAGS)
XLGCC = $(CROSS_GCC) $(GCCL_OPT) -isystem $(TOP)/../../include $(CFLAGS)
endif

# Find the necessary gcc options for building the profiling
# support code (src/libc/crt0/mcount.c). We strip out profiling options
# from the standard gcc options file, to avoid a chicken-and-egg problem
# in a profiling version of libc (the profiling support code calling
# profiling support code).
#
XNOPGGCC = $(CROSS_GCC) $(shell sed -f $(TOP)/../noprof.sed $(TOP)/../gcc.opt) -isystem $(TOP)/../../include $(CFLAGS)
XNOPGGPP = $(CROSS_GCC) $(shell sed -f $(TOP)/../noprof.sed $(TOP)/../gpp.opt) -isystem $(TOP)/../../include $(CFLAGS)

MISC = $(TOP)/../misc.exe

%.o : %.c
	@$(MISC) echo - $(CROSS_GCC) '...' $(EXTRA_CFLAGS) -c $<
	@$(XGCC) -c $(EXTRA_CFLAGS) $<
%.o : %.cc
	@$(MISC) echo - $(CROSS_GPP) '...' $(EXTRA_CXXFLAGS) -c -fno-exceptions $<
	@$(XGPP) $(EXTRA_CXXFLAGS) -c -fno-exceptions $<
%.o : %.S
	@$(MISC) echo - $(CROSS_GCC) '...' -c $<
	@$(XGCC) -c $<
%.o : %.s
	@$(MISC) echo - $(CROSS_GCC) '...' -c $<
	@$(XGCC) -x assembler-with-cpp -c $<

%.i : %.c
	$(XGCC) -c $< -E > $@
%.i : %.cc
	$(XGPP) -c $< -E > $@
%.i : %.S
	$(XGCC) -c $< -E > $@

%.ss : %.c
	$(XGCC) -c $< -S > $@
%.ss : %.cc
	$(XGPP) -c $< -S > $@

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
DJGPP_DJL = $(LIB)/djgpp.djl
#export DJGPP_DJL
endif

LINK = $(CROSS_LD) -s $(LDFLAGS) $(filter %.o,$^) $(filter-out %.o,$^) -o $@ $(LIBGCCA) -T $(DJGPP_DJL)
EXE = $(HOSTBIN)/stubify.exe $@

OBJS := $(addsuffix .o, $(basename $(SRC)))
EXES := $(addprefix $(BIN)/,$(addsuffix .exe, $(basename $(SRC))))

NOP = @$(MISC)

.PRECIOUS: %.exe %.out %.err

$(INFO)/%.info : %.tex
	makeinfo --no-split $<

$(INFO)/%.info : %.txi
	makeinfo --no-split $<

$(BIN)/%.exe : $C %.o $L
	$(LINK)
	$(EXE)

$(HOSTBIN)/%.exe : %
	$(GCC) -o $@ $^

all :: $(TOP)/../gpp.opt $(OBJS) $(EXTRA_OBJS) $(EXTRA_FILES)
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
