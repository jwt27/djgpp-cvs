# Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details

TOP = .

EXTRA_FILES += $(INFO)/djasm.info

include ../makefile.inc
# djasm.y would be killed by -Werror -Wall, so let's relax a bit:
XGCC = $(XLGCC)

all :: native \
	$(BIN)/djasm.exe \
	$E

native :: \
	$(HOSTBIN)/djasm.exe \
	$E
	$(NOP)


djasm.c: djasm.y
	$(CROSS_BISON) -o djasm.c djasm.y

$(HOSTBIN)/djasm.exe : djasm.y
	$(BISON) -o djasm-n.c djasm.y
	$(GCC) djasm-n.c -o $@
	$(MISC) rm djasm-n.c

clean ::
	@-$(MISC) rm djasm.c
	@-$(MISC) rm $(HOSTBIN)/djasm.exe
