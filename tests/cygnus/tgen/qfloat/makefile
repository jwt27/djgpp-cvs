# make file for 100-digit libraries:
#
#	qfloata.a	Strictly-rounded version
#	qfloatb.a	Non-Strictly-rounded Fairly-Fast version
#	qfloatf.a	Non-Strictly-rounded Fastest version

ANSIC		= -DANSIC
CC		= gcc
CFLAGS		= $(UFLAGS) $(OPTIMIZ)
LIBNAME_A	= qfloata.a
LIBNAME_B	= qfloatb.a
LIBNAME_F	= qfloatf.a
LIBNAME_X	= qfloatx.a
NO_OPTIMIZ	= -O0
OPTIMIZ		= -O2
STICKY		= -DSTICKY
UFLAGS		= -Wall -march=pentium
SETNQ		= #

.c.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

# Objects Common to all libraries
#
OBJECTS_C = mtherr.o qacosh.o  qasin.o qasinh.o qatanh.o qatn.o\
	    qcbrt.o  qconst.o  qcos.o  qcosh.o	qerf.o	qerfc.o\
	    qexp.o   qexp10.o qfloor.o\
	    qflti.o  qgamma.o  qjn.o   qlog.o	qlog10.o qpow.o\
	    qrand.o  qremain.o qsin.o  qsinh.o	qtan.o	 qtanh.o qyn.o

#	Objects That Produce Strictly Rounded Version
#		Slow Speed
#
OBJECTS_A = $(OBJECTS_C) qsqrta.o qflta.o

#	Objects That Produce Faster Non-Strictly Rounded Version
#		Medium Speed
#
OBJECTS_B = $(OBJECTS_C) qsqrt.o qfltbi.o #qfltb386.o

#	Objects That Produce Fastest Non-Strictly Rounded Version
#
OBJS_ASM  = subml.o mulrl.o divnl.o
OBJ_SQRT  = qsqrt.o
OBJECTS_F = $(OBJECTS_C) $(OBJS_ASM) $(OBJ_SQRT)

A_INCLUDES   =	qhead.asm
C_INCLUDES   =	qhead.h mconf.h	protos.h qprotos.h
HEADERS	     =	$(A_INCLUDES) $(C_INCLUDES)

Test:	$(LIBNAME_X)

Medium:	$(LIBNAME_B)
	@rem.com

Slow:	$(LIBNAME_A)
	@rem.com

# Medium-fast non-strictly rounded version - Medium Speed
$(LIBNAME_X): $(OBJECTS_B)
	ar -rs $@ $?

# Strictly rounded version, all C language  - Slow Speed
$(LIBNAME_A): $(OBJECTS_A)
	@-cp -f $@ $(LIBNAME_A:.a=.old)  > nul
	ar -rs $@ $?

# Medium-fast non-strictly rounded version - Medium Speed
$(LIBNAME_B): $(OBJECTS_B)
	@-cp -f $@ $(LIBNAME_B:.a=.old)   > nul
	ar -rs $@ $?

# Fastest Non-Strictly Rounded Version
$(LIBNAME_F): $(OBJECTS_F)
	@-cp -f $@ $(LIBNAME_F:.a=.old)   > nul
	ar -rs $@ $?

all: $(LIBNAME_A) $(LIBNAME_B) $(LIBNAME_F)

protos:
	zgmake qflt qlog qexp qpow qtanh qfloor mtherr qflta qsqrta

xcg:
	cat $(OBJECTS_A:.o=.c) > $(TMP)all.c
	xcg $(TMP)all.c | awk -f $(HOME)/extlist.awk | sed s/^........// |\
		sed "s/ [.[]..*]$$//" | cat -n > qfloat.xcg
	del $(TMP)all.c

#qcalc.o: qhead.h qcalc.h
#	$(CC) $(CFLAGS) qcalc.c

mtherr.o:	mtherr.c	$(C_INCLUDES)
qacosh.o:	qacosh.c	$(C_INCLUDES)
qasin.o:	qasin.c 	$(C_INCLUDES)
qasinh.o:	qasinh.c	$(C_INCLUDES)
qatanh.o:	qatanh.c	$(C_INCLUDES)
qatn.o:		qatn.c		$(C_INCLUDES)
qcbrt.o:	qcbrt.c 	$(C_INCLUDES)
qconst.o:	qconst.c	$(C_INCLUDES)
qcos.o:		qcos.c		$(C_INCLUDES)
qcosh.o:	qcosh.c 	$(C_INCLUDES)
qerf.o:		qerf.c		$(C_INCLUDES)
qerfc.o:	qerfc.c 	$(C_INCLUDES)
qexp.o:		qexp.c		$(C_INCLUDES)
qexp10.o:	qexp10.c	$(C_INCLUDES)
	$(CC) -c $(CPPFLAGS) $(UFLAGS) $<
qfloor.o:	qfloor.c	$(C_INCLUDES)
qflta.o:	qflta.c 	$(C_INCLUDES)
qfltbi.o:	qfltbi.c	$(C_INCLUDES)
qflti.o:	qflti.c 	$(C_INCLUDES)	qtens.h
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $(SETNQ) $<
qgamma.o:	qgamma.c	$(C_INCLUDES)
qjn.o:		qjn.c		$(C_INCLUDES)
qlog.o:		qlog.c		$(C_INCLUDES)
	$(CC) -c $(CPPFLAGS) $(UFLAGS) $<
qlog10.o:	qlog10.c	$(C_INCLUDES)
qpow.o:		qpow.c		$(C_INCLUDES)
	$(CC) -c $(CPPFLAGS) $(UFLAGS) $<
qrand.o:	qrand.c 	$(C_INCLUDES)
qremain.o:	qremain.c	$(C_INCLUDES)
qsin.o:		qsin.c		$(C_INCLUDES)
qsinh.o:	qsinh.c 	$(C_INCLUDES)
qsqrt.o:	qsqrt.c 	$(C_INCLUDES)
qsqrta.o:	qsqrta.c	$(C_INCLUDES)
qtan.o:		qtan.c		$(C_INCLUDES)
qtanh.o:	qtanh.c 	$(C_INCLUDES)
qyn.o:		qyn.c		$(C_INCLUDES)

# i386, coff version (DJGPP)
qfltb386.o: qfltbi.386
	as -o qfltb386.o qfltbi.386

subml.o: subml.asm qheadl.asm
mulrl.o: mulrl.asm qheadl.asm
divnl.o: divnl.asm qheadl.asm

mkdep:
	mkdep $(OBJECTS_A:.o=.c) qsqrt*.c

clist:
	@ls.exe -1 $(OBJECTS_C:.o=.c) $(OBJ_SQRT:.o=.c)

save:
	-pkzip -u -ex -x*.bak qfloat *.c *.asm *.386 *.h *.txt *.doc\
		 makefile.*

mklist:
	@echo -n $(OBJECTS_C:.o=.c) $(OBJS_ASM:.o=.asm) $(OBJ_SQRT:.o=.c)
	@echo "" $(HEADERS)

clean:
	-rm -f *.o *.a
