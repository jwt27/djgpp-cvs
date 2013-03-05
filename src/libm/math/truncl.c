/* Copyright (C) 2013 DJ Delorie, see COPYING.DJ for details */

#include <math.h>
#include <libc/ieee.h>

#define LONG_DOUBLE_BIAS                 (0x3FFFU)
#define MAX_BIN_EXPONENT                 (16383)
#define MIN_BIN_EXPONENT                 (-16382)
#define BIN_DIGITS_IN_FRACTION           (63)  /*  Amount of binary digits in fraction part of mantissa.  */
#define BIN_DIGITS_IN_MSW                (31)  /*  Amount of binary digits in msw of the fraction part of mantissa.  */
#define NO_SIGNIFICANT_DIGITS_IN_LSW(x)  ((x) < BIN_DIGITS_IN_MSW)
#define IS_INF_OR_NAN(x)                 ((x) > MAX_BIN_EXPONENT)
#define MAGNITUDE_IS_LESS_THAN_ONE(x)    ((x) < 0)


#ifdef __STDC__
long double truncl(long double x)
#else
long double truncl(x)
long double x;
#endif
{
  _longdouble_union_t ieee_value;
  int unbiased_exponent;


  ieee_value.ld = x;
  unbiased_exponent = ieee_value.ldt.exponent - LONG_DOUBLE_BIAS;

  if (NO_SIGNIFICANT_DIGITS_IN_LSW(unbiased_exponent))
  {
    /* All significant digits are in msw. */
    if (MAGNITUDE_IS_LESS_THAN_ONE(unbiased_exponent))
    {
      /* The magnitude of the number is < 1 so the result is +-0.  */
      int sign = ieee_value.ldt.sign;
      ieee_value.ld = 0;
      ieee_value.ldt.sign = sign;
    }
    else
    {
      ieee_value.ldt.mantissah &= ~(0x7FFFFFFFU >> unbiased_exponent);
      ieee_value.ldt.mantissal = 0;
    }

    return ieee_value.ld;
  }
  else if (unbiased_exponent > BIN_DIGITS_IN_FRACTION)
  {
    if (IS_INF_OR_NAN(unbiased_exponent))
      return x + x;  /* Trigger an exception.  */
  }
  else
  {
    /*
     *  All fraction bits in msw are relevant.
     *  Truncate irrelevant bits from lsw.
     */
    ieee_value.ldt.mantissal &= ~(0xFFFFFFFFU >> (unbiased_exponent - BIN_DIGITS_IN_MSW));
    return ieee_value.ld;
  }


  /*
   *  All bits in the fraction fields of the msw
   *  and lsw are needed in the result.
   */
  return x;
}
