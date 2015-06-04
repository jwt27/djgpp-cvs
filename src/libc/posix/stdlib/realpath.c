/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2010 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <limits.h>
#include <libc/symlink.h>
#include <errno.h>

/* Return the canonicalized form of the path in variable IN.  */
char *
realpath(const char *in, char *out)
{
  char in1[PATH_MAX];
  char *ret;

  if (in == NULL)
  {
    errno = EINVAL;
    return NULL;
  }

  if (in[0] == '\0')
  {
    errno = ENOENT;
    return NULL;
  }

  if (!__solve_symlinks(in, in1))
    return NULL; /* Return errno from from __solve_dir_symlinks().  */

  if (!(ret = out) && !(ret = malloc(PATH_MAX)))
  {
    errno = ENOMEM;
    return NULL;
  }

  if (__canonicalize_path(in1, ret, PATH_MAX) == NULL)
  {
    if (ret != out) free (ret);
    return NULL; /* Return errno from __canonicalize_path().  */
  }

  return ret;
}
