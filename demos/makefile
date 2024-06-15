all:
	$(MAKE) -C hello
	$(MAKE) -C helloasm
	$(MAKE) -C rmcb

djgpp:
	$(MAKE) -C hello -f makefile.djgpp
	$(MAKE) -C helloasm -f makefile.djgpp
	$(MAKE) -C rmcb -f makefile.djgpp

clean:
	$(MAKE) -C hello clean
	$(MAKE) -C helloasm clean
	$(MAKE) -C rmcb clean
