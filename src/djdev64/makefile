GCC = gcc

%.o : %.c
	$(GCC) $(CFLAGS) -iquote ../../include -fpic -c -o $@ $<

LIB = ../../lib

all : libdjdev64.so

clean :
	rm -f *.o libdjdev64.so

libdjdev64.so : djdev64.o
	$(GCC) -shared -o $@ $^
