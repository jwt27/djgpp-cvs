/* Copyright (C) 2013 DJ Delorie, see COPYING.DJ for details */

#include <math.h>


long int
lrint(double x)
{
  long int result;

  asm("fistpl %0" : "=m" (result) : "t" (x) : "st");

  return result;
}
