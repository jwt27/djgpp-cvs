%.o : %.cc
	gcc -O2 -g -c $<

.PHONY: all p_check.out p_compare.out clean

all : p_check.out p_compare.out

p_check.out : check.out
	@cat check.out

p_compare.out : compare.out
	@cat compare.out

check.out : check ../../lib/crt0.o ../../lib/libc.a
	@nm -g ../../lib/crt0.o ../../lib/libc.a | check > check.out

check : check.o slist.o objs.o
	gcc -o $@ $^
	coff2exe $@

compare.out : compare oldlibc.arm newlibc.arm
	@compare oldlibc.arm newlibc.arm > compare.out

compare : compare.o slist.o
	gcc -o $@ $^
	coff2exe $@

oldlibc.arm : /v1/lib/libc.a /v1/lib/libpc.a /v1/lib/libm.a
	nm --print-armap /v1/lib/libc.a | grep " in " > oldlibc.arm
	nm --print-armap /v1/lib/libpc.a | grep " in " >> oldlibc.arm
	nm --print-armap /v1/lib/libm.a | grep " in " >> oldlibc.arm

newlibc.arm : ../../lib/libc.a
	nm --print-armap ../../lib/libc.a > newlibc.arm

clean :
	-rm -f check.out check check.exe *.o libc.map