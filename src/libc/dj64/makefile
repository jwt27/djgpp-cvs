TOP = ..

include $(TOP)/../makefile.inc
CPPFLAGS += -I ../../djdev64/include

P = parsers
MP = $(P)/mkproto.sh
GA = $(P)/gen_asms.sh
LIBN = libdj64.so.0.2
SOURCES = thunks.c thunks_a.c thunks_c.c thunks_p.c dosobj.c smalloc.c
OBJECTS = $(SOURCES:.c=.o)

thunk_syms.o: thunk_syms.c asm_incsn.h
	$(XCPP) -CC -g0 $< | $(XASGCC) -xc - -c $(EXTRA_CFLAGS) -o $@
	$(CROSS_ASSTRIP) --strip-debug $@

ALL_AS_OBJS = $(addprefix ../,$(file < $(TOP)/makefile.rfo))
libc_tmp.a : $(TOP)/makefile.rfo thunk_syms.o $(ALL_AS_OBJS)
	@-$(MISC) rm $@
	$(CROSS_ASAR) q $@ $(ALL_AS_OBJS) thunk_syms.o
	$(CROSS_ASAR) s $@

asm_incs.h:
	$(GA) 0 $(TOP)/../../include >$@

asm_incsn.h:
	$(GA) 1 $(TOP)/../../include >$@

asym_incs.h:
	$(GA) 2 $(TOP)/../../include >$@

thunk_calls.tmp thunk_asms.tmp thunk_incs.h plt.inc &: \
    libc_tmp.a
	$(MP) $< $(TOP)/../../include thunk_asms.tmp \
		thunk_calls.tmp plt.inc thunk_incs.h

TGMK = $(shell pkg-config --variable=makeinc thunk_gen)
ifeq ($(TGMK),)
ifeq ($(filter clean,$(MAKECMDGOALS)),)
$(error thunk_gen not installed)
endif
endif
TFLAGS = -a 4 -p 4
include $(TGMK)

thunks.o: thunk_calls.h thunk_asms.h thunk_incs.h asym_incs.h
thunks_a.o: asm_incs.h plt_asmc.h asym_incs.h
thunks_c.o: thunk_incs.h thunk_calls.h
thunks_p.o: thunk_incs.h thunk_asms.h

plt.o : plt.S plt.inc

ALL_OBJS = $(addprefix ../,$(file < $(TOP)/makefile.rf))
$(LIB)/$(LIBN) : $(TOP)/makefile.rf $(ALL_OBJS) \
  thunk_calls.h thunk_asms.h $(OBJECTS)
	$(CROSS_GCC) -shared -Wl,-soname=$(LIBN) -Wl,-Bsymbolic -o $@ \
  $(ALL_OBJS) $(OBJECTS)
	@echo "Have `nm -u $@ | grep "U " | wc -l` undefined symbols"

$(LIB)/libc.a : libc_tmp.a plt.o | $(LIB)
	cp -f $< libctmp.a
	$(CROSS_ASAR) q libctmp.a plt.o
	$(CROSS_ASAR) s libctmp.a
	mv -f libctmp.a $@

$(LIB)/libdj64.so: $(LIB)/$(LIBN) | $(LIB)
	ln -sf $(LIBN) $@

final: $(LIB)/libc.a $(LIB)/libdj64.so | $(LIB)

clean::
	$(RM) rm *.a *.tmp *.inc asm_*.h thunk_*.h plt_*.h asym_incs.h
	$(RM) $(LIB)/libc.a $(LIB)/libdj64.so*
