/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */

#include <libc/stubs.h>
#include <sys/types.h>
#include <sys/vfs.h>
#include <sys/statvfs.h>
#include <dos.h>
#include <unistd.h>

int
statvfs (const char *path, struct statvfs *outbuf)
{
  struct statfs sbuf;
  long len;

  if (statfs(path, &sbuf) != 0)
    {
      /* Pass through the error from statfs(). */
      return -1;
    }

  /* Fill outbuf from sbuf. */
  outbuf->f_bsize  = sbuf.f_bsize;  
  outbuf->f_blocks = sbuf.f_blocks;
  outbuf->f_bfree  = sbuf.f_bfree;
  outbuf->f_bavail = sbuf.f_bavail;
  outbuf->f_fsid   = sbuf.f_fsid[0];

  /* The `file serial number' fields can be considered to be
   * numbers of inodes. Since an inode and block are roughly equivalent,
   * fill these in with the numbers of blocks. */
  outbuf->f_files  = sbuf.f_blocks;
  outbuf->f_ffree  = sbuf.f_bfree;
  outbuf->f_favail = sbuf.f_bavail;

  /* We pretend that the fundamental block size `f_frsize' is the same
   * as the file system's block size ("clusters" for FAT file systems).
   */
  outbuf->f_frsize = outbuf->f_bsize;

  /* Set the flags. */
  outbuf->f_flag = ST_NOSUID;
  if (_is_cdrom_drive(outbuf->f_fsid))
    outbuf->f_flag |= ST_RDONLY;

  /* Find the maximum file name length. */
  len = pathconf(path, _PC_NAME_MAX);
  if (len < 0)
    {
      /* Pass through the error from pathconf(). */
      return -1;
    }

  outbuf->f_namemax = len;

  return 0;
}

#ifdef TEST

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int
main (int argc, char *argv[])
{
  const char NOSUID[] = "nosuid";
  const char RDONLY[] = "rdonly";  
  struct statvfs svbuf;
  char buf[64];
  int i;

  if (argc < 2)
    {
      fprintf(stderr, "Syntax: %s <path> [<path> ...]\n", argv[0]);
      return EXIT_FAILURE;
    }

  for (i = 1; i < argc; i++)
    {
      if (statvfs(argv[i], &svbuf) < 0)
	{
	  perror(argv[0]);
	  return EXIT_FAILURE;
	}

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
