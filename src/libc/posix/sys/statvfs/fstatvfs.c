/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */

#include <libc/stubs.h>
#include <libc/fd_props.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/statvfs.h>

int
fstatvfs (int fd, struct statvfs *outbuf)
{
  const char *path = NULL;

  if (__has_fd_properties(fd))
    path = __get_fd_name(fd);

  if (!path)
    {
      /* We can't find the path for `fd'. */
      errno = ENOENT;
      return -1;
    }

  return statvfs(path, outbuf);
}

#ifdef TEST

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

int
main (int argc, char *argv[])
{
  const char NOSUID[] = "nosuid";
  const char RDONLY[] = "rdonly";  
  struct statvfs svbuf;
  char buf[64];
  int fd, i;

  if (argc < 2)
    {
      fprintf(stderr, "Syntax: %s <path> [<path> ...]\n", argv[0]);
      return EXIT_FAILURE;
    }

  for (i = 1; i < argc; i++)
    {
      fd = open(argv[i], O_RDONLY);
      if (fd < 0)
	{
	  perror(argv[0]);
	  return EXIT_FAILURE;
	}

      if (fstatvfs(fd, &svbuf) < 0)
	{
	  perror(argv[0]);
	  return EXIT_FAILURE;
	}

      close(fd);

      *buf = '\0';
      if (svbuf.f_flag & ST_NOSUID)
	strcat(buf, NOSUID);
      if (svbuf.f_flag & ST_RDONLY)
	{
	  if (svbuf.f_flag & ST_NOSUID)
	    strcat(buf, ",");
	  strcat(buf, RDONLY);
	}

      printf("%s:\n"
	     "\tBlock size:                                       %lu\n"
	     "\tFundamental block size:                           %lu\n"
	     "\tBlocks on filesystem:                             %lu\n"
	     "\tFree blocks on filesystem:                        %lu\n"
	     "\tFree blocks on filesystem for unprivileged users: %lu\n"
	     "\tFile serial numbers:                              %lu\n"
	     "\tFree file serial numbers:                         %lu\n"
	     "\tFree file serial numbers for unprivileged users:  %lu\n"
	     "\tFile system ID:                                   %lu\n"
	     "\tFile system flags:                                %s\n"
	     "\tMaximum file name length:                         %lu\n\n",
	     argv[i],
	     svbuf.f_bsize, svbuf.f_frsize,
	     svbuf.f_blocks, svbuf.f_bfree, svbuf.f_bavail,
	     svbuf.f_files, svbuf.f_ffree, svbuf.f_favail,
	     svbuf.f_fsid, buf, svbuf.f_namemax);
    }

  return EXIT_SUCCESS;
}

#endif /* TEST */
