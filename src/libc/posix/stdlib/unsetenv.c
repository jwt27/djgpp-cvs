/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2004 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */

#include <libc/stubs.h>
#include <libc/unconst.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

extern char **_environ;

int
unsetenv(const char *name)
{
  /* No environment == success */
  if (_environ == 0)
    return 0;

  /* Check for the failure conditions */
  if (name == NULL || *name == '\0' || strchr (name, '=') != NULL)
  {
    errno = EINVAL;
    return -1;
  }

  /* Let putenv() do the work
   * Note that we can just pass name directly as our putenv() treats
   * this the same as passing 'name='. */
  /* The cast is needed because POSIX specifies putenv() to take a non-const
   * parameter.  Our putenv is well-behaved, so this is OK.  */
  putenv (unconst (name, char*));

  return 0;
}
