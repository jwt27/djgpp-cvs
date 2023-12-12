/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdint.h>
#include <libc/stubs.h>
#include <math.h>

long double modfl(long double _x, long double *_pint)
{
  return __modfl(_x, _pint);
}