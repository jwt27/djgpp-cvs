# Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details
../../bin/mkdoc.exe : mkdoc.o
	ld ../../lib/crt0.o $^ -L../../lib/ -lgcc -lc -o ../../bin/mkdoc.exe -T ../../lib/djgpp.djl
	../../bin/stubify -v ../../bin/mkdoc.exe

mkdoc.o : mkdoc.cc
	gcc -nostdinc -I../../include -c mkdoc.cc
