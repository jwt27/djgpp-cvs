/* Copyright (C) 2013 DJ Delorie, see COPYING.DJ for details */

/*
FUNCTION
<<lrint>>, <<lrintf>>, <<lrintl>>--round to nearest integer value using current rounding direction
INDEX
	lrint
INDEX
	lrintf
INDEX
	lrintl

ANSI_SYNOPSIS
        #include <math.h>
        long int lrint(double <[x]>);
        long int lrintf(float <[x]>);
        long int lrintl(long double <[x]>);

DESCRIPTION
        The <<lrint>> functions round their argument to the nearest integer value,
        using the current rounding direction.

        Note that unlike <<rint>>, etc., the return type of these functions differs
        from that of their arguments.

RETURNS
        These functions return the rounded integer value of <[x]>.
        If <[x]> is NaN or an infinity, or the rounded value is too large
        to be stored in a long then a domain error occurs, and the return
        value is unspecified.

        These functions do not set errno.

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
#define IS_ZERO(num)                                         ((((num).dt.mantissah & ~(1L << BIN_DIGITS_IN_MANTISSAH)) == 0) && (((num).dt.mantissal & 0xFFFFFFFFUL) == 0) && (((num).dt.exponent & 0x07FFU) == 0))

#define ROUND_MANTISSAH(num, unbiased_exponent)              ((long int)(((uint32_t)(num).dt.mantissah | 0x00100000UL) >> (BIN_DIGITS_IN_MANTISSAH - (unbiased_exponent))))
#define ROUND_MANTISSAH_TO_INTEGER(num, unbiased_exponent)                                                                          \
(__gnuc_extension__                                                                                                                 \
  ({                                                                                                                                \
     (num).d += two52[(num).dt.sign];                                                                                               \
     (num).d -= two52[(num).dt.sign];                                                                                               \
     (unbiased_exponent) = (num).dt.exponent - DOUBLE_BIAS;                                                                         \
                                                                                                                                    \
     result = MAGNITUDE_IS_LESS_THAN_ONE((unbiased_exponent)) || IS_ZERO((num)) ? 0 : ROUND_MANTISSAH((num), (unbiased_exponent));  \
     (long int)result;                                                                                                              \
  })                                                                                                                                \
)

#define SHIFT_LEFT_MANTISSAH(num, unbiased_exponent)         (((uint32_t)(num).dt.mantissah | 0x00100000UL) << ((unbiased_exponent) - BIN_DIGITS_IN_MANTISSAH))
#define CONVERT_MANTISSA_TO_INTEGER(num, unbiased_exponent)  ((long int)(SHIFT_LEFT_MANTISSAH(num, unbiased_exponent) | (num).dt.mantissal << ((unbiased_exponent) - BIN_DIGITS_IN_FRACTION)))
#define ROUND_MANTISSA(num, unbiased_exponent)               ((long int)(SHIFT_LEFT_MANTISSAH(num, unbiased_exponent) | (num).dt.mantissal >> (BIN_DIGITS_IN_FRACTION - (unbiased_exponent))))
#define ROUND_MANTISSA_TO_INTEGER(num, unbiased_exponent)                                                                                    \
(__gnuc_extension__                                                                                                                          \
  ({                                                                                                                                         \
     (num).d += two52[(num).dt.sign];                                                                                                        \
     (num).d -= two52[(num).dt.sign];                                                                                                        \
     (unbiased_exponent) = (num).dt.exponent - DOUBLE_BIAS;                                                                                  \
                                                                                                                                             \
     result = ((unbiased_exponent) == BIN_DIGITS_IN_MANTISSAH) ? (long int)(num).dt.mantissah : ROUND_MANTISSA((num), (unbiased_exponent));  \
     (long int)result;                                                                                                                       \
  })                                                                                                                                         \
)


/* Adding a double, x, to 2^52 will cause the result to be rounded based on
   the fractional part of x, according to the implementation's current rounding
   mode.  2^52 is the smallest double that can be represented using all 52 significant
   digits. */
#ifdef __STDC__
static const double
#else
static double
#endif
two52[2] = {
  4.503599627370496e+15, /* 0, 0x3FFU + 0x034U, 0x00000U, 0x00000000U */
 -4.503599627370496e+15  /* 1, 0x3FFU + 0x034U, 0x00000U, 0x00000000U */
};

#ifdef __STDC__
long int
lrint(double x)
#else
long int
lrint(x)
double x;
#endif
{
  volatile _double_union_t ieee_value;
  int unbiased_exponent;


  ieee_value.d = x;
  unbiased_exponent = ieee_value.dt.exponent - DOUBLE_BIAS;

  if (MAGNITUDE_IS_TOO_LARGE(unbiased_exponent))  /* The number is too large.  */
    return (long int)x;                           /* It is left implementation defined what happens.  */
  else
  {
    long int result;


    if (NO_SIGNIFICANT_DIGITS_IN_MANTISSAL(unbiased_exponent))
      result = MAGNITUDE_IS_LESS_THAN_ONE_HALF(unbiased_exponent) ? 0 : ROUND_MANTISSAH_TO_INTEGER(ieee_value, unbiased_exponent);
    else
      result = ALL_DIGITS_ARE_SIGNIFICANT(unbiased_exponent) ? CONVERT_MANTISSA_TO_INTEGER(ieee_value, unbiased_exponent)  /* >= 2^52 is already an exact integer iff long int is 64 bit.  But this is not the case with djgpp.  */
                                                             : ROUND_MANTISSA_TO_INTEGER(ieee_value, unbiased_exponent);
    return ieee_value.dt.sign ? -result : result;
  }
}
