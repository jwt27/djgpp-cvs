MAKEFLAGS := --no-print-directory

all : misc.exe config makemake.exe subs

misc.exe : misc.c
	gcc -O2 -Wall misc.c -o misc.exe

makemake.exe : makemake.c
	gcc -O2 -Wall makemake.c -o makemake.exe

copyrite.exe : copyrite.cc
	gcc -O2 -Wall copyrite.cc -o copyrite.exe

subs:
	$(MAKE) -C libc

.PHONY : clean
clean : misc.exe makemake.exe
	$(MAKE) -C libc clean
	./misc.exe rm gcc.opt gcc-l.opt makefile.sub
	./misc.exe rm makemake.exe copyrite.exe misc.exe

config: misc.exe
	$(MAKE) -f makefile.cfg
