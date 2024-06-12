# Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 2000 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details
# -*- makefile -*-

include $(TOP)/../makefile.def

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
#GCC_INC_DIR := $(shell $(CROSS_GCC) -print-file-name=include)
#CFLAGS += -I$(GCC_INC_DIR)

# Pass defines as compiler/assembler switches
ASFLAGS += -DGAS_MAJOR=$(GAS_MAJOR)
ASFLAGS += -DGAS_MINOR=$(GAS_MINOR)
ASFLAGS += -DGAS_MINORMINOR=$(GAS_MINORMINOR)

CPPFLAGS += -DDJ64 -D_IN_DJ64

# Moved setting of $(GCC{L}_OPT) to top of file
#
XGCC = $(CROSS_GCC) @$(TOP)/../gcc.opt -I $(TOP)/../../include $(CFLAGS)
XCPP = $(CROSS_CPP) -I . -I $(TOP)/../../include $(CPPFLAGS)
XASGCC = $(CROSS_ASGCC) -I $(TOP)/../../include -m32 $(ASFLAGS)

MISC = $(TOP)/../misc.exe

%.o : %.c
	$(MISC) echo - $(CROSS_GCC) '...' $(EXTRA_CFLAGS) -c $<
	$(XCPP) -CC -g0 $< | $(XGCC) -xc - -c $(EXTRA_CFLAGS) -o $@
%.o : %.S
	$(MISC) echo - $(CROSS_GCC) '...' -c $<
	$(XASGCC) -o $@ -c $<
%.o : %.s
	$(MISC) echo - $(CROSS_GCC) '...' -c $<
	$(XASGCC) -x assembler-with-cpp -o $@ -c $<

LIB=$(TOP)/../../lib
BIN=$(TOP)/../../bin
INC=$(TOP)/../../include
C = $(LIB)/crt0.o
L = $(LIB)/libc.a

OBJS := $(addsuffix .o, $(basename $(SRC)))
AS_OBJS := $(addsuffix .o, $(basename $(AS_SRC)))
EXES := $(addprefix $(BIN)/,$(addsuffix .exe, $(basename $(SRC))))

NOP = @$(MISC)

.PRECIOUS: %.exe %.out %.err

all :: $(OBJS) $(EXTRA_OBJS) $(EXTRA_FILES) $(AS_OBJS)
	$(NOP)

$(LIB) :
	mkdir $@

ifneq ($(MAKEFILE_LIB),1)
all :: makefile.oh makefile.oho
makefile.oh: makefile
	@$(MISC) echo - building new response file
	@$(MISC) echo makefile.oh $(addprefix \&/,$(OBJS))
makefile.oho: makefile
	@$(MISC) echo - building new response file for as objs
	@$(MISC) echo makefile.oho $(addprefix \&/,$(AS_OBJS))
endif

clean ::
	-$(MISC) rm *.o *.d *.i makefile.oh makefile.oho

DEPS := $(wildcard *.d)
ifneq ($(DEPS),)
include $(DEPS)
endif
