# Makefile for C version of ELEFUNT package

# NB: If your C compiler implements double and float in different
# hardware precisions, the tests MUST be carried out in pure
# double; mixed float and double will give incorrect results.
# IEEE flag must be set for special code in machar() needed on
# IEEE arithmetic hosts.

# -O gives wrong results

# can be called with dmake -DCOMPILER=1
# or (for GNU Make) with make COMPILER=1
#   where COMPILER is TURBOC, TCC, BORLANDC, BCC or GCC
#   GCC is the default

ifdef TCC
  TURBOC = 1
endif
ifdef BCC
  BORLANDC = 1
endif
ifdef TURBOC
  BORLAND = 1
endif
ifdef BORLANDC
  BORLAND = 1
endif

# where to find include files
INCLUDE = -I.
# how to redirect output
ERROUT = >    # 4dos >&
DEFINES = -DIEEE
ifdef LDOUBLE
    DEFINES += -DLDOUBLE
endif

ifdef BORLAND
  MODEL *= l
  LDIR = -L..\..\lib
  LIBS = mathn$(MODEL).lib
  ELIB = em$(MODEL).lib
  OE = obj  # Extension of objects
  EXE = .exe # Extension of executables
  OUT = -e
  RUN =     # how to run the executables
ifdef TURBOC
    CC = tcc
    LD = tcc
    OPTIMIZE = -G -r+ -ff -d+ -O
    LOG = tcl # Extension of log files
else
    CC = bcc
    LD = bcc
    OPTIMIZE = -O2
    LOG = bcl # Extension of log files
endif
ifdef DEBUG
    LDFLAGS = -v -m$(MODEL) $(LDIR)
    CFLAGS = $(OPTIMIZE) $(INCLUDE) -m$(MODEL) -N+ -2 -w -v $(DEFINES)
else
    CFLAGS = $(OPTIMIZE) $(INCLUDE) -m$(MODEL) -N+ -2 -w  $(DEFINES)
    LDFLAGS = -m$(MODEL) $(LDIR)
endif
else  # no borland compiler, we use gcc
  LDIR = -L.
  OE = o  # Extension of objects
  EXE =.exe   # Extension of executables
  LOG = djl # Extenion of the log files
  OUT = -o
  CC = gcc -I../../../../../include
  LD = gcc
  LIBRARY_PATH := ../../../../../lib
  export LIBRARY_PATH
  RUN =    # how to run the executables
ifdef DEBUG
    # don't use -O!!
    CFLAGS = -g -Wall -fno-builtin $(INCLUDE) $(DEFINES)
    LDFLAGS = -g $(LDIR)
else
    CFLAGS = -Wall -fno-builtin $(INCLUDE) $(DEFINES)
    LDFLAGS = -s $(LDIR)
endif
endif

RM = -rm -f

OBJS = machar.$(OE) ran.$(OE) randl.$(OE) store.$(OE) ipow.$(OE) init.$(OE)

TARGETS = \
	tmacha.$(LOG) \
	talog.$(LOG)  \
	tasin.$(LOG)  \
	tatan.$(LOG)  \
	texp.$(LOG)   \
	texp2.$(LOG)  \
	texp10.$(LOG) \
	tpower.$(LOG) \
	tsin.$(LOG)   \
	tsinh.$(LOG)  \
	tsqrt.$(LOG)  \
	ttan.$(LOG)   \
	ttanh.$(LOG)

# $(EXECUTABLES) = $(TARGETS:/s/.$(LOG)/$(EXE))

all ::    $(TARGETS)
	@rem.com

# how to build the log files
%.$(LOG) : %$(EXE)
	$(RUN) $< $(ERROUT) $@

# $TARGETS : $(EXECUTABLES)

# This target just shows a quick summary of the tests with their
# name and accuracy loss.  Detailed examination of the output is
# required for a proper evaluation of the results.
summary:
	egrep "^1|ESTIMATED LOSS" *$(LOG)

clean:
	$(RM) *.o *.obj *.bak *.exe
	$(RM) talog tasin tatan texp tpower tsin tsinh
	$(RM) tsqrt ttan ttanh tmacha

realclean : clean       # delete log files, too
	$(RM) *.djl *.bcl *.tcl
ifdef BORLAND
tmacha$(EXE):	tmacha.$(OE) $(OBJS)
	$(LD) $(LDFLAGS) $(OUT)$@ $< $(LIBS)
talog$(EXE):	malog.$(OE) talog.$(OE) $(OBJS)
	$(LD) $(LDFLAGS) $(OUT)$@ $< $(LIBS)
tasin$(EXE):	masin.$(OE) tasin.$(OE) $(OBJS)
	$(LD) $(LDFLAGS) $(OUT)$@ $< $(LIBS)
tatan$(EXE):	matan.$(OE) tatan.$(OE) $(OBJS)
	$(LD) $(LDFLAGS) $(OUT)$@ $< $(LIBS)
texp$(EXE):	mexp.$(OE) texp.$(OE) $(OBJS)
	$(LD) $(LDFLAGS) $(OUT)$@ $< $(LIBS)
texp2$(EXE):	mexp2.$(OE) texp2.$(OE) $(OBJS)
	$(LD) $(LDFLAGS) $(OUT)$@ $< $(LIBS)
texp10$(EXE):	mexp10.$(OE) texp10.$(OE) $(OBJS)
	$(LD) $(LDFLAGS) $(OUT)$@ $< $(LIBS)
tpower$(EXE):	mpower.$(OE) tpower.$(OE) $(OBJS)
	$(LD) $(LDFLAGS) $(OUT)$@ $< $(LIBS)
