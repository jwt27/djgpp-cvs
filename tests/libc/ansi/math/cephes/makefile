# can be called with dmake -DCOMPILER=1
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

DEFINES =

ifdef LDOUBLE
    DEFINES += -DLDOUBLE
endif

ifdef BORLAND
  MODEL *= l
  LDIR = -L..\..\lib
  LIBS = mathn$(MODEL).lib
  OE = obj  # Extension of objects
  OUT = -e
ifdef TURBOC
    CC = tcc
    LD = tcc
    OPTIMIZE = -G -r+ -ff -d+ -O
else
    CC = bcc
    LD = bcc
    OPTIMIZE = -O2
endif
ifdef DEBUG
    LDFLAGS = -v -m$(MODEL) $(LDIR)
    CFLAGS = $(OPTIMIZE) $(INCLUDE) -m$(MODEL) -N+ -3 -w -v $(DEFINES)
else
    CFLAGS = $(OPTIMIZE) $(INCLUDE) -m$(MODEL) -N+ -3 -w  $(DEFINES)
    LDFLAGS = -m$(MODEL) $(LDIR)
endif
  TARGETS = mtst.exe
else  # no borland compiler, we use gcc
  OE = o  # Extension of objects
  OUT = -o
  CC = gcc -I../../../../../include
  LD = gcc
  LIBRARY_PATH := ../../../../../lib
  export LIBRARY_PATH
ifdef DEBUG
    CFLAGS = -g -Wall -O $(DEFINES) $(INCLUDE)
    LDFLAGS = -g $(LDIR)
    TARGETS = mtstg.exe
else
    CFLAGS = -Wall -O $(DEFINES) $(INCLUDE)
    LDFLAGS = $(LDIR)
    TARGETS = mtst.exe
endif
endif

.c.$(OE) :
	$(CC) -c $(CFLAGS) $<

OBJ= drand.$(OE) const.$(OE) mtst.$(OE)

all :: $(TARGETS)
	@rem.com

mtst.exe : $(OBJ)
	$(LD) $(LDFLAGS) $(OUT) $@ $^ $(LIBS)

mtstg.exe : $(OBJ)
	$(LD) $(LDFLAGS) $(OUT) $@ $^ $(LIBS)

clean :
	rm -f *.o *.obj mtst.exe mtstg.exe *.bak


const.$(OE) : const.c mconf.h
drand.$(OE) : drand.c mconf.h
mtst.$(OE) : mtst.c mconf.h
