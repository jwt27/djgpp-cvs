/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define BUFS  16384

static void
usage(void)
{
  fprintf(stderr,"Usage: split [inputFile] [chunkSize] [outputBase]\n");
  fprintf(stderr, "chunksize is bytes or kbytes (ex: 1440k), creates <outputBase>.000, <outputBase>.001, etc\n");
  exit(1);
}

static int
p_open(char *ob, int p)
{
  char partname[1024];
  sprintf(partname, "%s.%03d", ob, p);
  return open(partname, O_WRONLY|O_CREAT|O_TRUNC|O_BINARY, 0666);
}

int
main(int argc, char **argv)
{
  char buf[BUFS];
  long chunksize, left, r;
  int partnum;
  int inf, f;
  char *endp;
  
  if (argc != 4)
    usage();

  inf = open(argv[1], O_RDONLY|O_BINARY);
  if (inf < 0)
    usage();

  chunksize = strtol(argv[2], &endp, 0);
  if (chunksize < 1)
    usage();
  switch (*endp)
  {
    case 'k':
    case 'K':
      chunksize *= 1024L;
      break;
    case 'm':
    case 'M':
      chunksize *= 1048576L;
      break;
  }

  partnum = 0;
  left = chunksize;
  f = p_open(argv[3], partnum);
  while (1)
  {
    if (left < BUFS)
      r = read(inf, buf, left);
    else
      r = read(inf, buf, BUFS);
    if (r <= 0)
    {
      close(f);
      close(inf);
      exit(0);
    }
    
    write(f, buf, r);
    left -= r;
    
    if (left == 0)
    {
      close(f);
      partnum++;
      f = p_open(argv[3], partnum);
      left = chunksize;
    }
  }
}
