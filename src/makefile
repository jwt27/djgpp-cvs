all : misc.exe config makemake.exe subs

misc.exe : misc.c
	$(CC) -O2 -Wall misc.c -o misc.exe

makemake.exe : makemake.c
	$(CC) -O2 -Wall makemake.c -o makemake.exe

subs:
	$(MAKE) -C libc

.PHONY : clean
clean : misc.exe makemake.exe
	$(MAKE) -C libc clean
	./misc.exe rm gcc.opt makefile.sub
	./misc.exe rm makemake.exe copyrite.exe misc.exe

config: misc.exe
	$(MAKE) -f makefile.cfg
