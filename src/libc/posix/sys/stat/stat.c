/* Copyright (C) 2000 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */

/* Main entry point.  This is library stat() function.
   Actual code has been moved to lstat() in lstat.c.
 */

#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>

#ifdef TEST
#include "xstat.h"
#endif

int
stat(const char *path, struct stat *statbuf)
{
   return lstat(path, statbuf);
}

#ifdef  TEST

unsigned short _djstat_flags = 0;

void
main(int argc, char *argv[])
{
  struct stat stat_buf;
  char *endp;

  if (argc < 2)
    {
      fprintf (stderr, "Usage: %s <_djstat_flags> <file...>\n", argv[0]);
      exit(0);
    }

  if (stat(*argv, &stat_buf) != 0)
    perror ("stat failed on argv[0]");
  else
    fprintf(stderr, "DOS %d.%d (%s)\n", _osmajor, _osminor, _os_flavor);
  argc--; argv++;

  _djstat_flags = (unsigned short)strtoul(*argv, &endp, 0);
  argc--; argv++;

  while (argc--)
    {
      if (!stat(*argv, &stat_buf))
        {
          fprintf(stderr, "%s: %d %6u %o %d %d %ld %lu %s", *argv,
                  stat_buf.st_dev,
                  (unsigned)stat_buf.st_ino,
                  stat_buf.st_mode,
                  stat_buf.st_nlink,
                  stat_buf.st_uid,
                  (long)stat_buf.st_size,
                  (unsigned long)stat_buf.st_mtime,
                  ctime(&stat_buf.st_mtime));
          _djstat_describe_lossage(stderr);
        }
      else
        {
          fprintf(stderr, "%s: lossage", *argv);
          perror(" ");
          _djstat_describe_lossage(stderr);
        }

      ++argv;
    }

    exit (0);
}

#endif
