/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
#include <math.h>

static int __math_errhandling = MATH_ERRNO;

int
__get_math_errhandling (void)
{
  return __math_errhandling;
}
