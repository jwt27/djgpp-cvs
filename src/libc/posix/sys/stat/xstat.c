/* Copyright (C) 2012 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/*
 * This is file XSTAT.C
 *
 * Internal assist functions which are common to stat(), fstat() and lstat().
 *
 *
 * Copyright (c) 1994-96 Eli Zaretskii <eliz@is.elta.co.il>
 *
 * This software may be used freely as long as the above copyright
 * notice is left intact.  There is no warranty on this software.
 *
 */

#include <libc/stubs.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <limits.h>
#include <time.h>
#include <errno.h>
#include <sys/vfs.h>
#include <dos.h>
#include <dpmi.h>
#include <libc/farptrgs.h>
#include <libc/dosio.h>
#include <libc/bss.h>

#include "xstat.h"

static int xstat_count = -1;

/* ----------------------------------------------------------------------- */

/* Convert file date and time to time_t value suitable for
   struct stat fields.  */

time_t
_file_time_stamp(unsigned int dos_ftime)
{
  struct tm file_tm;

  memset(&file_tm, 0, sizeof(struct tm));
  file_tm.tm_isdst = -1;    /* let mktime() determine if DST is in effect */

  file_tm.tm_sec  = (dos_ftime & 0x1f) * 2;
  file_tm.tm_min  = (dos_ftime >>  5) & 0x3f;
  file_tm.tm_hour = (dos_ftime >> 11) & 0x1f;
  file_tm.tm_mday = (dos_ftime >> 16) & 0x1f;
  file_tm.tm_mon  = ((dos_ftime >> 21) & 0x0f) - 1; /* 0 = January */
  file_tm.tm_year = (dos_ftime >> 25) + 80;

  return mktime(&file_tm);
}

/* Get time stamp of a DOS file packed as a 32-bit int.
 * This does what Borland's getftime() does, except it doesn't
 * pollute the application namespace and returns an int instead
 * of struct ftime with packed bit-fields.
 */


int
_getftime(int fhandle, unsigned int *dos_ftime)
{
  __dpmi_regs regs;

  regs.x.ax = 0x5700;
  regs.x.bx = fhandle;
  __dpmi_int(0x21, &regs);

  if (regs.x.flags & 1)
  {
    errno = __doserr_to_errno(regs.x.ax);
    return -1;
  }

  *dos_ftime = ((unsigned int)regs.x.dx << 16) + (unsigned int)regs.x.cx;

  return 0;
}

/* Cache the cluster size (aka block size) for each drive letter, so we can
 * populate the st_blksize of struct stat easily. The cluster size is
 * measured in bytes.
 *
 * ASSUMPTION: path has already been fixed by `_fixpath'.
 */

/* Comment copied from DJGPP 2.03's src/libc/compat/mntent/mntent.c:
 *
 * There may be a maximum of 32 block devices.  Novell Netware indeed
 * allows for 32 disks (A-Z plus 6 more characters from '[' to '\'')
 */
static blksize_t cache_blksize[32];
static int cache_blksize_count = -1;

blksize_t
_get_cached_blksize (const char *path)
{
  struct statfs sbuf;
  int           d; /* index into drive cache = drive_num - 1 */  
  static int    overmax_d = sizeof(cache_blksize) / sizeof(cache_blksize[0]);

  /* Force initialization in restarted programs (emacs).  */
  if (cache_blksize_count != __bss_count)
  {
    cache_blksize_count = __bss_count;
    memset(cache_blksize, 0, sizeof(cache_blksize));

    /* Default floppy drives to 512B block size, to improve performance. */
    cache_blksize[0] = cache_blksize[1] = 512;
  }

  /* Get the drive number. The fixed filename will begin with a lowercase
   * letter or a symbol. The symbols for drives > 'z:' occur straight
   * after 'Z' in ASCII.
   */
  if ((path[0] >= 'a') && (path[0] <= 'z'))
    d = path[0] - 'a';
  else
    d = path[0] - 'A';

  if ((d < 0) || (d >= overmax_d))
  {
    errno = ENODEV;
    return -1;
  }

  if (!cache_blksize[d])
  {
    if (_is_remote_drive(d))  /* A = 0, B = 1, C = 2,  etc. */
    {
      /* Default remote drives to 4K block size, to improve performance.
       *
       * Also the size returned by statfs() may not be correct. Testing
       * against files shared by Samba 2.0.10 on Linux kernel 2.2.19
       * returned a 32K block size, even though the ext2 filesystem
       * holding the share share had a 4K block size. */
      cache_blksize[d] = 4096;
    }
    else
    {
      /* No entry => retrieve cluster size */
      if (statfs(path, &sbuf) != 0)
      {
        /* Failed, pass error through */
        return -1;
      }

      cache_blksize[d] = sbuf.f_bsize;
    }
  }

  return cache_blksize[d];
}

