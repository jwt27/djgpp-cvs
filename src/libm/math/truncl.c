/* Copyright (C) 2013 DJ Delorie, see COPYING.DJ for details */

#include <math.h>
#include <libc/ieee.h>

#define LONG_DOUBLE_BIAS                         (0x3FFFU)
#define MAX_BIN_EXPONENT                         (16383)
#define MIN_BIN_EXPONENT                         (-16382)
#define BIN_DIGITS_IN_FRACTION                   (63)  /*  Amount of binary digits in fraction part of mantissa.  */
#define BIN_DIGITS_IN_MANTISSAH                  (31)  /*  Amount of binary digits in msw of the fraction part of mantissa.  */
#define FRACTION_HIGH_PART_MASK                  (0x7FFFFFFFUL)
#define FRACTION_LOW_PART_MASK                   (0xFFFFFFFFUL)
#define ALL_DIGITS_ARE_SIGNIFICANT(exp)          ((exp) > BIN_DIGITS_IN_FRACTION)
#define NO_SIGNIFICANT_DIGITS_IN_MANTISSAL(exp)  ((exp) < BIN_DIGITS_IN_MANTISSAH)
#define MAGNITUDE_IS_LESS_THAN_ONE(exp)          ((exp) < 0)
#define IS_INF_OR_NAN(exp)                       ((exp) > MAX_BIN_EXPONENT)


#ifdef __STDC__
long double
truncl(long double x)
#else
long double
truncl(x)
long double x;
#endif
{
  _longdouble_union_t ieee_value;
  int unbiased_exponent;


  ieee_value.ld = x;
  unbiased_exponent = ieee_value.ldt.exponent - LONG_DOUBLE_BIAS;

  if (ALL_DIGITS_ARE_SIGNIFICANT(unbiased_exponent))        /* Trigger an exception if INF or NAN  */
    return IS_INF_OR_NAN(unbiased_exponent) ? x + x : x;    /* else return the number.  */
  else if (NO_SIGNIFICANT_DIGITS_IN_MANTISSAL(unbiased_exponent))
  {
    /* All significant digits are in msw. */
    if (MAGNITUDE_IS_LESS_THAN_ONE(unbiased_exponent))
    {
      /* The magnitude of the number is < 1 so the result is +-0.  */
      const int sign = ieee_value.ldt.sign;
      ieee_value.ld = 0;
      ieee_value.ldt.sign = sign;
    }
    else
    {
      ieee_value.ldt.mantissah &= ~(FRACTION_HIGH_PART_MASK >> unbiased_exponent);
      ieee_value.ldt.mantissal = 0;
    }

    return ieee_value.ld;
  }
  else
  {
    /*
     *  All fraction bits in msw are relevant.
     *  Truncate irrelevant bits from lsw.
     */
    ieee_value.ldt.mantissal &= ~(FRACTION_LOW_PART_MASK >> (unbiased_exponent - BIN_DIGITS_IN_MANTISSAH));
    return ieee_value.ld;
  }
}
