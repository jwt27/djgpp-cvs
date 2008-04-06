/* Copyright (C) 2008 DJ Delorie, see COPYING.DJ for details */
#include <math.h>
#include <libc/ieee.h>

int
__signbitf(float x)
{
  _float_union_t fp_value;

  fp_value.f = x;
  return (int)fp_value.ft.sign;
}
