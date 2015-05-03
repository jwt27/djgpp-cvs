TOP=../../..

SRC += tnmalloc.c
SRC += tmalldbg.c
SRC += evilalg2.c
SRC += evilalgo.c

EXTRA_LIBS := libtnmalloc.a

include $(TOP)/../makefile.inc

prepare :: libtnmalloc.a

TLIB_SRC += cokusmt.c

TLIB_OBJ := $(patsubst %.c,%.o,$(TLIB_SRC))
#
libtnmalloc.a:	$(TLIB_OBJ)
	rm -f $@
	$(CROSS_AR) rc $@ $(TLIB_OBJ)
