/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>

long
fpathconf(int fildes, int name)
{
  static char root_path[] = "/";
  struct stat st_buf;
  char fs_root[4];
  char *p = root_path;

  /* `fstat' returns non-negative `st_dev' for regular disk files.  */
  if (fstat(fildes, &st_buf) == 0 && st_buf.st_dev >= 0)
  {
    fs_root[0] = 'A' + st_buf.st_dev;
    strcpy(fs_root + 1, ":\\");
    p = fs_root;
  }
  return pathconf(p, name);
}
