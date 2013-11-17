/* Copyright (C) 2013 DJ Delorie, see COPYING.DJ for details */
/*
 * finitel(x) returns 1 is x is finite, else 0;
 * no branching!
 */

#include <math.h>
#include <libc/ieee.h>


#ifdef __STDC__
int
finitel(long double x)
#else
int
finitel(x)
long double x;
#endif
{
  _longdouble_union_t ieee754;


  ieee754.ld = x;
  return (int)((unsigned int)(ieee754.ldt.exponent - 0x7FFF) >> 31);
}
