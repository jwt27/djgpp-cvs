/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdio.h>
#include <dir.h>
#include <dos.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct F {
	char *name;
	struct F *next;
	} F;

static int
dtou(char *fname)
{
  int sf, df, l;
  char buf[16384];
  char tfname[80], drive[3], path[80];
  struct ftime ftime;
  sf = open(fname, O_RDONLY|O_TEXT);
  if (sf < 1)
  {
    perror(fname);
    return 1;
  }
  fnsplit(fname, drive, path, NULL, NULL);
  fnmerge(tfname, drive, path, "utod", "tm$");
  df = open(tfname, O_WRONLY|O_CREAT|O_TRUNC|O_BINARY, 0644);
  if (df < 1)
  {
    perror(tfname);
    close(sf);
    return 1;
  }

  while ((l=read(sf, buf, 16384)) > 0)
    write(df, buf, l);

  getftime(sf, &ftime);
  setftime(df, &ftime);
  close(sf);
  close(df);

  remove(fname);
  rename(tfname, fname);
  return 0;
}

int
main(int argc, char **argv)
{
  int rv = 0;
  for (argc--, argv++; argc; argc--, argv++)
    rv += dtou(*argv);
  return rv;
}
