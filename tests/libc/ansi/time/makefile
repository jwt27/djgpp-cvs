TOP=../..

SRC += mktime.c
SRC += strftime.c
SRC += tzinfo.c
SRC += xstrftm.c

include $(TOP)/../makefile.inc

# Ignore warning about strtfime %x format.
xstrftm.exe:	CFLAGS += -Wno-error
