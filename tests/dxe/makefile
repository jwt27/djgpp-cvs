
all: dxetest.exe

dxetest.exe: dxetest add.dxe
	stubify dxetest
	dxetest

add.dxe:
	gcc -c add.c
	dxegen add.dxe _add add.o
