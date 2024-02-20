TOP = ..

include $(TOP)/../makefile.inc

PD = parsers
MP = $(PD)/mkproto.sh
GA = $(PD)/gen_asms.sh
PC = $(PD)/parse_decls.sh
TG = $(shell pkg-config --variable=binary thunk_gen)
ifeq ($(TG),)
ifeq ($(filter clean,$(MAKECMDGOALS)),)
$(error thunk_gen not installed)
endif
endif
TGS = $(shell pkg-config --variable=tgscript thunk_gen)
TGM4 = $(shell pkg-config --variable=m4script thunk_gen)
TFLAGS = -a 4 -p 4
LIBN = libdj64.so.0.1
SOURCES = thunks.c thunks_a.c thunks_c.c thunks_p.c dosobj.c smalloc.c
OBJECTS = $(SOURCES:.c=.o)

thunk_syms.o: thunk_syms.c asm_incsn.h
	$(XCPP) -CC -g0 $< | $(XASGCC) -xc - -c $(EXTRA_CFLAGS) -o $@
	$(CROSS_STRIP) --strip-debug $@

libc_tmp.a : $(TOP)/makefile.rfo thunk_syms.o
	@-$(MISC) rm $@
	$(CROSS_AR) q $@ $(addprefix ../,$(file < $<)) thunk_syms.o
	$(CROSS_AR) s $@

asm_incs.h:
	$(GA) 0 $(TOP)/../../include >$@

asm_incsn.h:
	$(GA) 1 $(TOP)/../../include >$@

plt_asmc.h: thunk_asms.tmp
	$(PC) 4 $< >$@

asym_incs.h:
	$(GA) 2 $(TOP)/../../include >$@

thunk_calls.tmp thunk_asms.tmp thunk_incs.h plt.inc &: \
    libc_tmp.a
	$(MP) $< $(TOP)/../../include thunk_asms.tmp \
		thunk_calls.tmp plt.inc thunk_incs.h

thunk_calls.h: thunk_calls.tmp
	$(TG) $(TFLAGS) <$< >$@

thunk_asms.h: thunk_asms.tmp
	$(TG) $(TFLAGS) 1 <$< | $(TGS) $(TGM4) >$@_ \
		|| ($(RM) $@_ ; false)
	$(TG) $(TFLAGS) 2 <$< >$@__ \
		|| ($(RM) $@__ ; false)
	cat $@_ $@__ >$@
	rm -f $@_ $@__

thunks.o: thunk_calls.h thunk_asms.h thunk_incs.h asym_incs.h
thunks_a.o: asm_incs.h plt_asmc.h asym_incs.h
thunks_c.o: thunk_incs.h thunk_calls.h
thunks_p.o: thunk_incs.h thunk_asms.h

plt.o : plt.S plt.inc

$(LIB)/$(LIBN) : $(TOP)/makefile.rf thunk_calls.h thunk_asms.h $(OBJECTS)
	$(CROSS_GCC) -shared -Wl,-soname=$(LIBN) -Wl,-Bsymbolic -o $@ \
  $(addprefix ../,$(file < $<)) $(OBJECTS)
	@echo "Have `nm -u $@ | grep "U " | wc -l` undefined symbols"

$(LIB)/libc.a : libc_tmp.a plt.o
	cp -f $< libctmp.a
	$(CROSS_AR) q libctmp.a plt.o
	$(CROSS_AR) s libctmp.a
	mv -f libctmp.a $@

$(LIB)/libdj64.so: $(LIB)/$(LIBN)
	ln -sf $(LIBN) $@

final: $(LIB)/libc.a $(LIB)/libdj64.so

clean::
	$(RM) rm *.a *.tmp *.inc asm_*.h thunk_*.h plt_*.h asym_incs.h
	$(RM) $(LIB)/libc.a $(LIB)/libdj64.so*
