/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2000 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <libc/symlink.h>
#include <libc/unconst.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <io.h>

#include <libc/dosio.h>
#include <libc/fd_props.h>

/* Extra share flags that can be indicated by the user */
int __djgpp_share_flags;

int
open(const char* filename, int oflag, ...)
{
  int fd, dmode, bintext, dont_have_share;
  char real_name[FILENAME_MAX + 1];
  int should_create = (oflag & (O_CREAT | O_EXCL)) == (O_CREAT | O_EXCL);

  /* Solve symlinks and honor O_NOLINK flag  */
  if (oflag & O_NOLINK)
  {
     if (!__solve_dir_symlinks(filename, real_name))
        return -1; /* errno from from __solve_dir_symlinks() */
  }
  else
  {
     if (!__solve_symlinks(filename, real_name))
        return -1; 
  }

  /* Honor O_NOFOLLOW flag. */
  if (oflag & O_NOFOLLOW)
  {
      /* O_NOFOLLOW, as defined in glibc, requires open() to fail if the
       * last path component is a symlink.  However, it still requires to 
       * resolve all other path components.
       * We check if there were any symlinks by comparing __solve_symlinks()
       * input and output.  That function does not perform any path 
       * canonicalization so it should be safe.  */
      if (strcmp(filename, real_name))
      {
         /* Yes, there were symlinks in the path.  Now take all but the last
          * path component from `real_name', add last path component from
          * `filename', and try to resolve that mess. 
          */
         char   temp[FILENAME_MAX + 1];
         char   resolved[2];
         char * last_separator;
         int    old_errno = errno;
         strcpy(temp, real_name);
         last_separator = basename(temp);
         *last_separator = '\0';
         last_separator = basename(filename);
         strcat(temp, "/");
         strcat(temp, last_separator);
         if ((readlink(temp, resolved, 1) != -1) || (errno != EINVAL))
         {
            /* Yes, the last path component was a symlink. */
            errno = ELOOP;
            return -1;
         }
         errno = old_errno;
      }
  }

  /* Check this up front, to reduce cost and minimize effect */
  if (should_create)
    if (__file_exists(real_name))
    {
      /* file exists and we didn't want it to */
      errno = EEXIST;
      return -1;
    }

  /* figure out what mode we're opening the file in */
  bintext = oflag & (O_TEXT | O_BINARY);
  if (!bintext)
    bintext = _fmode & (O_TEXT | O_BINARY);
  if (!bintext)
    bintext = O_BINARY;

  /* DOS doesn't want to see these bits */
  oflag &= ~(O_TEXT | O_BINARY);

  dmode = (*((&oflag)+1) & S_IWUSR) ? 0 : 1;

  /* Merge the share flags if they are specified */
  dont_have_share = ((oflag &
                     (SH_DENYNO | SH_DENYRW | SH_DENYRD | SH_DENYWR)) == 0);
  if (dont_have_share && __djgpp_share_flags)
    {
     dont_have_share=0;
     oflag|=__djgpp_share_flags;
    }

  if (should_create)
    fd = _creatnew(real_name, dmode, oflag & 0xff);
  else
  {
    fd = _open(real_name, oflag);

    if (fd == -1)
    {
      /* It doesn't make sense to try anything else if there are no
	 more file handles available.  */
      if (errno == EMFILE || errno == ENFILE)
	return fd;

      if (__file_exists(real_name))
      {
	/* Under multi-taskers, such as Windows, our file might be
	   open by some other program with DENY-NONE sharing bit,
	   which fails the `_open' call above.  Try again with
	   DENY-NONE bit set, unless some sharing bits were already
	   set in the initial call.  */
	if (dont_have_share)
	  fd = _open(real_name, oflag | SH_DENYNO);
      }
      /* Don't call _creat on existing files for which _open fails,
         since the file could be truncated as a result.  */
      else if ((oflag & O_CREAT))
	fd = _creat(real_name, dmode);
    }
  }

  if (fd == -1)
    return fd;	/* errno already set by _open or _creat */

  if ((oflag & O_TRUNC) && !should_create)
#ifndef TRUNC_CHECK
    _write(fd, 0, 0);
#else
    /* Windows 2000/XP will fail 0 byte writes (truncate) on a character
       device (nul, con) if opened with lfn calls.  We can either ignore
       the return completely or ignore errors on NT.  Since a truncate
       fail should never happen (and if it does we expect an error on
       the next write) this probably doesn't make much difference. */

    if (_write(fd, 0, 0) < 0 && _os_trueversion != 0x532)
    {
      _close(fd);
      return -1;
    }
#endif

  /* we do this last because _open and _create set it also. */
  /* force setmode() to do ioctl() for cooked/raw */
  __file_handle_set(fd, bintext ^ (O_BINARY|O_TEXT));
  /* this will do cooked/raw ioctl() on character devices */
  setmode(fd, bintext);
  __set_fd_properties(fd, real_name, oflag);

  if ( oflag & O_APPEND )
  {
    llseek(fd, 0, SEEK_END);
  }

  return fd;
}

