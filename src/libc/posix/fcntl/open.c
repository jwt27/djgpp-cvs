/* Copyright (C) 2014 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
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
#include <sys/fsext.h>
#include <dos.h>

#include <libc/dosio.h>
#include <libc/fd_props.h>

/* Extra share flags that can be indicated by the user */
int __djgpp_share_flags;

/* Move a file descriptor FD such that it is at least MIN_FD.
   If the file descriptor is changed (meaning it was origially
   *below* MIN_FD), the old one will be closed.
   If the operation failed (no more handles available?), -1 will
   be returned, in which case the original descriptor is still
   valid.

   This jewel is due to Morten Welinder <terra@diku.dk>.  */
static int
move_fd (int fd, int min_fd)
{
  int new_fd, tmp_fd;

  if (fd == -1 || fd >= min_fd)
    return fd;

  tmp_fd = dup (fd);
  if (tmp_fd == -1)
    return tmp_fd;

  new_fd = move_fd (tmp_fd, min_fd);
  if (new_fd != -1)
    close (fd);		/* success--get rid of the original descriptor */
  else
    close (tmp_fd);	/* failure--get rid of the temporary descriptor */
  return new_fd;
}

static int
opendir_as_fd (const char *filename, const int oflag)
{
  int fd, old_fd, flags, ret;

  /* Check the flags. */
  if ((oflag & (O_RDONLY|O_WRONLY|O_RDWR)) != O_RDONLY)
  {
    /* Only read-only access is allowed. */
    errno = EISDIR;
    return -1;
  }

  /*
   * Allocate a file descriptor that:
   *
   * - is dup'd off nul, so that bogus operations at least go somewhere
   *   sensible;
   * - is in binary mode;
   * - is non-inheritable;
   * - is marked as a directory.
   *
   * __FSEXT_alloc_fd() conveniently handles the first two. File handles
   * greater than 19 are not inherited, due to a misfeature
   * of the DOS exec call.
   */
  old_fd = __FSEXT_alloc_fd(NULL);
  if (old_fd < 0)
    return -1; /* Pass through errno. */

  fd = move_fd(old_fd, 20);
  if (fd < 0)
  {
    close(old_fd);
    errno = EMFILE;
    return -1;
  }

  __set_fd_properties(fd, filename, 0);
  __set_fd_flags(fd, FILE_DESC_DIRECTORY);

  flags = fcntl(fd, F_GETFD);
  if (flags < 0)
    return -1; /* Pass through errno. */
  flags |= FD_CLOEXEC;
  ret = fcntl(fd, F_SETFD, flags);
  if (ret < 0)
    return -1; /* Pass through errno. */

  return fd;
}

int
open(const char* filename, int oflag, ...)
{
  const int original_oflag = oflag;
  int fd, dmode, bintext, dont_have_share;
  char real_name[FILENAME_MAX + 1];
  int should_create = (oflag & (O_CREAT | O_EXCL)) == (O_CREAT | O_EXCL);
  int dirs_solved = 0; /* Only directories resolved in real_name? */

  /* Solve symlinks and honor O_NOLINK flag  */
  if (oflag & O_NOLINK)
  {
    if (!__solve_dir_symlinks(filename, real_name))
      return -1; /* errno from from __solve_dir_symlinks() */
    dirs_solved = 1;
  }
  else
  {
     if (!__solve_symlinks(filename, real_name))
        return -1; /* errno from from __solve_symlinks() */
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
      char  temp[FILENAME_MAX + 1];
      char  resolved[2];
      char *last_separator;
      int   old_errno = errno;
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
  {
    /* Symlink: We're not allowed to follow a symlink, when creating a file
     * with the same name as the symlink.
     */
    char temp_name[FILENAME_MAX + 1];
#define IRD_BUF_SIZE FILENAME_MAX + 1
    char ird_buf[IRD_BUF_SIZE];
    const size_t ird_bufsize = IRD_BUF_SIZE;
#undef IRD_BUF_SIZE

    if (!dirs_solved)
    {
      if (!__solve_dir_symlinks(filename, temp_name))
        return -1; /* errno from from __solve_dir_symlinks() */
    }
    else
      strcpy(temp_name, real_name);

    if (__internal_readlink(temp_name, 0, ird_buf, ird_bufsize) < 0)
    {
      /* If the error is something other than "doesn't exist"
       * or "isn't a symlink", return it.
       */
      if ((errno != ENOENT) && (errno != EINVAL))
	return -1; /* errno from __internal_readlink() */
    }
    else
    {
      /* It's a symlink. */
      errno = EEXIST;
      return -1;
    }

    /* Normal file */
    if (__file_exists(real_name))
    {
      /* file exists and we didn't want it to */
      errno = EEXIST;
      return -1;
    }
  }

  /* figure out what mode we're opening the file in */
  bintext = oflag & (O_TEXT | O_BINARY);
  if (!bintext)
    bintext = _fmode & (O_TEXT | O_BINARY);
  if (!bintext)
    bintext = O_BINARY;

  /* DOS doesn't want to see these bits */
  oflag &= ~(O_TEXT | O_BINARY);

  dmode = (*((&oflag) + 1) & S_IWUSR) ? 0 : 1;

  /* Merge the share flags if they are specified */
  dont_have_share = ((oflag &
                     (SH_DENYNO | SH_DENYRW | SH_DENYRD | SH_DENYWR)) == 0);
  if (dont_have_share && __djgpp_share_flags)
  {
    dont_have_share = 0;
    oflag |= __djgpp_share_flags;
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
      {
	fd = _creat(real_name, dmode);
        if (fd == -1 && _os_trueversion == 0x532)
        {
          /* For Windows 2000/XP, calling _creat will create one file
             more even if the file descriptors have already becoming exhausted.
             This means that the created file will exist in the directory
             but will have a file descriptor of -1 and thus becoming inaccessible.
             To avoid this it will be deleted here.
             The calling of _creat for the last possible file descriptor,
             this is the highst file descriptor that is possible,
             will already rise EMFILE.  */
          if (errno == EMFILE || errno == ENFILE)
          {
            int previous_errno = errno;
            remove(real_name);
            errno = previous_errno;
            return fd;
          }
        }
      }
    }
  }

  /* Is the target a directory? If so, generate a file descriptor
   * for the directory. Skip the rest of `open', because it does not
   * apply to directories. */
  if ((fd == -1) && (access(real_name, D_OK) == 0))
    return opendir_as_fd(real_name, original_oflag);

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