tpowere$(EXE):	mpower.$(OE) tpowere.$(OE) $(OBJS)
	$(LD) $(LDFLAGS) $(OUT)$@ $< $(LIBS) $(ELIB)
tsin$(EXE):	msin.$(OE) tsin.$(OE) $(OBJS)
	$(LD) $(LDFLAGS) $(OUT)$@ $< $(LIBS)
tsinh$(EXE):	msinh.$(OE) tsinh.$(OE) $(OBJS)
	$(LD) $(LDFLAGS) $(OUT)$@ $< $(LIBS)
tsqrt$(EXE):	msqrt.$(OE) tsqrt.$(OE) $(OBJS)
	$(LD) $(LDFLAGS) $(OUT)$@ $< $(LIBS)
ttan$(EXE):	mtan.$(OE) ttan.$(OE) $(OBJS)
	$(LD) $(LDFLAGS) $(OUT)$@ $< $(LIBS)
ttanh$(EXE):	mtanh.$(OE) ttanh.$(OE) $(OBJS)
	$(LD) $(LDFLAGS) $(OUT)$@ $< $(LIBS)
else
tmacha$(EXE):	tmacha.$(OE) $(OBJS)
	$(LD) $(LDFLAGS) $(OUT) $@ $^ $(LIBS)
talog$(EXE):	malog.$(OE) talog.$(OE) $(OBJS)
	$(LD) $(LDFLAGS) $(OUT) $@ $^ $(LIBS)
tasin$(EXE):	masin.$(OE) tasin.$(OE) $(OBJS)
	$(LD) $(LDFLAGS) $(OUT) $@ $^ $(LIBS)
tatan$(EXE):	matan.$(OE) tatan.$(OE) $(OBJS)
	$(LD) $(LDFLAGS) $(OUT) $@ $^ $(LIBS)
texp$(EXE):	mexp.$(OE) texp.$(OE) $(OBJS)
	$(LD) $(LDFLAGS) $(OUT) $@ $^ $(LIBS)
texp2$(EXE):	mexp2.$(OE) texp2.$(OE) $(OBJS)
	$(LD) $(LDFLAGS) $(OUT) $@ $^ $(LIBS)
texp10$(EXE):	mexp10.$(OE) texp10.$(OE) $(OBJS)
	$(LD) $(LDFLAGS) $(OUT) $@ $^ $(LIBS)
tpower$(EXE):	mpower.$(OE) tpower.$(OE) $(OBJS)
	$(LD) $(LDFLAGS) $(OUT) $@ $^ $(LIBS)
tsin$(EXE):	msin.$(OE) tsin.$(OE) $(OBJS)
	$(LD) $(LDFLAGS) $(OUT) $@ $^ $(LIBS)
tsinh$(EXE):	msinh.$(OE) tsinh.$(OE) $(OBJS)
	$(LD) $(LDFLAGS) $(OUT) $@ $^ $(LIBS)
tsqrt$(EXE):	msqrt.$(OE) tsqrt.$(OE) $(OBJS)
	$(LD) $(LDFLAGS) $(OUT) $@ $^ $(LIBS)
ttan$(EXE):	mtan.$(OE) ttan.$(OE) $(OBJS)
	$(LD) $(LDFLAGS) $(OUT) $@ $^ $(LIBS)
ttanh$(EXE):	mtanh.$(OE) ttanh.$(OE) $(OBJS)
	$(LD) $(LDFLAGS) $(OUT) $@ $^ $(LIBS)
endif

# dependencies found by gcc -MM *.c

tpower.$(OE) : tpower.c elefunt.h
talog.$(OE) : talog.c elefunt.h
tasin.$(OE) : tasin.c elefunt.h
tatan.$(OE) : tatan.c elefunt.h
tsin.$(OE) : tsin.c elefunt.h
tsinh.$(OE) : tsinh.c elefunt.h
tsqrt.$(OE) : tsqrt.c elefunt.h
ttan.$(OE) : ttan.c elefunt.h
ttanh.$(OE) : ttanh.c elefunt.h
tmacha.$(OE) : tmacha.c elefunt.h
atan.$(OE) : atan.c
exp.$(OE) : exp.c
ipow.$(OE) : ipow.c elefunt.h
ldexp.$(OE) : ldexp.c
tsmach.$(OE) : tsmach.c elefunt.h
malog.$(OE) : malog.c elefunt.h
masin.$(OE) : masin.c elefunt.h
matan.$(OE) : matan.c elefunt.h
mexp.$(OE) : mexp.c elefunt.h
mpower.$(OE) : mpower.c elefunt.h
msin.$(OE) : msin.c elefunt.h
msinh.$(OE) : msinh.c elefunt.h
msqrt.$(OE) : msqrt.c elefunt.h
mtan.$(OE) : mtan.c elefunt.h
mtanh.$(OE) : mtanh.c elefunt.h
errshow.$(OE) : errshow.c
pow.$(OE) : pow.c
ran.$(OE) : ran.c elefunt.h
randl.$(OE) : randl.c elefunt.h
sinh.$(OE) : sinh.c
store.$(OE) : store.c elefunt.h
tan.$(OE) : tan.c
tldexp.$(OE) : tldexp.c elefunt.h
ftnprt.$(OE) : ftnprt.c
terrshow.$(OE) : terrshow.c
machar.$(OE) : machar.c elefunt.h
mather.$(OE) : mather.c
texp.$(OE) : texp.c elefunt.h
texp2.$(OE) : texp2.c elefunt.h
texp10.$(OE) : texp10.c elefunt.h
init.$(OE) : init.c
