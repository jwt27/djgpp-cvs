/* Copyright (C) 2005 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <string.h>

char *
strtok(char *s, const char *delim)
{
  static char *last;

  return strtok_r(s, delim, &last);
}
