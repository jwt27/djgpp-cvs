/* Copyright (C) 2013 DJ Delorie, see COPYING.DJ for details */

/*
FUNCTION
<<lround>>, <<lroundf>>, <<lroundl>>--round to integer, to nearest
INDEX
        lround
INDEX
        lroundf
INDEX
        lroundl

ANSI_SYNOPSIS
        #include <math.h>
        long int lround(double <[x]>);
        long int lroundf(float <[x]>);
        long int lroundl(long double <[x]>);

DESCRIPTION
        The <<lround>>, <<lroundl>> and <<lroundl>> functions round their
        argument to the nearest integer value, rounding halfway cases away from
        zero, regardless of the current rounding direction.  If the rounded
        value is outside the range of the return type, the numeric result is
        unspecified (depending upon the floating-point implementation, not the
        library).  A range error may occur if the magnitude of x is too large.

RETURNS
        <[x]> rounded to an integral value as an integer.

SEEALSO
        See the <<round>> functions for the return being the same floating-point
        type as the argument.  See also <<lrint>>.

PORTABILITY
ANSI C, POSIX

*/

#include <stdint.h>
#include <math.h>
#include <libc/ieee.h>

#if defined (__GNUC__) && (__GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 8))
# define __gnuc_extension__  __extension__
#else
# define __gnuc_extension__
#endif

#define DOUBLE_BIAS                                          (0x3FFU)
#define BIN_DIGITS_IN_FRACTION                               (52)  /*  Amount of binary digits in fraction part of mantissa.  */
#define BIN_DIGITS_IN_MANTISSAH                              (20)  /*  Amount of binary digits in msw of the fraction part of mantissa.  */
#define ALL_DIGITS_ARE_SIGNIFICANT(exp)                      ((exp) > (BIN_DIGITS_IN_FRACTION - 1))
#define NO_SIGNIFICANT_DIGITS_IN_MANTISSAL(exp)              ((exp) < BIN_DIGITS_IN_MANTISSAH)
#define MAGNITUDE_IS_TOO_LARGE(exp)                          ((exp) > (int)(sizeof(long int) * 8) - 2)
#define MAGNITUDE_IS_LESS_THAN_ONE(exp)                      ((exp) < 0)
#define MAGNITUDE_IS_LESS_THAN_ONE_HALF(exp)                 ((exp) < -1)

#define ROUND_MANTISSAH_TO_INTEGER(num, unbiased_exponent)   ((long int)((((uint32_t)(num).dt.mantissah | 0x00100000UL) + (0x0080000UL >> (unbiased_exponent))) >> (BIN_DIGITS_IN_MANTISSAH - (unbiased_exponent))))

#define SHIFT_LEFT_MANTISSAH(num, unbiased_exponent)         (((uint64_t)(num).dt.mantissah | 0x00100000ULL) << ((unbiased_exponent) - BIN_DIGITS_IN_MANTISSAH))
#define SHIFT_LEFT_MANTISSAL(num, unbiased_exponent)         ((uint64_t)(num).dt.mantissal << ((unbiased_exponent) - BIN_DIGITS_IN_FRACTION))
#define CONVERT_MANTISSA_TO_INTEGER(num, unbiased_exponent)  ((long int)(SHIFT_LEFT_MANTISSAH(num, unbiased_exponent) | SHIFT_LEFT_MANTISSAL(num, unbiased_exponent)))
#define ROUND_MANTISSA_TO_INTEGER(num, unbiased_exponent)                                                                                         \
(__gnuc_extension__                                                                                                                               \
  ({                                                                                                                                              \
     uint32_t rounded_mantissal = (uint32_t)(num).dt.mantissal + (0x80000000UL >> ((unbiased_exponent) - BIN_DIGITS_IN_MANTISSAH));               \
     if (result = ((long int)(num).dt.mantissah | 0x00100000UL), rounded_mantissal < (num).dt.mantissal) result++;                                \
     if ((unbiased_exponent) > BIN_DIGITS_IN_MANTISSAH)                                                                                           \
       result = result << ((unbiased_exponent) - BIN_DIGITS_IN_MANTISSAH) | rounded_mantissal >> (BIN_DIGITS_IN_FRACTION - (unbiased_exponent));  \
     (long int)result;                                                                                                                            \
  })                                                                                                                                              \
)


#ifdef __STDC__
long int
lround(double x)
#else
long int
lround(x)
double x;
#endif
{
  _double_union_t ieee_value;
  int unbiased_exponent;


  ieee_value.d = x;
  unbiased_exponent = ieee_value.dt.exponent - DOUBLE_BIAS;

  if (MAGNITUDE_IS_TOO_LARGE(unbiased_exponent))  /* The number is too large.  */
    return (long int)x;                           /* It is left implementation defined what happens.  */
  else
  {
    long int result;
    int sign = ieee_value.dt.sign;


    if (NO_SIGNIFICANT_DIGITS_IN_MANTISSAL(unbiased_exponent))
    {
      if (MAGNITUDE_IS_LESS_THAN_ONE(unbiased_exponent))
        result = MAGNITUDE_IS_LESS_THAN_ONE_HALF(unbiased_exponent) ? 0 : 1;
      else
        result = ROUND_MANTISSAH_TO_INTEGER(ieee_value, unbiased_exponent);
    }
    else
    {
      if (ALL_DIGITS_ARE_SIGNIFICANT(unbiased_exponent))                      /* >= 2^52 is already an exact integer iff long int is 64 bit.  */
        result = CONVERT_MANTISSA_TO_INTEGER(ieee_value, unbiased_exponent);  /* But this is not the case with djgpp.  */
      else
        result = ROUND_MANTISSA_TO_INTEGER(ieee_value, unbiased_exponent);
    }

    return sign ? -result : result;
  }
}
