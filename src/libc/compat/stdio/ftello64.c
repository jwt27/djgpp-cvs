/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2007 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2006 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

off64_t
ftello64(FILE *_stream)
{
  /* FIXME ??? */
  int r;
  r = fflush(_stream);
  if (r != 0)
    return -1;
  return llseek(fileno(_stream), 0, SEEK_CUR);
}

