# Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details
# Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details
TOP=.

LDFLAGS = -s

SRC += bin2h.c
SRC += dtou.c
SRC += echo.c
SRC += gxx.c
SRC += merge.c
SRC += redir.c
SRC += split.c
SRC += update.c
SRC += utod.c

EXTRA_FILES += $(INFO)/utils.inf
EXTRA_FILES += $(BIN)/rem.com
EXTRA_FILES += phony_texi2ps
EXTRA_FILES += phony_djtar

include $(TOP)/../makefile.prg

$(BIN)/rem.com : rem.asm
	djasm $^ $@

.PHONY: phony_texi2ps
phony_texi2ps:
	@$(MAKE) --no-print-directory -C texi2ps

.PHONY: phony_djtar
phony_djtar:
	@$(MAKE) --no-print-directory -C djtar
