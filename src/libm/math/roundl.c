/* Copyright (C) 2013 DJ Delorie, see COPYING.DJ for details */

#include <stdint.h>
#include <math.h>
#include <libc/ieee.h>

#if defined (__GNUC__) && (__GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 8))
# define __gnuc_extension__  __extension__
#else
# define __gnuc_extension__
#endif

#define LONG_DOUBLE_BIAS                                    (0x3FFFU)
#define MAX_BIN_EXPONENT                                    (16383)
#define MIN_BIN_EXPONENT                                    (-16382)
#define BIN_DIGITS_IN_FRACTION                              (63)  /*  Amount of binary digits in fraction part of mantissa.  */
#define BIN_DIGITS_IN_MANTISSAH                             (31)  /*  Amount of binary digits in msw of the fraction part of mantissa.  */
#define ALL_DIGITS_ARE_SIGNIFICANT(exp)                     ((exp) > (BIN_DIGITS_IN_FRACTION - 1))
#define NO_SIGNIFICANT_DIGITS_IN_MANTISSAL(exp)             ((exp) < BIN_DIGITS_IN_MANTISSAH)
#define MAGNITUDE_IS_LESS_THAN_ONE(exp)                     ((exp) < 0)
#define MAGNITUDE_IS_GREATER_THAN_ONE_HALF(exp)             ((exp) == -1)
#define IS_INF_OR_NAN(exp)                                  ((exp) > MAX_BIN_EXPONENT)

#define SIGNIFICANT_FRACTION_DIGITS_MASK(exp)               (0xFFFFFFFFUL >> ((exp) - BIN_DIGITS_IN_MANTISSAH))
#define IS_INTEGRAL(num, unbiased_exponent)                 (((num).ldt.mantissal & SIGNIFICANT_FRACTION_DIGITS_MASK(unbiased_exponent)) == 0)

#define CARRY_INTO_INTEGER_PART(x)                          ((x) & 0x100000000ULL)
#define MANTISSAH_SIGNIFICANT_FRACTION_DIGITS_MASK(exp)     (0x7FFFFFFFUL >> (exp))
#define IS_MANTISSAH_INTEGRAL(num, unbiased_exponent)       ((((num).ldt.mantissah & MANTISSAH_SIGNIFICANT_FRACTION_DIGITS_MASK(unbiased_exponent)) == 0) && ((num).ldt.mantissal == 0))
#define ROUND_MANTISSAH_TO_INTEGER(num, unbiased_exponent)                                                     \
(__gnuc_extension__                                                                                            \
  ({                                                                                                           \
     uint64_t rounded_mantissa = (uint64_t)(num).ldt.mantissah + (0x40000000ULL >> (unbiased_exponent));       \
     if (CARRY_INTO_INTEGER_PART(rounded_mantissa))                                                            \
     {                                                                                                         \
       rounded_mantissa >>= 1;                                                                                 \
       (num).ldt.exponent += 1;                                                                                \
     }                                                                                                         \
     (num).ldt.mantissah = rounded_mantissa & ~MANTISSAH_SIGNIFICANT_FRACTION_DIGITS_MASK(unbiased_exponent);  \
     (num).ldt.mantissal = 0;                                                                                  \
  })                                                                                                           \
)
#define ROUND_MANTISSA_TO_INTEGER(num, unbiased_exponent)                                                                   \
(__gnuc_extension__                                                                                                         \
  ({                                                                                                                        \
     const uint32_t rounded_mantissal = (num).ldt.mantissal + (1UL << (BIN_DIGITS_IN_FRACTION - 1 - (unbiased_exponent)));  \
                                                                                                                            \
     if (rounded_mantissal < (num).ldt.mantissal)                                                                           \
       (num).ldt.mantissah += 1;  /* Carry from rounded mantissal.  */                                                      \
     (num).ldt.mantissal = rounded_mantissal & ~SIGNIFICANT_FRACTION_DIGITS_MASK((unbiased_exponent));                      \
  })                                                                                                                        \
)


#ifdef __STDC__
long double
roundl(long double x)
#else
long double
round(x)
long double x;
#endif
{
  _longdouble_union_t ieee_value;
  int unbiased_exponent;


  ieee_value.ld = x;
  unbiased_exponent = ieee_value.ldt.exponent - LONG_DOUBLE_BIAS;

  if (ALL_DIGITS_ARE_SIGNIFICANT(unbiased_exponent))        /* Trigger an exception if INF or NAN  */
    return IS_INF_OR_NAN(unbiased_exponent) ? x + x : x;    /* else return the number.  */
  else
  {
    if (NO_SIGNIFICANT_DIGITS_IN_MANTISSAL(unbiased_exponent))
    {
      /* All significant digits are in msw. */
      if (MAGNITUDE_IS_LESS_THAN_ONE(unbiased_exponent))
      {
        const int sign = ieee_value.ldt.sign;
        ieee_value.ld = MAGNITUDE_IS_GREATER_THAN_ONE_HALF(unbiased_exponent) ? 1 : 0;
        ieee_value.ldt.sign = sign;
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

    return ieee_value.ld;
  }
}
