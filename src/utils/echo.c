/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdio.h>

int
main(int argc, char **argv)
{
  int i;
  int nflag=0;
  FILE *out = stdout;
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
    if ((argc > 2) && (strcmp(argv[1], "-a") == 0))
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
    if ((argc > 1) && (strcmp(argv[1], "-n") == 0))
    {
      nflag = 1;
      argc--;
      argv++;
    }
  }
  for (i=1; i<argc; i++)
  {
    if (i>1) fputc(' ', out);
    fputs(argv[i], out);
  }
  if (!nflag)
    fputc('\n', out);
  fclose(out);
  return 0;
}
