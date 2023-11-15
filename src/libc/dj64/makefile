TOP = ..

include $(TOP)/../makefile.inc

PD = parsers
TG = thunk_gen/thunk_gen
MP = $(PD)/mkproto.sh
GA = $(PD)/gen_asms.sh
PC = $(PD)/parse_decls.sh
TFLAGS = -a 4 -p 4

thunk_syms.o: thunk_syms.c asm_incsn.h
	$(XGCC) -m32 -c $< $(EXTRA_CFLAGS) -o $@
	$(CROSS_STRIP) --strip-debug $@

libc_tmp.a : $(TOP)/makefile.rfo thunk_syms.o
	@-$(MISC) rm $@
	$(CROSS_AR) q $@ $(addprefix ../,$(file < $<)) thunk_syms.o
	$(CROSS_AR) s $@

$(TG):
	$(MAKE) -C $(PD)

asm_incs.h:
	$(GA) 0 $(TOP)/../../include >$@

asm_incsn.h:
	$(GA) 1 $(TOP)/../../include >$@

plt_asms.inc:
	$(GA) 2 $(TOP)/../../include >$@

plt_asmc.inc: thunk_asms.tmp
	$(PC) 4 $< >$@

asym_incs.h:
	$(GA) 3 $(TOP)/../../include >$@

thunk_calls.tmp thunk_asms.tmp thunk_incs.h plt.inc &: \
    libc_tmp.a $(TG)
	$(MP) $< $(TOP)/../../include thunk_asms.tmp \
		thunk_calls.tmp plt.inc thunk_incs.h

thunk_calls.h: thunk_calls.tmp
	$(TG) $(TFLAGS) <$< >$@

thunk_asms.h: thunk_asms.tmp
	$(TG) $(TFLAGS) 1 <$< | sort | uniq | autom4te -l m4sugar $(PD)/thunks.m4 - >$@_ \
		|| ($(RM) $@_ ; false)
	$(TG) $(TFLAGS) 2 <$< >$@__ \
		|| ($(RM) $@__ ; false)
	cat $@_ $@__ >$@
	rm -f $@_ $@__

thunks.o: thunk_calls.h thunk_asms.h thunk_incs.h asym_incs.h
thunks_a.o: asm_incs.h

plt.o : plt.S plt.inc plt_asms.inc plt_asmc.inc
	$(XGCC) -m32 $< -c -o $@

$(LIB)/libdj64.so : $(TOP)/makefile.rf thunk_calls.h thunk_asms.h thunks.o \
  thunks_a.o
	$(CROSS_GCC) -shared -Wl,-Bsymbolic -o $@ \
  $(addprefix ../,$(file < $<)) thunks.o thunks_a.o
	@echo "Have `nm -u $@ | grep "U " | wc -l` undefined symbols"

$(LIB)/libc.a : libc_tmp.a plt.o
	cp -f $< libctmp.a
	$(CROSS_AR) q libctmp.a plt.o
	$(CROSS_AR) s libctmp.a
	mv -f libctmp.a $@

final: $(LIB)/libc.a $(LIB)/libdj64.so

clean ::
	$(MAKE) -C thunk_gen clean
	@$(MISC) rm *.a *.tmp *.inc asm_*.h thunk_*.h
