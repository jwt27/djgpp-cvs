/* Copyright (C) 2013 DJ Delorie, see COPYING.DJ for details */

/*
FUNCTION
<<trunc>>, <<truncf>>, <<truncl>>--round to integer, towards zero
INDEX
        trunc
INDEX
        truncf
INDEX
        truncl

ANSI_SYNOPSIS
        #include <math.h>
        double trunc(double <[x]>);
        float truncf(float <[x]>);
        long double truncl(long double <[x]>);

DESCRIPTION
        The <<trunc>> functions round their argument to the integer value,
        in floating format, nearest to but no larger in magnitude than the
        argument, regardless of the current rounding direction.

RETURNS
        <[x]> truncated to an integral value.
        If <[x]> is NaN, a NaN will be returned.
        If <[x]> is +/-0 or +/-Inf, <[x]> will be returned.

PORTABILITY
ANSI C, POSIX

*/

#include <math.h>
#include <libc/ieee.h>

#define DOUBLE_BIAS                              (0x3FFU)
#define MAX_BIN_EXPONENT                         (1023)
#define MIN_BIN_EXPONENT                         (-1022)
#define BIN_DIGITS_IN_FRACTION                   (52)  /*  Amount of binary digits in fraction part of mantissa.  */
#define BIN_DIGITS_IN_MANTISSAH                  (20)  /*  Amount of binary digits in msw of the fraction part of mantissa.  */
#define FRACTION_HIGH_PART_MASK                  (0x000FFFFFUL)
#define FRACTION_LOW_PART_MASK                   (0xFFFFFFFFUL)
#define ALL_DIGITS_ARE_SIGNIFICANT(exp)          ((exp) > (BIN_DIGITS_IN_FRACTION - 1))
#define NO_SIGNIFICANT_DIGITS_IN_MANTISSAL(exp)  ((exp) < BIN_DIGITS_IN_MANTISSAH)
#define MAGNITUDE_IS_LESS_THAN_ONE(exp)          ((exp) < 0)
#define IS_INF_OR_NAN(exp)                       ((exp) > MAX_BIN_EXPONENT)


#ifdef __STDC__
double
trunc(double x)
#else
double
trunc(x)
double x;
#endif
{
  _double_union_t ieee_value;
  int unbiased_exponent;


  ieee_value.d = x;
  unbiased_exponent = ieee_value.dt.exponent - DOUBLE_BIAS;

  if (ALL_DIGITS_ARE_SIGNIFICANT(unbiased_exponent))        /* Trigger an exception if INF or NAN  */
    return IS_INF_OR_NAN(unbiased_exponent) ? x + x : x;    /* else return the number.  */
  else if (NO_SIGNIFICANT_DIGITS_IN_MANTISSAL(unbiased_exponent))
  {
    /* All significant digits are in msw. */
    if (MAGNITUDE_IS_LESS_THAN_ONE(unbiased_exponent))
    {
      /* The magnitude of the number is < 1 so the result is +-0.  */
      const int sign = ieee_value.dt.sign;
      ieee_value.d = 0;
      ieee_value.dt.sign = sign;
    }
    else
    {
      ieee_value.dt.mantissah &= ~(FRACTION_HIGH_PART_MASK >> unbiased_exponent);
      ieee_value.dt.mantissal = 0;
    }

    return ieee_value.d;
  }
  else
  {
    /*
     *  All fraction bits in msw are relevant.
     *  Truncate irrelevant bits from lsw.
     */
    ieee_value.dt.mantissal &= ~(FRACTION_LOW_PART_MASK >> (unbiased_exponent - BIN_DIGITS_IN_MANTISSAH));
    return ieee_value.d;
  }
}
