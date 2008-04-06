/* Copyright (C) 2008 DJ Delorie, see COPYING.DJ for details */
#include <math.h>
#include <libc/ieee.h>

int
__signbitd(double x)
{
  _double_union_t fp_value;

  fp_value.d = x;
  return (int)fp_value.dt.sign;
}
