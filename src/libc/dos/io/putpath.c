/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/dosio.h>
#include <libc/farptrgs.h>
#include <go32.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

void
_put_path(const char *path)
{
  _put_path2(path, 0);
}

void
_put_path2(const char *path, int offset)
{
  int o, space = _go32_info_block.size_of_transfer_buffer - offset;

  if (path == 0)
  {
    errno = EFAULT;
    abort();
  }

  if (strcmp(path, "/dev/null") == 0)
    path = "nul";
  if (strcmp(path, "/dev/tty") == 0)
    path = "con";

  _farsetsel(_dos_ds);

  /* collapse multiple slashes to a single slash */
  for (o=__tb+offset; *path; path++)
  {
    if (path[0] != '/' || path[1] != '/')
    {
      _farnspokeb(o, *path);
      o++;
      if (--space < 2) /* safety check */
	break;
    }
  }

  /* remove trailing slash if it doesn't
     represent the root directory */
  if (o-2 >= __tb+offset
      && _farnspeekb(o-1) == '/'
      && _farnspeekb(o-2) != ':')
    o--;

  /* null terminate it */
  _farnspokeb(o, 0);
}
