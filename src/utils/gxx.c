/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
#include <stdio.h>
#include <process.h>
#include <stdlib.h>

char lgpp[] = "-lgpp";
char lstdc[] = "-lstdcxx";
char lm[] = "-lm";

int
main(int argc, char **argv)
{
  char **newargs = (char **)malloc((argc+4) * sizeof(char *));
  int i;
  int add_libs = 1;

  for (i=0; i<argc; i++)
  {
    if (strcmp(argv[i], "-c") == 0
	|| strcmp(argv[i], "-S") == 0)
      add_libs = 0;
    newargs[i] = argv[i];
  }

  if (add_libs) {
    newargs[i++] = lstdc;
    newargs[i++] = lgpp;
    newargs[i++] = lm;
  }
  newargs[i++] = 0;

  return execvp("gcc.exe", newargs);
}
