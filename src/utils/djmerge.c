/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <utime.h>

#ifndef O_BINARY
#define O_BINARY 0
#endif

#define BUFS 16384

static void
usage(void)
{
  fprintf(stderr,"Usage: djmerge [-t] [inputBase] [outputFile]\n");
  fprintf(stderr, "reads <inputBase>.000, <inputBase>.001, etc\n");
  fprintf(stderr, " -t means don't set the file's time stamp and modes as for original file\n");
  exit(1);
}

static int
p_open(char *ob, int p, struct stat *stbuf)
{
  char partname[1024];
  sprintf(partname, "%s.%03d", ob, p);

  /* We used to call fstat, but that loses on NT, since the mode
     bits come as if the file were read-only, and the merged file
     is then created read-only as well...  */
  if (p == 0 && stat(partname, stbuf) != 0)
  {
    perror("Couldn't stat, file's time and modes won't be preserved");
    stbuf->st_ino = -1;
  }
  return open(partname, O_RDONLY|O_BINARY);
}

int
main(int argc, char **argv)
{
  char buf[BUFS];
  long r;
  int partnum;
  int outf, f;
  struct stat stbuf;
  int preserve_file_time = 1;
  
  if (argc != 3 && argc != 4)
    usage();

  if (strcmp(argv[1], "-t") == 0)
  {
    preserve_file_time = 0;
    ++argv;
  }

  outf = open(argv[2], O_WRONLY|O_CREAT|O_TRUNC|O_BINARY, 0666);
  if (outf < 0)
    usage();

  partnum = 0;
  f = p_open(argv[1], partnum, &stbuf);
  if (stbuf.st_ino == -1)
    preserve_file_time = 0;
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
      f = p_open(argv[1], partnum, NULL);

      if (f < 0)
      {
	struct utimbuf timbuf;

        close(outf);
	if (preserve_file_time)
	{
	  timbuf.actime = stbuf.st_atime;
	  timbuf.modtime = stbuf.st_mtime;
	  utime(argv[2], &timbuf);
	  chown(argv[2], stbuf.st_uid, stbuf.st_gid);
	  chmod(argv[2], stbuf.st_mode);
	}
        exit(0);
      }
    }
    
    write(outf, buf, r);
  }
}
