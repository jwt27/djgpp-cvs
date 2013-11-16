/* Copyright (C) 2013 DJ Delorie, see COPYING.DJ for details */
/*
 * isnanl(x) returns 1 is x is nan, else 0;
 * no branching!
 */

#include <math.h>
#include <libc/ieee.h>


#ifdef __STDC__
int
isnanl(long double x)
#else
int
isnanl(x)
long double x;
#endif
{
  _longdouble_union_t ieee754;


  ieee754.ld = x;
  ieee754.ldt.mantissal |= ieee754.ldt.mantissah ^ 0x80000000;
  ieee754.ldt.mantissah = (((unsigned int)ieee754.ldt.exponent) << 1) | ((-ieee754.ldt.mantissal | ieee754.ldt.mantissal) >> 31);
  return (int)((0xFFFE - ieee754.ldt.mantissah) >> 31);
}
