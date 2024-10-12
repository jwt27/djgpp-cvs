# find the suitable cross-assembler
DJ64AS = $(CROSS_PREFIX)as
DJ64ASFLAGS = --32 --defsym _DJ64=1
CROSS_PREFIX := i686-linux-gnu-
ifeq ($(shell $(DJ64AS) --version 2>/dev/null),)
CROSS_PREFIX := x86_64-linux-gnu-
endif
ifeq ($(shell $(DJ64AS) --version 2>/dev/null),)
ifneq ($(filter x86_64 amd64,$(shell uname -m)),)
CROSS_PREFIX :=
ifeq ($(shell $(DJ64AS) --version 2>/dev/null),)
# found nothing, allow dj64-gcc to use defaults
DJ64AS =
DJ64ASFLAGS =
endif
else
$(error cross-binutils not installed)
endif
endif
export DJ64AS
export DJ64ASFLAGS

XSTRIP = $(CROSS_PREFIX)strip --strip-debug
XLD = $(CROSS_PREFIX)ld
LD = $(CC)
# stub version 5
DJ64_XLDFLAGS = -V 5
# freebsd's dlopen() ignores link order and binds to libc the symbols
# defined in libdj64.so. Use static linking as a work-around.
ifeq ($(shell uname -s),FreeBSD)
DJ64STATIC = 1
endif
ifeq ($(DJ64STATIC),0)
$(error DJ64STATIC must be empty, not 0)
endif
ifeq ($(DJ64STATIC),1)
DJLDFLAGS = $(shell pkg-config --libs dj64_s)
DJ64_XLDFLAGS += -f 0x40
else
RP := -Wl,-rpath=/usr/local/i386-pc-dj64/lib64 \
  -Wl,-rpath=/usr/i386-pc-dj64/lib64
ifneq ($(PREFIX),)
RP += -Wl,-rpath=$(PREFIX)/i386-pc-dj64/lib64
endif
# sort removes duplicates
DJLDFLAGS = $(shell pkg-config --libs dj64) $(sort $(RP))
endif
DJ64_XLIB = libtmp.so
ifneq ($(AS_OBJECTS),)
XELF = tmp.elf
endif
DJ64_XOBJS = $(DJ64_XLIB) $(XELF)

.INTERMEDIATE: $(DJ64_XOBJS)

ifneq ($(PDHDR),)
HASH := \#
ifneq ($(shell grep "ASMCFUNC" $(PDHDR) | grep -cv "$(HASH)define"),0)
PLT_O = plt.o
endif
endif
GLOB_ASM = $(wildcard glob_asm.h)

ifneq ($(AS_OBJECTS),)
XLDFLAGS = -melf_i386 -static
ifeq ($(DJ64STATIC),1)
XLDFLAGS += $(shell pkg-config --static --libs dj64static)
DJ64_XLDFLAGS += -f 0x4000
else
XLDFLAGS += $(shell pkg-config --variable=crt0 dj64) \
  --section-start=.note.gnu.property=0x08148000 -section-start=.text=0x08149000
endif
$(XELF): $(AS_OBJECTS) $(PLT_O)
	$(XLD) $^ $(XLDFLAGS) -o $@
	$(XSTRIP) $@
DJ64_XLDFLAGS += -l $(XELF)
else
ifeq ($(DJ64STATIC),1)
DJ64_XLDFLAGS += -l $(shell pkg-config --variable=crt0 dj64_s) -f 0x4000
else
DJ64_XLDFLAGS += -f 0x80
endif
endif

$(DJ64_XLIB): $(OBJECTS)
	$(LD) $^ $(DJLDFLAGS) -o $@

%.o: %.c
	dj64-gcc $(CFLAGS) -I. -o $@ -c $<
%.o: %.S
	dj64-gcc -o $@ -c $<
plt.o: plt.inc $(GLOB_ASM)
	echo "#include <dj64/plt.S.inc>" | dj64-gcc -I. -o $@ -c -
thunks_c.o: thunk_calls.h
thunks_p.o: thunk_asms.h plt_asmc.h

ifneq ($(PDHDR),)
ifneq ($(GLOB_ASM),)
$(OBJECTS): glob_asmdefs.h
endif
# hook in thunk-gen - make sure to not do that before defining `all:` target
TGMK = $(shell pkg-config --variable=makeinc thunk_gen)
ifeq ($(TGMK),)
ifeq ($(filter clean,$(MAKECMDGOALS)),)
$(error thunk_gen not installed)
endif
else
ifeq ($(filter clean,$(MAKECMDGOALS)),)
$(shell pkg-config --atleast-version=1.2 thunk_gen)
ifneq ($(.SHELLSTATUS),0)
$(error thunk_gen is too old, 1.2 is needed)
endif
endif
TFLAGS = -a 4 -p 4
include $(TGMK)
endif
endif

clean_dj64:
	$(RM) $(OBJECTS) $(AS_OBJECTS) plt.o plt.inc *.tmp
	$(RM) thunk_calls.h thunk_asms.h plt_asmc.h glob_asmdefs.h
	$(RM) $(DJ64_XOBJS)
