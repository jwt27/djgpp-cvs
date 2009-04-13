/* Copyright (C) 2009 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2000 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <libc/bss.h>
#include <libc/symlink.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static int mktemp_count = -1;

char *
mktemp (char *_template)
{
  static int count = 0;
  char *cp, *dp;
  int i, len, xcount, loopcnt;
  char real_path[FILENAME_MAX];

  /* Reinitialize counter if we were restarted (emacs).  */
  if (__bss_count != mktemp_count)
    {
      mktemp_count = __bss_count;
      count = 0;
    }

  len = strlen (_template);
  cp = _template + len;

  xcount = 0;
  while (xcount < 6 && cp > _template && cp[-1] == 'X')
    xcount++, cp--;

  if (xcount) {
    dp = cp;
    while (dp > _template && dp[-1] != '/' && dp[-1] != '\\' && dp[-1] != ':')
      dp--;

    /* Keep the first characters of the template, but turn the rest into
       Xs.  */
    while (cp > dp + 8 - xcount) {
      *--cp = 'X';
      xcount = (xcount >= 6) ? 6 : 1 + xcount;
    }

    /* If dots occur too early -- squash them.  */
    while (dp < cp) {
      if (*dp == '.') *dp = 'a';
      dp++;
    }

    /* Try to add ".tmp" to the filename.  Truncate unused Xs.  */
    if (cp + xcount + 3 < _template + len)
      strcpy (cp + xcount, ".tmp");
    else
      cp[xcount] = 0;

    /* This loop can run up to 2<<(5*6) times, or about 10^9 times.  */
    for (loopcnt = 0; loopcnt < (1 << (5 * xcount)); loopcnt++) {
      int c = count++;
      for (i = 0; i < xcount; i++, c >>= 5)
	cp[i] = "abcdefghijklmnopqrstuvwxyz012345"[c & 0x1f];
      if (!__solve_symlinks(_template, real_path)) 
      {
         *_template = 0;
         return 0;
      }	
      if (!__file_exists(real_path))
	return _template;
    }
  }

  /* Failure:  truncate the template and return NULL.  */
  *_template = 0;
  return 0;
}



#ifdef TEST
int
main ()
{
  char *s, *s0;

  s = strdup (s0 = "/usr/foo/dddddddXXXXXX");
  mktemp (s);
  printf ("%s -> %s\n", s0, s);

  s = strdup (s0 = "/usr/foo/ddddddXXXXXX");
  mktemp (s);
  printf ("%s -> %s\n", s0, s);

  s = strdup (s0 = "/usr/foo/dddddXXXXXX");
  mktemp (s);
  printf ("%s -> %s\n", s0, s);

  s = strdup (s0 = "/usr/foo/ddddXXXXXX");
  mktemp (s);
  printf ("%s -> %s\n", s0, s);

  s = strdup (s0 = "/usr/foo/dddXXXXXX");
  mktemp (s);
  printf ("%s -> %s\n", s0, s);

  s = strdup (s0 = "/usr/foo/.dddXXXXXX");
  mktemp (s);
  printf ("%s -> %s\n", s0, s);

  s = strdup (s0 = "/usr/foo/dddXXYXXX");
  mktemp (s);
  printf ("%s -> %s\n", s0, s);

  s = strdup (s0 = "/usr/foo/XXXXXX");
  mktemp (s);
  printf ("%s -> %s\n", s0, s);

  s = strdup (s0 = "/usr/foo/XXXXXXXX");
  mktemp (s);
  printf ("%s -> %s\n", s0, s);

  return 0;
}
#endif

