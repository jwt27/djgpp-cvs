/* Copyright (C) 2014 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2013 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2009 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2000 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <libc/bss.h>
#include <libc/symlink.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

static int mktemp_count = -1;

char *
mktemp(char *_template)
{
  int i, len, xcount;
  unsigned int use_lfn = _USE_LFN;

  for (i = 0; _template[i]; i++)
    ;
  for (xcount = 0, len = i; xcount < 6  && _template[--i] == 'X'; xcount++)
    ;
  if (use_lfn && xcount < 6)
  {
    errno = EINVAL;
    return NULL;
  }
  else
  {
    static int count = 0;

    /* Reinitialize counter if we were restarted (emacs).  */
    if (__bss_count != mktemp_count)
    {
      mktemp_count = __bss_count;
      count = 0;
    }

    if (xcount)
    {
      char *cp, *dp;
      char real_path[FILENAME_MAX];
      int loopcnt;

      dp = cp = _template + i;

      while (dp > _template && dp[-1] != '/' && dp[-1] != '\\' && dp[-1] != ':')
        dp--;

      if (!use_lfn)
      {
        /* Keep the first characters of the template,
           but turn the rest into Xs.  */
        while (cp > dp + 8 - xcount)
        {
          *--cp = 'X';
          xcount = (xcount >= 6) ? 6 : 1 + xcount;
        }

        /* If dots occur too early -- squash them.  */
        while (dp < cp)
        {
          if (*dp == '.') *dp = 'a';
          dp++;
        }

        /* Try to add ".tmp" to the filename.  Truncate unused Xs.  */
        if (cp + xcount + 3 < _template + len)
          strcpy (cp + xcount, ".tmp");
        else
          cp[xcount] = 0;

      }

      /* This loop can run up to 2<<(5*6) times, or about 10^9 times.  */
      for (loopcnt = 0; loopcnt < (1 << (5 * xcount)); loopcnt++)
      {
        int c = count++;
        for (i = 0; i < xcount; i++, c >>= 5)
          cp[i] = "abcdefghijklmnopqrstuvwxyz012345"[c & 0x1f];
        if (!__solve_symlinks(_template, real_path))
        {
          *_template = 0;
          return NULL;
        }
        if (!__file_exists(real_path))
          return _template;
      }
    }
  }

  /* Failure:  truncate the template and return NULL.  */
  *_template = 0;
  return NULL;
}



#ifdef TEST
int
main(void)
{
  char *s, *s0;

  s = strdup (s0 = "/usr/foo/dddddddXXXXXX");
  errno = 0;
  mktemp (s);
  printf ("%s -> %s  errno = %d\n", s0, s, errno);

  s = strdup (s0 = "/usr/foo/ddddddXXXXXX");
  errno = 0;
  mktemp (s);
  printf ("%s -> %s  errno = %d\n", s0, s, errno);

  s = strdup (s0 = "/usr/foo/dddddXXXXXX");
  errno = 0;
  mktemp (s);
  printf ("%s -> %s  errno = %d\n", s0, s, errno);

  s = strdup (s0 = "/usr/foo/ddddXXXXXX");
  errno = 0;
  mktemp (s);
  printf ("%s -> %s  errno = %d\n", s0, s, errno);

  s = strdup (s0 = "/usr/foo/dddXXXXXX");
  errno = 0;
  mktemp (s);
  printf ("%s -> %s  errno = %d\n", s0, s, errno);

  s = strdup (s0 = "/usr/foo/.dddXXXXXX");
  errno = 0;
  mktemp (s);
  printf ("%s -> %s  errno = %d\n", s0, s, errno);

  s = strdup (s0 = "/usr/foo/dddXXYXXX");
  errno = 0;
  mktemp (s);
  printf ("%s -> %s  errno = %d\n", s0, s, errno);

  s = strdup (s0 = "/usr/foo/XXXXXX");
  errno = 0;
  mktemp (s);
  printf ("%s -> %s  errno = %d\n", s0, s, errno);

  s = strdup (s0 = "/usr/foo/XXXXXXXX");
  errno = 0;
  mktemp (s);
  printf ("%s -> %s  errno = %d\n", s0, s, errno);

  return 0;
}
#endif
