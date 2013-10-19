/* Copyright (C) 2013 DJ Delorie, see COPYING.DJ for details */

#include <math.h>
#include <libc/ieee.h>

#define FLOAT_BIAS                       (0x7FU)
#define MAX_BIN_EXPONENT                 (127)
#define MIN_BIN_EXPONENT                 (-126)
#define BIN_DIGITS_IN_FRACTION           (23)    /*  Amount of binary digits in fraction part of mantissa.  */
#define FRACTION_MASK                    (0x7FFFFFU)
#define ALL_DIGITS_ARE_SIGNIFICANT(exp)  ((exp) > (BIN_DIGITS_IN_FRACTION - 1))
#define MAGNITUDE_IS_LESS_THAN_ONE(exp)  ((exp) < 0)
#define IS_INF_OR_NAN(exp)               ((exp) > MAX_BIN_EXPONENT)


#ifdef __STDC__
float
truncf(float x)
#else
float
truncf(x)
float x;
#endif
{
  _float_union_t ieee_value;
  int unbiased_exponent;


  ieee_value.f = x;
  unbiased_exponent = ieee_value.ft.exponent - FLOAT_BIAS;

  if (ALL_DIGITS_ARE_SIGNIFICANT(unbiased_exponent))        /* Trigger an exception if INF or NAN  */
    return IS_INF_OR_NAN(unbiased_exponent) ? x + x : x;    /* else return the number.  */
  else
  {
    if (MAGNITUDE_IS_LESS_THAN_ONE(unbiased_exponent))
    {
      /* The magnitude of the number is < 1 so the result is +-0.  */
      const int sign = ieee_value.ft.sign;
      ieee_value.f = 0;
      ieee_value.ft.sign = sign;
    }
    else
      ieee_value.ft.mantissa &= ~(FRACTION_MASK >> unbiased_exponent);

    return ieee_value.f;
  }
}
