# Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details
CC = gcc

default: nobins

nobins:	nobins.c
	$(CC) -g -Wall -o $@ $<

clean:
	rm -f nobins nobins.exe
