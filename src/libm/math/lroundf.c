/* Copyright (C) 2013 DJ Delorie, see COPYING.DJ for details */

#include <stdint.h>
#include <math.h>
#include <libc/ieee.h>

#define FLOAT_BIAS                                           (0x7FU)
#define BIN_DIGITS_IN_FRACTION                               (23)    /*  Amount of binary digits in fraction part of mantissa.  */
#define MAGNITUDE_IS_TOO_LARGE(exp)                          ((exp) > (int)(sizeof(long int) * 8) - 2)
#define MAGNITUDE_IS_LESS_THAN_ONE(exp)                      ((exp) < 0)
#define MAGNITUDE_IS_LESS_THAN_ONE_HALF(exp)                 ((exp) < -1)
#define CONVERT_MANTISSA_TO_INTEGER(num, unbiased_exponent)  ((long int)((uint32_t)(num).ft.mantissa | 0x00800000UL) << ((unbiased_exponent) - BIN_DIGITS_IN_FRACTION))
#define ROUND_MANTISSA_TO_INTEGER(num, unbiased_exponent)    ((long int)((((uint32_t)(num).ft.mantissa | 0x00800000UL) + (0x00400000UL >> (unbiased_exponent))) >> (BIN_DIGITS_IN_FRACTION - (unbiased_exponent))))


#ifdef __STDC__
long int
lroundf(float x)
#else
long int
lroundf(x)
float x;
#endif
{
  _float_union_t ieee_value;
  int unbiased_exponent;


  ieee_value.f = x;
  unbiased_exponent = ieee_value.ft.exponent - FLOAT_BIAS;

  if (MAGNITUDE_IS_TOO_LARGE(unbiased_exponent))  /* The number is too large.  */
    return (long int)x;                           /* It is left implementation defined what happens.  */
  else
  {
    long int result;
    int sign = ieee_value.ft.sign;


    if (MAGNITUDE_IS_LESS_THAN_ONE(unbiased_exponent))
      result = MAGNITUDE_IS_LESS_THAN_ONE_HALF(unbiased_exponent) ? 0 : 1;
    else if (unbiased_exponent > (BIN_DIGITS_IN_FRACTION - 1))
      result = CONVERT_MANTISSA_TO_INTEGER(ieee_value, unbiased_exponent);  /* >= 2^23 is already an exact integer.  */
    else
      result = ROUND_MANTISSA_TO_INTEGER(ieee_value, unbiased_exponent);

    return sign ? -result :  result;
  }
}
