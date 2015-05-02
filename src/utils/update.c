/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/* This file is intended to be compiled with Turbo-C */

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <io.h>

static void
fatal(const char *s)
{
  fputs(s, stderr);
  exit(1);
}

int
main(int argc, char **argv)
{
  int f1;
  int f2;
  char buf1[512];
  char buf2[512];
  int len1, len2;
  
  if (argc < 3)
  {
    fprintf(stderr, "Usage: update <srcfile> <destfile>\n");
    fprintf(stderr, "If srcfile is different than destfile, srcfile is copied to destfile.\n");
    exit(1);
  }

  f1 = _open(argv[1], O_RDONLY);
  if (f1 < 0)
    fatal("Cannot open src file\n");

  f2 = _open(argv[2], O_RDONLY);

  if (f2 >= 0)
    while (1)
    {
      len1 = _read(f1, buf1, 512);
      len2 = _read(f2, buf2, 512);
      if (len1 != len2)
        break;
      if (memcmp(buf1, buf2, len1))
        break;
      if (len1)
        continue;
      exit(0);
    }

  if (f2 >= 0)
  {
    printf("File `%s' updated\n", argv[2]);
    _close(f2);
  }
  else
    printf("File `%s' created\n", argv[2]);

  lseek(f1, 0L, 0);
  f2 = _creat(argv[2], 0);
  while (1)
  {
    len1 = _read(f1, buf1, 512);
    if (len1 == 0)
      break;
    len2 = _write(f2, buf1, len1);
  }
  _close(f1);
  _close(f2);
  exit(0);
}
