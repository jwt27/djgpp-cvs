/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <io.h>

#include <libc/dosio.h>

/* Extra share flags that can be indicated by the user */
int __djgpp_share_flags;

int
open(const char* filename, int oflag, ...)
{
  int fd, dmode, bintext, dont_have_share;
  int should_create = (oflag & (O_CREAT | O_EXCL)) == (O_CREAT | O_EXCL);

  /* Check this up front, to reduce cost and minimize effect */
  if (should_create)
    if (__file_exists(filename))
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
    fd = _creatnew(filename, dmode, oflag & 0xff);
  else
  {
    fd = _open(filename, oflag);

    if (fd == -1)
    {
      /* It doesn't make sense to try anything else if there are no
	 more file handles available.  */
      if (errno == EMFILE || errno == ENFILE)
	return fd;

      if (__file_exists(filename))
      {
	/* Under multi-taskers, such as Windows, our file might be
	   open by some other program with DENY-NONE sharing bit,
	   which fails the `_open' call above.  Try again with
	   DENY-NONE bit set, unless some sharing bits were already
	   set in the initial call.  */
	if (dont_have_share)
	  fd = _open(filename, oflag | SH_DENYNO);
      }
      /* Don't call _creat on existing files for which _open fails,
         since the file could be truncated as a result.  */
      else if ((oflag & O_CREAT))
	fd = _creat(filename, dmode);
    }
  }

  if (fd == -1)
    return fd;	/* errno already set by _open or _creat */

  if ((oflag & O_TRUNC) && !should_create)
    if (_write(fd, 0, 0) < 0)
      return -1;

  /* we do this last because _open and _create set it also. */
  /* force setmode() to do ioctl() for cooked/raw */
  __file_handle_set(fd, bintext ^ (O_BINARY|O_TEXT));
  /* this will do cooked/raw ioctl() on character devices */
  setmode(fd, bintext);

  if(oflag & O_APPEND)
    lseek(fd, 0, SEEK_END);

  return fd;
}
