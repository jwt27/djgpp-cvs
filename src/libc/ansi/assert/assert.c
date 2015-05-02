/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
#include <assert.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>

void	__dj_assert(const char *msg, const char *file, int line, const char *func)
{
  /* Assertion failed at foo.c line 45, function bar: x<y */
  fprintf(stderr, "Assertion failed at %s line %d", file, line);
  if(func)
    fprintf(stderr, ", function %s", func);
  fprintf(stderr, ": %s\n", msg);
  raise(SIGABRT);
  exit(1);
}