/* Invent an inode number for those files which don't have valid DOS
 * cluster number.  These could be: devices like /dev/nul; empty
 * files which were not allocated disk space yet; or files on
 * networked drives, for which the redirector doesn't bring the
 * cluster number.
 *
 * To ensure proper operation of this function, you must call it
 * with a filename in some canonical form.  E.g., with a name
 * returned by truename(), or that returned by _fixpath().  The
 * point here is that the entire program MUST abide by these
 * conventions through its operation, or else you risk getting
 * different inode numbers for the same file.
 *
 * This function is due to Eric Backus and was taken with minor
 * modifications from stat.c, as included in DJGPP 1.11m5.
 * The function now returns 0 instead of -1 if it can't allocate
 * memory for a new name, so that f?stat() won't fail if the inode
 * is unavailable, but return zero inode instead.
 */

/*
  (c) Copyright 1992 Eric Backus

  This software may be used freely so long as this copyright notice is
  left intact.  There is no warranty on this software.
*/

struct name_list
{
  struct name_list *next;
  char             *name;
  unsigned          mtime;
  unsigned long     size;
  long              inode;
};

ino_t
_invent_inode(const char *name, unsigned time_stamp, unsigned long fsize)
{
  static struct name_list  *name_list[256];

  /* We count upwards from 2^28+1, which can't yield two files with
   * identical inode numbers: FAT16 uses maximum ~2^16 and FAT32 uses
   * maximum ~2^28. 
   */
#define INVENTED_INODE_START ( (1 << 28) + 1 )

  /* INODE_COUNT is declared LONG and not ino_t, because some DOS-based
   * compilers use short or unsigned short for ino_t.
   */
  static long        inode_count = INVENTED_INODE_START;

  struct name_list  *name_ptr, *prev_ptr;
  const char        *p;
  int                hash;

  /* Force initialization in restarted programs (emacs).  */
  if (xstat_count != __bss_count)
  {
    xstat_count = __bss_count;
    inode_count = INVENTED_INODE_START;
    memset (name_list, 0, sizeof name_list);
  }

  if (!name)
    return 0;

  /* Skip over device and leading slash.  This will allow for less
   * inode numbers to be used, because there is nothing bad in generating
   * identical inode for two files which belong to different drives.
   */
  if (*name && name[1] == ':' && (name[2] == '\\' || name[2] == '/'))
  {
    /* If this is a root directory, return inode = 1.  This is compatible
       with the code on stat.c which deals with root directories. */
    if (name[3] == 0)
      return (ino_t)1;

    name += 3;
  }

  /* If the passed name is empty, invent a new inode unconditionally.
   * This is for those unfortunate circumstances where we couldn't
   * get a name (e.g., fstat() under Novell).  For these we want at
   * least to ensure that no two calls will get the same inode number.
   * The lossage here is that you get different inodes even if you call
   * twice with the same file.  Sigh...
   */
  if (!*name)
  {
    ino_t retval = inode_count;

    inode_count++;
    return retval;
  }

  /* We could probably use a better hash than this */
  p = name;
  hash = 0;
  while (*p != '\0')
    hash += *p++;
  hash &= 0xff;

  /* Have we seen this string? */
  name_ptr = name_list[hash];
  prev_ptr = name_ptr;
  while (name_ptr)
  {
    if (strcmp(name, name_ptr->name) == 0 &&
        name_ptr->mtime == time_stamp &&
        name_ptr->size  == fsize)
      break;
    prev_ptr = name_ptr;
    name_ptr = name_ptr->next;
  }

  if (name_ptr)
    /* Same string, time stamp, and size, so same inode */
    return name_ptr->inode;
  else
  {
    ino_t retval;

    /* New string with same hash code */
    name_ptr = (struct name_list *)malloc(sizeof *name_ptr);
    if (name_ptr == 0)
      return 0;
    name_ptr->next = (struct name_list *)0;
    name_ptr->name = (char *)malloc(strlen(name) + 1);
    if (name_ptr->name == 0)
    {
      free(name_ptr);
      return 0;
    }
    strcpy(name_ptr->name, name);
    name_ptr->mtime = time_stamp;
    name_ptr->size = fsize;
    name_ptr->inode = inode_count;
    if (prev_ptr)
      prev_ptr->next = name_ptr;
    else
      name_list[hash] = name_ptr;
    retval = inode_count;
    inode_count++; /* Increment for next call. */

    return retval;
  }
}
