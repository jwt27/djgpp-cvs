/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#ifndef O_BINARY
#define O_BINARY 0
#endif

#define BUFS 16384

static void
usage(void)
{
  fprintf(stderr,"Usage: djmerge [inputBase] [outputFile]\n");
  fprintf(stderr, "reads <inputBase>.000, <inputBase>.001, etc\n");
  exit(1);
}

static int
p_open(char *ob, int p)
{
  char partname[1024];
  sprintf(partname, "%s.%03d", ob, p);
  return open(partname, O_RDONLY|O_BINARY);
}

int
main(int argc, char **argv)
{
  char buf[BUFS];
  long r;
  int partnum;
  int outf, f;
  
  if (argc != 3)
    usage();

  outf = open(argv[2], O_WRONLY|O_CREAT|O_TRUNC|O_BINARY, 0666);
  if (outf < 0)
    usage();

  partnum = 0;
  f = p_open(argv[1], partnum);
  if (f < 0)
  {
    fprintf(stderr,"FATAL: Cannot open %s.000", argv[1]);
    perror("");
    exit(1);
  }
  while (1)
  {
    r = read(f, buf, BUFS);

    if (r <= 0)
    {
      close(f);
      partnum++;
      f = p_open(argv[1], partnum);

      if (f < 0)
      {
        close(outf);
        exit(0);
      }
    }
    
    write(outf, buf, r);
  }
}
