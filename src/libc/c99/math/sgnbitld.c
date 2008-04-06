/* Copyright (C) 2008 DJ Delorie, see COPYING.DJ for details */
#include <math.h>
#include <libc/ieee.h>

int
__signbitld(long double x)
{
  _longdouble_union_t fp_value;

  fp_value.ld = x;
  return (int)fp_value.ldt.sign;
}
