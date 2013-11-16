/* Copyright (C) 2013 DJ Delorie, see COPYING.DJ for details */
/*
 * isinfl(x) returns 1 if x is infinity, else 0;
 * no branching!
 */

#include <math.h>
#include <libc/ieee.h>


#ifdef __STDC__
int
isinfl(long double x)
#else
int
isinfl(x)
long double x;
#endif
{
  _longdouble_union_t ieee754;


  ieee754.ld = x;
  ieee754.ldt.mantissal |= (ieee754.ldt.mantissah ^ 0x80000000) | (ieee754.ldt.exponent ^ 0x7FFF);
  return (int)(1 - ((-ieee754.ldt.mantissal | ieee754.ldt.mantissal) >> 31));
}
