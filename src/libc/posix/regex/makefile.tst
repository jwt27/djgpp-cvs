CC = gcc
RM = rm

CFLAGS  += -g -Wall -I../../../../include
LDFLAGS += -g -Wall -L../../../../lib

OBJS = main.o debug.o split.o

default:	check

# Check target copied from makefile.bsd
check:	re tests
	./re <tests
	./re -el <tests
	./re -er <tests

check-ignore:	re tests
	-./re <tests
	-./re -el <tests
	-./re -er <tests

re:	$(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o:	%.c
	$(CC) $(CFLAGS) -c $<

clean:
	$(RM) -f $(OBJS) re re.exe
