/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
#include <stdio.h>
#include <process.h>
#include <stdlib.h>

char lgpp[] = "-lgpp";
char lm[] = "-lm";

int
main(int argc, char **argv)
{
  char **newargs = (char **)malloc((argc+3) * sizeof(char *));
  int i;
  for (i=0; i<argc; i++)
    newargs[i] = argv[i];

  newargs[i++] = lgpp;
  newargs[i++] = lm;
  newargs[i++] = 0;

  return execvp("gcc.exe", newargs);
}
