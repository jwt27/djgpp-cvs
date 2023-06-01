GCC = gcc

%.o : %.c
	$(GCC) $(CFLAGS) -fpic -c -o $@ $<

LIB = ../../lib

all : libdjdev64.so

clean :
	rm *.o libdjdev64.so

libdjdev64.so : djdev64.o
	$(GCC) -shared -o $@ $^
