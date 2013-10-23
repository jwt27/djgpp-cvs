/* Copyright (C) 2013 DJ Delorie, see COPYING.DJ for details */

#include <stdint.h>
#include <math.h>
#include <libc/ieee.h>

#if defined (__GNUC__) && (__GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 8))
# define __gnuc_extension__  __extension__
#else
# define __gnuc_extension__
#endif

#define FLOAT_BIAS                                (0x7FU)
#define BIN_DIGITS_IN_FRACTION                    (23)    /*  Amount of binary digits in fraction part of mantissa.  */
#define MAX_BIN_EXPONENT                          (127)
#define MIN_BIN_EXPONENT                          (-126)
#define ALL_DIGITS_ARE_SIGNIFICANT(exp)           ((exp) > (BIN_DIGITS_IN_FRACTION - 1))
#define MAGNITUDE_IS_LESS_THAN_ONE(exp)           ((exp) < 0)
#define MAGNITUDE_IS_GREATER_THAN_ONE_HALF(exp)   ((exp) == -1)
#define IS_INF_OR_NAN(exp)                        ((exp) > MAX_BIN_EXPONENT)

#define SIGNIFICANT_FRACTION_DIGITS_MASK(exp)     (0x007FFFFFUL >> (exp))
#define IS_INTEGRAL(num, unbiased_exponent)       (((num).ft.mantissa & SIGNIFICANT_FRACTION_DIGITS_MASK(unbiased_exponent)) == 0)
#define ROUND_TO_INTEGER(num, unbiased_exponent)                                                       \
(__gnuc_extension__                                                                                    \
  ({                                                                                                   \
     uint32_t rounded_mantissa = (uint32_t)(num).ft.mantissa + (0x00400000UL >> (unbiased_exponent));  \
     (num).ft.mantissa = rounded_mantissa & ~SIGNIFICANT_FRACTION_DIGITS_MASK(unbiased_exponent);      \
     (num).ft.exponent += (rounded_mantissa & ~0x007FFFFFUL) >> BIN_DIGITS_IN_FRACTION;                \
  })                                                                                                   \
)


#ifdef __STDC__
float
roundf(float x)
#else
float
roundf(x)
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
      const int sign = ieee_value.ft.sign;
      ieee_value.f = MAGNITUDE_IS_GREATER_THAN_ONE_HALF(unbiased_exponent) ? 1 : 0;
      ieee_value.ft.sign = sign;
    }
    else if (!IS_INTEGRAL(ieee_value, unbiased_exponent))
      ROUND_TO_INTEGER(ieee_value, unbiased_exponent);

    return ieee_value.f;
  }
}
