/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
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
  int o = __tb+offset;
  int space = _go32_info_block.size_of_transfer_buffer - offset;
  const char *p = path;

  if (path == 0)
  {
    errno = EFAULT;
    abort();
  }

  _farsetsel(_dos_ds);

  if (p[0] && p[1] == ':')
    p += 2;
  if (strncmp(p, "/dev/", 5) == 0)
  {
    if (strcmp(p+5, "null") == 0)
      path = "nul";
    else if (strcmp(p+5, "tty") == 0)
      path = "con";
    else if (((p[5] >= 'a' && p[5] <= 'z')
	      || (p[5] >= 'A' && p[5] <= 'Z'))
	     && (p[6] == '/' || p[6] == '\\' || p[6] == '\0'))
    {
      /* map /dev/a/ to a:/ */
      _farnspokeb(o++, p[5]);
      _farnspokeb(o++, ':');
      path = p + 6;
      space -= 2;
    }
    else if (p[5])
      path = p + 5;
  }

  /* collapse multiple slashes to a single slash */
  for (; *path; path++)
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
