/* Copyright (C) 2013 DJ Delorie, see COPYING.DJ for details */

#include <math.h>


long long int
llrint(double x)
{
  long long int result;

  asm("fistpll %0" : "=m" (result) : "t" (x) : "st");

  return result;
}
