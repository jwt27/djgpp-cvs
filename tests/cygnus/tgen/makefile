# makefile for test-vector generators
#

TOP = ../..
CC = gcc

CRT0       = $(TOP)/../lib/crt0.o
LIBGCCA   := $(shell $(CC) -print-file-name=libgcc.a)
LIBSTDCXX := $(shell $(CC) -print-file-name=libstdcxx.a)

LDFLAGS = -nostdlib -L$(TOP)/../lib
LIBS    = $(CRT0) $(NEWLIBM) $(LIBSTDCXX) $(LIBGCCA) -lc $(LIBGCCA)

.SUFFIXES:
.SUFFIXES: .exe .o .c .cpp

.c.exe:
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

.o.exe:
	$(CXX) $(LDFLAGS) -o $@ $< $(QFLTLIB) $(LIBS)

.cpp.o:
	$(CXX) -c $< $(CFLAGS) $(CPPFLAGS)

QFLOAT  = ./qfloat
TEST    = ..
MATH    = $(TOP)/../src/libm/math
HEADERS	=	$(QFLOAT)/qfloat.h\
		$(TEST)/test.h\
		genmathv.h

INCDIRS	=  -I. -I$(QFLOAT) -I$(MATH) -I$(TEST) -I$(TOP)/../include

QFLTLIB	= $(QFLOAT)/qfloatx.a

NEWLIBM	= $(TOP)/../lib/libm.a

# -Ic:/testmath/extended# -Ic:/cephes.src/qfloat

CFLAGS	= -nostdinc -O2 $(INCDIRS) -D_USE_LIBM_MATH_H -D_HAVE_STDC \
		 -march=pentium -Wall

ALL =	gacosvec.exe\
	gacoshvec.exe\
	gasinvec.exe\
	gasinhvec.exe\
	gatanvec.exe\
	gatan2vec.exe\
	gatanhvec.exe\
	gcosvec.exe\
	gcoshvec.exe\
	gerfvec.exe\
	gexpvec.exe\
	gfmodvec.exe\
	gfrexpvec.exe\
	ggammavec.exe\
	ghypotvec.exe\
	gjy01vec.exe\
	gjynvec.exe\
	gldexpvec.exe\
	glog1pvec.exe\
	glog10vec.exe\
	glogvec.exe\
	glog2vec.exe\
	gmiscvec.exe\
	gmodfvec.exe\
	gpowvec.exe\
	gsinhvec.exe\
	gsinvec.exe\
	gsqrtvec.exe\
	gtanhvec.exe\
	gtanvec.exe

all: $(ALL)

$(ALL):	 $(QFLTLIB) $(NEWLIBM)
$(ALL:.exe=.o):  $(HEADERS)

$(QFLTLIB):
	$(MAKE) -C $(@D) $(@F) UFLAGS='-Wall' CFLAGS='-Wall -O2'

chkdata.exe:  chkdata.c
	$(CC) -o $@  $(CFLAGS) -L$(TOP)/../lib -L../math $^ -lm

chkvals.exe:	chkvals.c
	$(CC) -o $@  $(CFLAGS) $^ -lm

save:
	-ls *.cpp chkdata.c chkvals.c geteps.c \
		*.h makefile *.sh readme > junklist.___
	-pkzip -u -ex testgen @junklist.___
	-rm junklist.___

clean:
	-cd qfloat; $(MAKE) $@
	-rm -f *.o *.exe
