/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <utime.h>
#include <dpmi.h>

static void
usage(void)
{
  fprintf(stderr,"Usage: djsplit [-t] [inputFile] [chunkSize] [outputBase]\n");
  fprintf(stderr, "chunksize is bytes or kbytes (ex: 1440k), creates <outputBase>.000, <outputBase>.001, etc\n");
  fprintf(stderr, " -t means don't set the chunks' time stamp and modes as for original file\n");
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

static void
p_set_modes(char *ob, int p, struct stat *stbuf)
{
  char partname[FILENAME_MAX];
  struct utimbuf timbuf;

  sprintf(partname, "%s.%03d", ob, p);
  timbuf.actime = stbuf->st_atime;
  timbuf.modtime = stbuf->st_mtime;
  utime(partname, &timbuf);
  chown(partname, stbuf->st_uid, stbuf->st_gid);
  chmod(partname, stbuf->st_mode);
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
  struct stat stbuf;
  int preserve_file_time = 1;
  
  if (argc != 4 && argc != 5)
    usage();

  /* We used to call fstat, but that loses on NT, since the mode
     bits come as if the file were read-only, and the split files
     are then created read-only as well...  */
  if (stat (argv[1], &stbuf))
  {
    perror("Couldn't stat, file's time and modes won't be preserved");
    preserve_file_time = 0;
  }

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

  if (strcmp(argv[2], "-t") == 0)
  {
    preserve_file_time = 0;
    ++argv;
  }

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
      if (preserve_file_time)
	p_set_modes(argv[3], partnum, &stbuf);
      exit(0);
    }
    
    write(f, buf, r);
    left -= r;
    
    if (left == 0)
    {
      close(f);
      if (preserve_file_time)
	p_set_modes(argv[3], partnum, &stbuf);
      partnum++;
      f = p_open(argv[3], partnum);
      left = chunksize;
    }
  }
}
