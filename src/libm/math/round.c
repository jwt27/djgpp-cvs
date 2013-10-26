/* Copyright (C) 2013 DJ Delorie, see COPYING.DJ for details */

/*
FUNCTION
<<round>>, <<roundf>>, <<roundl>>--round to nearest integer
INDEX
	round
INDEX
	roundf
INDEX
	roundl

ANSI_SYNOPSIS
        #include <math.h>
        double round(double <[x]>);
        float roundf(float <[x]>);
        long double round(long double <[x]>);

DESCRIPTION
        The <<round>> functions round their argument to the nearest integer
        value in floating-point format, rounding halfway cases away from zero,
        regardless of the current rounding direction.  (While the "inexact"
        floating-point exception behavior is unspecified by the C standard, the
        <<round>> functions are written so that "inexact" is not raised if the
        result does not equal the argument, which behavior is as recommended by
        IEEE 754 for its related functions.)

RETURNS
        <[x]> rounded to an integral value.

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

#define DOUBLE_BIAS                                         (0x3FFU)
#define MAX_BIN_EXPONENT                                    (1023)
#define MIN_BIN_EXPONENT                                    (-1022)
#define BIN_DIGITS_IN_FRACTION                              (52)  /*  Amount of binary digits in fraction part of mantissa.  */
#define BIN_DIGITS_IN_MANTISSAH                             (20)  /*  Amount of binary digits in msw of the fraction part of mantissa.  */
#define ALL_DIGITS_ARE_SIGNIFICANT(exp)                     ((exp) > (BIN_DIGITS_IN_FRACTION - 1))
#define NO_SIGNIFICANT_DIGITS_IN_MANTISSAL(exp)             ((exp) < BIN_DIGITS_IN_MANTISSAH)
#define MAGNITUDE_IS_LESS_THAN_ONE(exp)                     ((exp) < 0)
#define MAGNITUDE_IS_GREATER_THAN_ONE_HALF(exp)             ((exp) == -1)
#define IS_INF_OR_NAN(exp)                                  ((exp) > MAX_BIN_EXPONENT)

#define SIGNIFICANT_FRACTION_DIGITS_MASK(exp)               (0xFFFFFFFFUL >> ((exp) - BIN_DIGITS_IN_MANTISSAH))
#define IS_INTEGRAL(num, unbiased_exponent)                 (((num).dt.mantissal & SIGNIFICANT_FRACTION_DIGITS_MASK(unbiased_exponent)) == 0)

#define CARRY_INTO_INTEGER_PART(x)                          ((x) & 0x00100000UL)
#define MANTISSAH_SIGNIFICANT_FRACTION_DIGITS_MASK(exp)     (0x000FFFFFUL >> (exp))
#define IS_MANTISSAH_INTEGRAL(num, unbiased_exponent)       ((((num).dt.mantissah & MANTISSAH_SIGNIFICANT_FRACTION_DIGITS_MASK(unbiased_exponent)) == 0) && ((num).dt.mantissal == 0))
#define ROUND_MANTISSAH_TO_INTEGER(num, unbiased_exponent)                                                    \
(__gnuc_extension__                                                                                           \
  ({                                                                                                          \
     uint64_t rounded_mantissa = (uint64_t)(num).dt.mantissah + (0x00080000UL >> (unbiased_exponent));        \
     (num).dt.mantissah = rounded_mantissa & ~MANTISSAH_SIGNIFICANT_FRACTION_DIGITS_MASK(unbiased_exponent);  \
     (num).dt.mantissal = 0;                                                                                  \
     (num).dt.exponent += (rounded_mantissa & ~0x000FFFFUL) >> BIN_DIGITS_IN_MANTISSAH;                       \
  })                                                                                                          \
)
#define ROUND_MANTISSA_TO_INTEGER(num, unbiased_exponent)                                                                            \
(__gnuc_extension__                                                                                                                  \
  ({                                                                                                                                 \
     const uint32_t rounded_mantissal = (uint32_t)(num).dt.mantissal + (1UL << (BIN_DIGITS_IN_FRACTION - 1 - (unbiased_exponent)));  \
                                                                                                                                     \
     if (rounded_mantissal < (num).dt.mantissal)                                                                                     \
     {                                                                                                                               \
       const uint32_t result = (uint32_t)(num).dt.mantissah + 1;  /* Carry from rounded mantissal.  */                               \
                                                                                                                                     \
       if (CARRY_INTO_INTEGER_PART(result))                                                                                          \
         (num).dt.exponent += 1;  /* Carry from rounded mantissal.  */                                                               \
       (num).dt.mantissah = result;                                                                                                  \
     }                                                                                                                               \
     (num).dt.mantissal = rounded_mantissal & ~SIGNIFICANT_FRACTION_DIGITS_MASK((unbiased_exponent));                                \
  })                                                                                                                                 \
)


#ifdef __STDC__
double
round(double x)
#else
double
round(x)
double x;
#endif
{
  _double_union_t ieee_value;
  int unbiased_exponent;


  ieee_value.d = x;
  unbiased_exponent = ieee_value.dt.exponent - DOUBLE_BIAS;

  if (ALL_DIGITS_ARE_SIGNIFICANT(unbiased_exponent))        /* Trigger an exception if INF or NAN  */
    return IS_INF_OR_NAN(unbiased_exponent) ? x + x : x;    /* else return the number.  */
  else
  {
    if (NO_SIGNIFICANT_DIGITS_IN_MANTISSAL(unbiased_exponent))
    {
      /* All significant digits are in msw. */
      if (MAGNITUDE_IS_LESS_THAN_ONE(unbiased_exponent))
      {
        const int sign = ieee_value.dt.sign;
        ieee_value.d = MAGNITUDE_IS_GREATER_THAN_ONE_HALF(unbiased_exponent) ? 1 : 0;
        ieee_value.dt.sign = sign;
      }
      else if (!IS_MANTISSAH_INTEGRAL(ieee_value, unbiased_exponent))
        ROUND_MANTISSAH_TO_INTEGER(ieee_value, unbiased_exponent);
    }
    else
    {
      /* Also digits in mantissa low part are significant.  */
      if (!IS_INTEGRAL(ieee_value, unbiased_exponent))
        ROUND_MANTISSA_TO_INTEGER(ieee_value, unbiased_exponent);
    }

    return ieee_value.d;
  }
}
