# Hello ASM demo

This demo shows the use of an asm code in dj64-enabled projects.
Namely, the calls between C and asm code in both directions,
touching asm variables from C, and so on.

## building

Run `make` to build with dj64, or `make -f makefile.djgpp` to build
with djgpp. Run `make info` to print the information about the built
executable.

## running

`dosemu -T ./hello.exe`
