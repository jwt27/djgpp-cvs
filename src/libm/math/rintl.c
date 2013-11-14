/* Copyright (C) 2013 DJ Delorie, see COPYING.DJ for details */

#include <stdint.h>
#include <math.h>
#include <libc/ieee.h>

#if defined (__GNUC__) && (__GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 8))
# define __gnuc_extension__  __extension__
#else
# define __gnuc_extension__
#endif

#define LONG_DOUBLE_BIAS                                          (0x3FFFU)
#define MAX_BIN_EXPONENT                                          (16383)
#define MIN_BIN_EXPONENT                                          (-16382)
#define BIN_DIGITS_IN_FRACTION                                    (63)  /*  Amount of binary digits in fraction part of mantissa.  */
#define BIN_DIGITS_IN_MANTISSAH                                   (31)  /*  Amount of binary digits in msw of the fraction part of mantissa.  */
#define ALL_DIGITS_ARE_SIGNIFICANT(exp)                           ((exp) > (BIN_DIGITS_IN_FRACTION - 1))
#define NO_SIGNIFICANT_DIGITS_IN_MANTISSAL(exp)                   ((exp) < BIN_DIGITS_IN_MANTISSAH)
#define MAGNITUDE_IS_LESS_THAN_ONE(exp)                           ((exp) < 0)
#define IS_INF_OR_NAN(exp)                                        ((exp) > MAX_BIN_EXPONENT)
#define IS_ZERO(num)                                              (((num).ldt.mantissah | (num).ldt.mantissal | (num).ldt.exponent) == 0)

#define MANTISSAH_SIGNIFICANT_FRACTION_DIGITS_MASK(pattern, exp)  ((pattern) >> (exp))
#define IS_MANTISSAH_INTEGRAL(num, unbiased_exponent)             ((((num).ldt.mantissah & MANTISSAH_SIGNIFICANT_FRACTION_DIGITS_MASK(0x7FFFFFFFUL, unbiased_exponent)) | (num).ldt.mantissal) == 0)
#define ROUND_MANTISSAH_TO_INTEGER(num, unbiased_exponent)                                                                                                  \
(__gnuc_extension__                                                                                                                                         \
  ({                                                                                                                                                        \
     const uint32_t rounded_mantissah = (uint32_t)(MANTISSAH_SIGNIFICANT_FRACTION_DIGITS_MASK(0x7FFFFFFFUL, (unbiased_exponent)) >> 1);                     \
                                                                                                                                                            \
     if ((rounded_mantissah & (num).ldt.mantissah) | (num).ldt.mantissal)                                                                                   \
     {                                                                                                                                                      \
       if ((unbiased_exponent) == (BIN_DIGITS_IN_MANTISSAH - 1))                                                                                            \
         (num).ldt.mantissal = 0x40000000UL;                                                                                                                \
       else                                                                                                                                                 \
         (num).ldt.mantissah = (~rounded_mantissah & (num).ldt.mantissah) | MANTISSAH_SIGNIFICANT_FRACTION_DIGITS_MASK(0x20000000UL, (unbiased_exponent));  \
     }                                                                                                                                                      \
                                                                                                                                                            \
     (num).ld += two63[(num).ldt.sign];                                                                                                                     \
     (num).ld -= two63[(num).ldt.sign];                                                                                                                     \
  })                                                                                                                                                        \
)

#define SIGNIFICANT_FRACTION_DIGITS_MASK(pattern, exp)            ((pattern) >> ((exp) - BIN_DIGITS_IN_MANTISSAH))
#define IS_INTEGRAL(num, unbiased_exponent)                       (((num).ldt.mantissal & SIGNIFICANT_FRACTION_DIGITS_MASK(0x7FFFFFFFUL, (unbiased_exponent))) == 0)
#define ROUND_MANTISSA_TO_INTEGER(num, unbiased_exponent)                                                                                       \
(__gnuc_extension__                                                                                                                             \
  ({                                                                                                                                            \
     const uint32_t rounded_mantissal = (uint32_t)(SIGNIFICANT_FRACTION_DIGITS_MASK(0x7FFFFFFFUL, (unbiased_exponent)) >> 1);                   \
                                                                                                                                                \
     if (rounded_mantissal & (num).ldt.mantissal)                                                                                               \
       (num).ldt.mantissal = (~rounded_mantissal & (num).ldt.mantissal) | SIGNIFICANT_FRACTION_DIGITS_MASK(0x40000000UL, (unbiased_exponent));  \
                                                                                                                                                \
     (num).ld += two63[(num).ldt.sign];                                                                                                         \
     (num).ld -= two63[(num).ldt.sign];                                                                                                         \
  })                                                                                                                                            \
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
long double
rintl(long double x)
#else
long double
rintl(x)
long double x;
#endif
{
  volatile _longdouble_union_t ieee_value;
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
        if (!IS_ZERO(ieee_value))
        {
          const int sign = ieee_value.ldt.sign;
          int32_t mantissal = (int32_t)ieee_value.ldt.mantissah | (int32_t)ieee_value.ldt.mantissal;

          ieee_value.ldt.mantissah &= 0xE0000000UL;
          ieee_value.ldt.mantissah |= (mantissal | -mantissal) & 0x80000000UL;
          ieee_value.ld += two63[ieee_value.ldt.sign];                                                                                                                   \
          ieee_value.ld -= two63[ieee_value.ldt.sign];                                                                                                                   \
          ieee_value.ldt.sign = sign;
        }
      }
      else if (!IS_MANTISSAH_INTEGRAL(ieee_value, unbiased_exponent))
        ROUND_MANTISSAH_TO_INTEGER(ieee_value, unbiased_exponent);
    }
    else if (!IS_INTEGRAL(ieee_value, unbiased_exponent))
        ROUND_MANTISSA_TO_INTEGER(ieee_value, unbiased_exponent);  /* Also digits in mantissa low part are significant.  */

    return ieee_value.ld;
  }
}
