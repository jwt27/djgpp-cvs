/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdio.h>
#include <dir.h>
#include <dos.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

typedef struct F {
	char *name;
	struct F *next;
	} F;

static int
utod(char *fname)
{
  int sf, df, l;
  struct ftime ftime;
  char buf[16384];
  char tfname[80], drive[3], path[80];
  sf = open(fname, O_RDONLY|O_BINARY);
  if (sf < 0)
  {
    perror(fname);
    return 1;
  }
  fnsplit(fname, drive, path, NULL, NULL);
  fnmerge(tfname, drive, path, "utod", "tm$");
  df = open(tfname, O_WRONLY|O_CREAT|O_TRUNC|O_TEXT, 0644);
  if (df < 0)
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
    rv += utod(*argv);
  return rv;
}
