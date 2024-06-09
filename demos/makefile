all:
	$(MAKE) -C hello
	$(MAKE) -C hello_asm
	$(MAKE) -C rmcb

djgpp:
	$(MAKE) -C hello -f makefile.djgpp
	$(MAKE) -C hello_asm -f makefile.djgpp
	$(MAKE) -C rmcb -f makefile.djgpp

clean:
	$(MAKE) -C hello clean
	$(MAKE) -C hello_asm clean
	$(MAKE) -C rmcb clean
