/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

static char *startup_djdir;

static void __attribute__((constructor))
init_confstr(void)
{
  char *djdir = getenv("DJDIR");
  if (djdir)
  {
    startup_djdir = malloc(strlen(djdir) + 1);
    if (startup_djdir)
      strcpy(startup_djdir, djdir);
  }
}

size_t
confstr(int name, char *buf, size_t len)
{
  size_t out_len = 0;

  switch (name)
  {
    case _CS_PATH:
    {
      if (startup_djdir)
      {
        out_len = snprintf(buf, len, "%s/bin", startup_djdir);
        /* snprintf excludes the null terminator from its return value,
           but confstr includes it.  */
        ++out_len;
      }
      break;
    }
    /* No options are required for the default 32-bit environment.  */
    case _CS_POSIX_V6_ILP32_OFF32_CFLAGS:
    case _CS_POSIX_V6_ILP32_OFF32_LDFLAGS:
    case _CS_POSIX_V6_ILP32_OFF32_LIBS:
    {
      if (buf && (len > 0))
	buf[0] = 0;
      /* confstr includes the null terminator in its return value. */
      ++out_len;
      break;
    }

    default:
    {
      errno = EINVAL;
    }
  }
  return out_len;
}

