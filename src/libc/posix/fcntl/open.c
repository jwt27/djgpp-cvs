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

int
open(const char* filename, int oflag, ...)
{
  int fd, dmode, bintext;

  /* Check this up front, to reduce cost and minimize effect */
  if ((oflag & (O_CREAT | O_EXCL)) == (O_CREAT | O_EXCL))
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

  fd = _open(filename, oflag & 0xff);	/* only low byte used */
  if (fd == -1 && oflag & O_CREAT)
    fd = _creat(filename, dmode);

  if (fd == -1)
    return fd;	/* errno already set by _open or _creat */

  if (oflag & O_TRUNC)
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
