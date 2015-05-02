# Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details
TOP=.

LDFLAGS = -s

SRC += bin2h.c
SRC += djecho.c
SRC += djmerge.c
SRC += djsplit.c
SRC += dtou.c
SRC += getconf.c
SRC += gxx.c
SRC += redir.c
SRC += update.c
SRC += utod.c

EXTRA_FILES += $(INFO)/utils.info
EXTRA_FILES += $(BIN)/rem.com
EXTRA_FILES += phony_texi2ps
EXTRA_FILES += phony_djtar

include $(TOP)/../makefile.prg

native :: $(HOSTBIN)/bin2h.exe
	$(NOP)

$(HOSTBIN)/bin2h.exe : bin2h.c
	$(GCC) bin2h.c -o $@

$(BIN)/rem.com : rem.asm
	$(HOSTBIN)/djasm.exe $^ $@

.PHONY: phony_texi2ps
phony_texi2ps:
	@$(MAKE) --no-print-directory -C texi2ps

.PHONY: phony_djtar
phony_djtar:
	@$(MAKE) --no-print-directory -C djtar

clean ::
	@$(MAKE) --no-print-directory -C texi2ps $@
	@$(MAKE) --no-print-directory -C djtar $@
