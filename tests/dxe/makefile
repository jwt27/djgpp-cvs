
.SUFFIXES: .exe .dxe

CC = gcc
CFLAGS = -O2 -Wall -W -pedantic

CXX = gpp
CXXFLAGS = $(CFLAGS) -fno-exceptions
DXE3GEN = dxe3gen
DXE3RES = dxe3res
LD = gxx

.c.o:
	$(CC) -o $@ -c $(CFLAGS) $<
.cpp.o:
	$(CXX) -o $@ -c $(CXXFLAGS) $<
.o.exe:
	$(LD) -o $@ $(LDFLAGS) $^
.o.dxe:
	$(DXE3GEN) -o $@ $< -U

all: dxetest.exe \
      dlsimp.exe dldemo.exe statsimp.exe statlink.exe statfull.exe test3.dxe

dxetest.exe: dxetest.c add.dxe
	gcc -O2 dxetest.c -o dxetest.exe
	dxetest

add.dxe: add.o
	dxegen add.dxe _add add.o

dlsimp.exe: dlsimp.o 
dldemo.exe: dldemo.o 
statsimp.exe: statsimp.o libtst1.a
statlink.exe: statlink.o libtst2.a
statfull.exe: export.o statfull.o libtst2x.a libtst3p.a

libtst1.a: test1.dxe
	$(DXE3GEN) -I $@ $<

libtst2.a: test2.dxe
	$(DXE3GEN) -I $@ $<

libtst2x.a: test2x.dxe
	$(DXE3GEN) -I $@ $<

libtst3p.a: test3p.dxe
	$(DXE3GEN) -I $@ $<

test1.dxe: test1.o
	$(DXE3GEN) -o $@ $^ -lc -E _my_

test2.dxe: test2.o

test2x.dxe: test2x.o

test3.dxe: test3.o

test3p.dxe: test3.o
	$(DXE3GEN) -o $@ $^ -U -P test2x.dxe

test2x.o: test2.cpp
	$(CXX) -o $@ -c $(CXXFLAGS) -fexceptions -DTEST_CPP_EXC $<

export.c: test2x.dxe test3p.dxe
	$(DXE3RES) -o $@ $^

export.o: export.c
	$(CC) -o $@ -c $(CFLAGS) $<

run:
	dxetest
	dlsimp
	dldemo
	statsimp
	statlink
	statfull

clean:
	-rm *.o *.*xe *.a export.c
