# ---------------------------------------------------------------------------
# Copyright 1995-1996 by Morten Welinder (terra@diku.dk)
# Distributed under the GPL, see COPYING for details.
# ---------------------------------------------------------------------------
ASM16 = djasm
CFLAGS = -m486 -DDEBUG -O2 -Wall
LIBDIR = ../lib

INCFILES = \
	cleanup.inc \
	cpu.inc \
	debug.inc \
	defines.inc \
	dos.inc \
	ems-vcpi.inc \
	error.inc \
	exeparam.inc \
	highdata.inc \
	hook-int.inc \
	low-data.inc \
	memory.inc \
	movego32.inc \
	page-dir.inc \
	real-15.inc \
	real-2f.inc \
	resident.inc \
	sw-raw.inc \
	sw-vcpi.inc \
	unload.inc \
	warphigh.inc \
	xms.inc

ASMFILES = \
	dpmi.asm

CFILES = \
	callback.c \
	clients.c \
	cmdline.c \
	globals.c \
	ldt.c \
	main.c \
	memory.c \
	prot-31.c \
	serv-int.c \
	sprintf.c \
	write.c

HFILES = \
	clients.h \
	code32.h \
	cpu.h \
	interface.h \
	ldt.h

SFILES = \
	interrupt.S \
	misc.S

MISCFILES = \
	defines.sed \
	doc \
	link.scr \
	copying \
	readme \
	makefile

.SUFFIXES: .dis
.o.dis:
	objdump --disassemble $< > $@

%.o : %.S
	${CC} -c -o $@ $<
# ---------------------------------------------------------------------------
all: dpmi.exe

dpmi.exe dpmi.map: dpmi.asm $(INCFILES) code32.o
	$(ASM16) dpmi.asm dpmi.exe dpmi.map
#	cp dpmi.exe c:/

code32.o: $(CFILES:.c=.o) $(SFILES:.S=.o) link.scr
	ld -o $@ -r -Tlink.scr -L${LIBDIR} $+ -lc
# It would be nice to run the link command through "gcc", but gcc does
# too many things we don't want: sector alignment, .exe file, more?

defines.h: defines.inc defines.sed
	sed -f defines.sed defines.inc > $@

# zip (pkzip) is too stupid to take long arguments in a reasonable format
# so we have to do it the simple way.
server.zip: clean
	rm -f server.zip
	zip server ${ASMFILES} ${MISCFILES}
	zip server *.c *.inc *.s *.h
# ---------------------------------------------------------------------------
# pseudo-targets.  (See also "all:" above)

clean:
	rm -f dpmi.exe dpmi.map code32.o defines.h
	rm -f $(CFILES:.c=.o)
	rm -f $(SFILES:.S=.o) *.dis

distclean: clean
	rm -f *~

tags TAGS: ${CFILES} ${HFILES} ${INCFILES} ${SFILES} ${ASMFILES}
	etags --language=c ${CFILES} ${HFILES} \
	      --language=asm ${INCFILES} ${SFILES} ${ASMFILES}

zip: server.zip
# -----------------------------------------------------------------------------
BASIC = code32.h interface.h defines.h cpu.h

callback.o: ${BASIC} clients.h dpmi.h ldt.h
clients.o: ${BASIC} clients.h dpmi.h ldt.h
cmdline.o: ${BASIC}
globals.o: interface.h cpu.h
interrupt.o: defines.h
ldt.o: ${BASIC} ldt.h clients.h
main.o: ${BASIC} clients.h
memory.o: ${BASIC}
prot-31.o: ${BASIC} clients.h dpmi.h ldt.h
serv-int.o: ${BASIC}
sprintf.o: ${BASIC}
write.o: ${BASIC}
