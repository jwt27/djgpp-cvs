/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/* Miscellaneous things that are hard to do the same
   between Unix and MS-DOS */

int
main(int argc, char **argv)
{
  /* MS-DOS uses \, unix uses / */
  if (argc > 2 && strcmp(argv[1], "mkdir") == 0)
    mkdir(argv[2], 0777);

  /* redirection and long command lines don't always
     mix well under MS-DOS */
  if (argc > 2 && strcmp(argv[1], "echo") == 0)
  {
    FILE *f;
    int i;
    if (strcmp(argv[2], "-") == 0)
      f = stdout;
    else
    {
      f = fopen(argv[2], "w");
      if (f == 0)
      {
	perror(argv[2]);
	exit(1);
      }
    }
    if (f == 0)
    {
      perror(argv[2]);
      exit(1);
    }
    for (i=3; i<argc; i++)
    {
      if (i > 3) fputc(' ', f);
      fputs(argv[i], f);
    }
    fputc('\n', f);
    fflush(f);
    if (f != stdout)
      fclose(f);
  }

  /* copy \ vs cp / */
  if (argc > 3 && strcmp(argv[1], "cp") == 0)
  {
    FILE *in, *out;
    int c;
    in = fopen(argv[2], "rb");
    if (!in) {
      printf("misc: cp: can't read from %s\n", argv[2]);
      exit(1);
    }
    out = fopen(argv[3], "wb");
    if (!out) {
      printf("misc: cp: can't write to %s\n", argv[3]);
      exit(1);
    }
    while ((c = fgetc(in)) != EOF)
      fputc(c, out);
    fclose(in);
    fclose(out);
  }

  /* erase \ vs rm / */
  if (argc > 2 && strcmp(argv[1], "rm") == 0)
  {
    int i;
    for (i=2; i<argc; i++)
      unlink(argv[i]);
  }

  /* No args, just like "true" which MS-DOS doesn't have */
  return 0;
}
