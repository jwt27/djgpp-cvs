/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
#include <stdio.h>
#include <string.h>
#include <errno.h>

void
perror(const char *s)
{
  if (s && *s)
    fprintf(stderr, "%s: ", s);
  fprintf(stderr, "%s\n", strerror(errno));
}
