/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdio.h>
#include <string.h>

int
main(int argc, char **argv)
{
  int i;
  int nflag=0;
  FILE *out = stdout;
  char sep = ' ';

  while (argc > 1 && argv[1][0] == '-')
  {
    if ((argc > 2) && (strcmp(argv[1], "-o") == 0))
    {
      out = fopen(argv[2], "w");
      if (!out)
      {
	perror(argv[2]);
	out = stdout;
      }
      argc -= 2;
      argv += 2;
    }
    else if ((argc > 2) && (strcmp(argv[1], "-a") == 0))
    {
      out = fopen(argv[2], "a");
      if (!out)
      {
	perror(argv[2]);
	out = stdout;
      }
      argc -= 2;
      argv += 2;
    }
    else if ((argc > 1) && (strcmp(argv[1], "-n") == 0))
    {
      nflag = 1;
      argc--;
      argv++;
    }
    else if ((argc > 1) && (strcmp(argv[1], "-s") == 0))
    {
      sep = '\n';
      argc--;
      argv++;
    }
    else
    {
      fprintf(stderr, "Usage: echo [-o file] [-a file] [-n] [-s] args...\n");
      return 1;
    }
  }
  for (i=1; i<argc; i++)
  {
    if (i>1) fputc(sep, out);
    fputs(argv[i], out);
  }
  if (!nflag)
    fputc('\n', out);
  fclose(out);
  return 0;
}
