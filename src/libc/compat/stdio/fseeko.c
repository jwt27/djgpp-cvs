/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2007 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2006 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>

int
fseeko(FILE *_stream, off_t _offset, int _mode)
{
  return fseek(_stream, _offset, _mode);
}

