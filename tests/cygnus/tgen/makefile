# makefile for test-vector generators
#

TOP = ../..
CC = gcc
LDFLAGS = $(NEWLIBM)

.SUFFIXES:
.SUFFIXES: .exe .o .c .cpp

.c.exe:
	$(CC) -o $@ $^  $(CFLAGS) $(CPPFLAGS)

.o.exe:
	$(CC) -L$(TOP)/../lib -o $@ $< $(QFLTLIB) $(LDFLAGS)

.cpp.o:
	$(CC) -c $< $(CFLAGS) $(CPPFLAGS)

QFLOAT  = ./qfloat
TEST    = ..
MATH    = $(TOP)/../src/libm/math
HEADERS	=	$(QFLOAT)/qfloat.h\
		$(TEST)/test.h\
		genmathv.h

INCDIRS	=  -I. -I$(QFLOAT) -I$(MATH) -I$(TEST) -I$(DJDIR)/include

QFLTLIB	= $(QFLOAT)/qfloatx.a

NEWLIBM	= $(MATH)/libm.a

# -Ic:/testmath/extended# -Ic:/cephes.src/qfloat

CFLAGS	= -O2 $(INCDIRS) -D_USE_LIBM_MATH_H -D_HAVE_STDC \
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
