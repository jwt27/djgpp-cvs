# Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details
TOP=../..

SRC += regcomp.c
SRC += regerror.c
SRC += regexec.c
SRC += regfree.c

include $(TOP)/../makefile.inc

GCC = gcc -O2 -I. -I- -I$(TOP)/../../include $(CFLAGS) -c $<
