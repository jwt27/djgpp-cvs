/* Copyright (C) 2013 DJ Delorie, see COPYING.DJ for details */

#include <stdint.h>
#include <math.h>
#include <libc/ieee.h>

#if defined (__GNUC__) && (__GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 8))
# define __gnuc_extension__  __extension__
#else
# define __gnuc_extension__
#endif

#define FLOAT_BIAS                                           (0x7FU)
#define BIN_DIGITS_IN_FRACTION                               (23)    /*  Amount of binary digits in fraction part of mantissa.  */
#define ALL_DIGITS_ARE_SIGNIFICANT(exp)                      ((exp) > (BIN_DIGITS_IN_FRACTION - 1))
#define MAGNITUDE_IS_TOO_LARGE(exp)                          ((exp) > (int)(sizeof(long int) * 8) - 2)
#define MAGNITUDE_IS_LESS_THAN_ONE(exp)                      ((exp) < 0)
#define MAGNITUDE_IS_LESS_THAN_ONE_HALF(exp)                 ((exp) < -1)
#define IS_ZERO(num)                                         ((((num).ft.mantissa & ~(1UL << BIN_DIGITS_IN_FRACTION)) == 0) && (((num).ft.exponent & 0xFFU) == 0))
#define CONVERT_MANTISSA_TO_INTEGER(num, unbiased_exponent)  ((long int)(((uint32_t)(num).ft.mantissa | 0x00800000UL) << ((unbiased_exponent) - BIN_DIGITS_IN_FRACTION)))
#define ROUND_MANTISSA(num, unbiased_exponent)               ((long int)(((uint32_t)(num).ft.mantissa | 0x00800000UL) >> (BIN_DIGITS_IN_FRACTION - (unbiased_exponent))))
#define ROUND_MANTISSA_TO_INTEGER(num, unbiased_exponent)                                                                          \
(__gnuc_extension__                                                                                                                \
  ({                                                                                                                               \
     (num).f += two23[(num).ft.sign];                                                                                              \
     (num).f -= two23[(num).ft.sign];                                                                                              \
     (unbiased_exponent) = (num).ft.exponent - FLOAT_BIAS;                                                                         \
                                                                                                                                   \
     result = MAGNITUDE_IS_LESS_THAN_ONE((unbiased_exponent)) || IS_ZERO((num)) ? 0 : ROUND_MANTISSA((num), (unbiased_exponent));  \
     (long int)result;                                                                                                             \
  })                                                                                                                               \
)


/* Adding a float, x, to 2^23 will cause the result to be rounded based on
   the fractional part of x, according to the implementation's current rounding
   mode.  2^23 is the smallest float that can be represented using all 23 significant
   digits. */
#ifdef __STDC__
static const float
#else
static float
#endif
two23[2] = {
  8388608,  /* 0, 0x7FU + 0x17U, 0x000000U */
 -8388608   /* 1, 0x7FU + 0x17U, 0x000000U */
};

#ifdef __STDC__
long int
lrintf(float x)
#else
long int
lrintf(x)
float x;
#endif
{
  volatile _float_union_t ieee_value;
  int unbiased_exponent;


  ieee_value.f = x;
  unbiased_exponent = ieee_value.ft.exponent - FLOAT_BIAS;

  if (MAGNITUDE_IS_TOO_LARGE(unbiased_exponent))  /* The number is too large.  */
    return (long int)x;                           /* It is left implementation defined what happens.  */
  else
  {
    long int result;


    if (MAGNITUDE_IS_LESS_THAN_ONE_HALF(unbiased_exponent))
      result = 0;
    else
      result = ALL_DIGITS_ARE_SIGNIFICANT(unbiased_exponent) ? CONVERT_MANTISSA_TO_INTEGER(ieee_value, unbiased_exponent)  /* >= 2^23 is already an exact integer.  */
                                                             : ROUND_MANTISSA_TO_INTEGER(ieee_value, unbiased_exponent);
    return ieee_value.ft.sign ? -result : result;
  }
}
