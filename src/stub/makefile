# Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details

TOP = .

include ../makefile.inc
# djasm.y would be killed by -Werror -Wall, so let's relax a bit:
XGCC = $(XLGCC)

all :: native \
	$(BIN)/djasm.exe \
	$(BIN)/go32-v2.exe \
	$(BIN)/stubedit.exe \
	$(BIN)/stubify.exe \
	$(BIN)/coff2exe.exe \
	$(BIN)/exe2coff.exe \
	$(INC)/stubinfo.h \
	$E

native :: \
	$(HOSTBIN)/djasm.exe \
	$(HOSTBIN)/stubedit.exe \
	$(HOSTBIN)/stubify.exe \
	$(INC)/stubinfo.h \
	$E
	$(NOP)


sbrk16.ah : sbrk16.asm $(HOSTBIN)/djasm.exe
	$(HOSTBIN)/djasm.exe sbrk16.asm sbrk16.ah

$(INC)/stubinfo.h : stub.asm stub.map ./stub2inc.exe
	./stub2inc.exe stub.map stub.asm $(INC)/stubinfo.h

stub.h stub.map : stub.asm $(HOSTBIN)/djasm.exe
	$(HOSTBIN)/djasm.exe stub.asm stub.h stub.map


djasm.c: djasm.y
	$(CROSS_BISON) -o djasm.c djasm.y

# with a native build, gcc is *using* stubify!
$(BIN)/stubify.exe : stubbify.exe
	$(MISC) cp $< $(BIN)/stubify.exe

stubbify.exe: $(C) stubify.o $(L)
	$(LINK)
	$(EXE)

$(BIN)/coff2exe.exe : $(HOSTBIN)/stubify.exe
	$< -g $@
	$(HOSTBIN)/stubedit.exe $@ runfile=stubify

stubify.o: stubify.c stub.h

$(BIN)/exe2coff.exe : exe2coff.o

$(BIN)/go32-v2.exe : $(C) go32-v2.o $(LIB)/libdbg.a $(L)
	$(LINK)
	$(EXE)

$(BIN)/stubedit.exe : $(C) stubedit.o $(L)
	$(LINK)
	$(EXE)


$(HOSTBIN)/djasm.exe : djasm.y
	$(BISON) -o djasm-n.c djasm.y
	$(GCC) djasm-n.c -o $@
	$(MISC) rm djasm-n.c

$(HOSTBIN)/stubify.exe : stubify.c stub.h
	$(GCC) stubify.c -o $@

$(HOSTBIN)/stubedit.exe : stubedit.c $(INC)/stubinfo.h
	$(GCC) stubedit.c -o $@

./stub2inc.exe : stub2inc.c
	$(GCC) stub2inc.c -o $@

clean ::
	@-$(MISC) rm djasm.c stub.map stub.h stub2inc.exe stubbify.exe
	@-$(MISC) rm $(HOSTBIN)/djasm.exe $(HOSTBIN)/stubedit.exe $(HOSTBIN)/stubify.exe
