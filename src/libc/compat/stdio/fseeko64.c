/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2007 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2006 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

int
fseeko64(FILE *_stream, off64_t _offset, int _mode)
{
  /* FIXME ??? */
  int r;
  offset_t o;
  r = fflush(_stream);
  if (r != 0)
    return -1;
  o = llseek(fileno(_stream), _offset, _mode);
  if (o == -1)
    return -1;
  return 0;
}

