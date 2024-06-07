# find the suitable cross-assembler
GAS = $(CROSS_PREFIX)as
CROSS_PREFIX := i686-linux-gnu-
ifeq ($(shell $(GAS) --version 2>/dev/null),)
CROSS_PREFIX := x86_64-linux-gnu-
endif
ifeq ($(shell $(GAS) --version 2>/dev/null),)
ifeq ($(shell uname -m),x86_64)
CROSS_PREFIX :=
else
$(error cross-binutils not installed)
endif
endif
XAS = $(CPP) $(XCPPFLAGS) -x assembler-with-cpp $(1) | $(GAS) --32
XCPPFLAGS = -I. $(shell pkg-config --variable=cppflags dj64)
XSTRIP = $(CROSS_PREFIX)strip --strip-debug
XLD = $(CROSS_PREFIX)ld
XLDFLAGS = $(shell pkg-config --static --libs dj64static) -melf_i386 -static
LD = gcc
LDFLAGS = $(shell pkg-config --libs dj64) \
  -Wl,-rpath=/usr/local/i386-pc-dj64/lib64 \
  -Wl,-rpath=/usr/i386-pc-dj64/lib64
DJ64_XLIB = libtmp.so
DJ64_XELF = tmp.elf
DJ64_XOBJS = $(DJ64_XLIB) $(DJ64_XELF)

.INTERMEDIATE: $(DJ64_XOBJS)

$(DJ64_XELF): $(AS_OBJECTS) plt.o
	$(XLD) $^ $(XLDFLAGS) -o $@
	$(XSTRIP) $@

$(DJ64_XLIB): $(OBJECTS)
	$(LD) $^ $(LDFLAGS) -o $@

%.o: %.c
	dj64-gcc $(CFLAGS) -I. -o $@ -c $<
%.o: %.S
	$(call XAS,$<) -o $@
plt.o: plt.inc glob_asm.h
	echo "#include <dj64/plt.S.inc>" | $(call XAS,-) -o $@
thunks_c.o: thunk_calls.h
thunks_p.o: thunk_asms.h plt_asmc.h

# hook in thunk-gen - make sure to not do that before defining `all:` target
TGMK = $(shell pkg-config --variable=makeinc thunk_gen)
ifeq ($(TGMK),)
ifeq ($(filter clean,$(MAKECMDGOALS)),)
$(error thunk_gen not installed)
endif
else
TFLAGS = -a 4 -p 4
include $(TGMK)
endif

clean_dj64:
	$(RM) $(OBJECTS) $(AS_OBJECTS) plt.o plt.inc *.tmp
	$(RM) thunk_calls.h thunk_asms.h plt_asmc.h $(DJ64_XOBJS)
