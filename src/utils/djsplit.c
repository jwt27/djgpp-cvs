/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <dpmi.h>

static void
usage(void)
{
  fprintf(stderr,"Usage: djsplit [inputFile] [chunkSize] [outputBase]\n");
  fprintf(stderr, "chunksize is bytes or kbytes (ex: 1440k), creates <outputBase>.000, <outputBase>.001, etc\n");
  exit(1);
}

static int
p_open(char *ob, int p)
{
  char partname[1024];
  sprintf(partname, "%s.%03d", ob, p);
  printf("%s...\n", partname);
  return open(partname, O_WRONLY|O_CREAT|O_TRUNC|O_BINARY, 0666);
}

char *buf;
int bufsize;

int
main(int argc, char **argv)
{
  long chunksize, left, r;
  int partnum;
  int inf, f;
  char *endp;
  
  if (argc != 4)
    usage();

  inf = open(argv[1], O_RDONLY|O_BINARY);
  if (inf < 0)
    usage();

  bufsize = _go32_dpmi_remaining_physical_memory();
  do {
    bufsize /= 2;
    buf = malloc(bufsize);
  } while (buf == 0 && bufsize > 16384);

  /* Protect against crazy DPMI hosts which
     won't cooperate.  This really should never happen.  */
  if (buf == NULL)
  {
    bufsize = 4 * 1024;		/* this will be painfully slow... */
    buf = (char *) alloca(bufsize);
  }
  printf("buf size: %d\n", bufsize);

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
    if (left < bufsize)
      r = read(inf, buf, left);
    else
      r = read(inf, buf, bufsize);
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
