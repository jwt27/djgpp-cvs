all:: paranoia.exe
	@rem.com

LIBRARY_PATH := ../../../../../lib
C_INCLUDE_PATH := ../../../../../include
export LIBRARY_PATH
export C_INCLUDE_PATH

paranoia.exe: paranoia.c
	gcc -Wall -fno-builtin -g -o paranoia.exe paranoia.c

test: paranoia.exe
	paranoia
