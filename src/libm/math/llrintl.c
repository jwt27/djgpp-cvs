/* Copyright (C) 2013 DJ Delorie, see COPYING.DJ for details */

#include <stdint.h>
#include <math.h>
#include <libc/ieee.h>

#if defined (__GNUC__) && (__GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 8))
# define __gnuc_extension__  __extension__
#else
# define __gnuc_extension__
#endif

#define LONG_DOUBLE_BIAS                                     (0x3FFFU)
#define BIN_DIGITS_IN_FRACTION                               (63)  /*  Amount of binary digits in fraction part of mantissa.  */
#define BIN_DIGITS_IN_MANTISSAH                              (31)  /*  Amount of binary digits in msw of the fraction part of mantissa.  */
#define ALL_DIGITS_ARE_SIGNIFICANT(exp)                      ((exp) > (BIN_DIGITS_IN_FRACTION - 1))
#define NO_SIGNIFICANT_DIGITS_IN_MANTISSAL(exp)              ((exp) < BIN_DIGITS_IN_MANTISSAH)
#define MAGNITUDE_IS_TOO_LARGE(exp)                          ((exp) > (int)(sizeof(long long int) * 8) - 2)
#define MAGNITUDE_IS_LESS_THAN_ONE(exp)                      ((exp) < 0)
#define MAGNITUDE_IS_LESS_THAN_ONE_HALF(exp)                 ((exp) < -1)
#define IS_ZERO(num)                                         ((((num).ldt.mantissah & 0xFFFFFFFFUL) == 0) && (((num).ldt.mantissal & 0xFFFFFFFFUL) == 0) && (((num).ldt.exponent & 0x7FFFU) == 0))

#define ROUND_MANTISSAH(num, unbiased_exponent)              ((long long int)((uint64_t)(num).ldt.mantissah >> (BIN_DIGITS_IN_MANTISSAH - (unbiased_exponent))))
#define ROUND_MANTISSAH_TO_INTEGER(num, unbiased_exponent)                                                                          \
(__gnuc_extension__                                                                                                                 \
  ({                                                                                                                                \
     (num).ld += two63[(num).ldt.sign];                                                                                             \
     (num).ld -= two63[(num).ldt.sign];                                                                                             \
     (unbiased_exponent) = (num).ldt.exponent - LONG_DOUBLE_BIAS;                                                                   \
                                                                                                                                    \
     result = MAGNITUDE_IS_LESS_THAN_ONE((unbiased_exponent)) || IS_ZERO((num)) ? 0 : ROUND_MANTISSAH((num), (unbiased_exponent));  \
     (long long int)result;                                                                                                         \
  })                                                                                                                                \
)

#define SHIFT_LEFT_MANTISSAH(num, unbiased_exponent)         ((uint64_t)(num).ldt.mantissah << ((unbiased_exponent) - BIN_DIGITS_IN_MANTISSAH))
#define CONVERT_MANTISSA_TO_INTEGER(num, unbiased_exponent)  ((long long int)(SHIFT_LEFT_MANTISSAH(num, unbiased_exponent) | (num).ldt.mantissal << ((unbiased_exponent) - BIN_DIGITS_IN_FRACTION)))
#define ROUND_MANTISSA(num, unbiased_exponent)               ((long long int)(SHIFT_LEFT_MANTISSAH(num, unbiased_exponent) | (num).ldt.mantissal >> (BIN_DIGITS_IN_FRACTION - (unbiased_exponent))))
#define ROUND_MANTISSA_TO_INTEGER(num, unbiased_exponent)                                                                                          \
(__gnuc_extension__                                                                                                                                \
  ({                                                                                                                                               \
     (num).ld += two63[(num).ldt.sign];                                                                                                            \
     (num).ld -= two63[(num).ldt.sign];                                                                                                            \
     (unbiased_exponent) = (num).ldt.exponent - LONG_DOUBLE_BIAS;                                                                                  \
                                                                                                                                                   \
     result = ((unbiased_exponent) == BIN_DIGITS_IN_MANTISSAH) ? (long long int)(num).ldt.mantissah : ROUND_MANTISSA((num), (unbiased_exponent));  \
     (long long int)result;                                                                                                                        \
  })                                                                                                                                               \
)


/* Adding a long double, x, to 2^63 will cause the result to be rounded based on
   the fractional part of x, according to the implementation's current rounding
   mode.  2^63 is the smallest long double that can be represented using all 63
   significant digits. */
#ifdef __STDC__
static const long double
#else
static long double
#endif
two63[2] = {
  9.223372036854775808E+18,  /* 0, 0x3FFFE + 0x003F, 0x80000000U, 0x00000000U */
 -9.223372036854775808E+18   /* 1, 0x3FFFE + 0x003F, 0x80000000U, 0x00000000U */
};

#ifdef __STDC__
long long int
llrintl(long double x)
#else
long long int
llrintl(x)
long double x;
#endif
{
  volatile _longdouble_union_t ieee_value;
  int unbiased_exponent;


  ieee_value.ld = x;
  unbiased_exponent = ieee_value.ldt.exponent - LONG_DOUBLE_BIAS;

  if (MAGNITUDE_IS_TOO_LARGE(unbiased_exponent))  /* The number is too large.  */
    return (long long int)x;                      /* It is left implementation defined what happens.  */
  else
  {
    long long int result;


    if (NO_SIGNIFICANT_DIGITS_IN_MANTISSAL(unbiased_exponent))
      result = MAGNITUDE_IS_LESS_THAN_ONE_HALF(unbiased_exponent) ? 0 : ROUND_MANTISSAH_TO_INTEGER(ieee_value, unbiased_exponent);
    else
      result = ALL_DIGITS_ARE_SIGNIFICANT(unbiased_exponent) ? CONVERT_MANTISSA_TO_INTEGER(ieee_value, unbiased_exponent)
                                                             : ROUND_MANTISSA_TO_INTEGER(ieee_value, unbiased_exponent);
    return ieee_value.ldt.sign ? -result : result;
  }
}
