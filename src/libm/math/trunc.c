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
	The <<trunc>> functions round their argument to the integer value, in
	floating format, nearest to but no larger in magnitude than the
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

#define DOUBLE_BIAS                      (0x3FFU)
#define MAX_BIN_EXPONENT                 (1023)
#define MIN_BIN_EXPONENT                 (-1022)
#define BIN_DIGITS_IN_FRACTION           (51)  /*  Amount of binary digits in fraction part of mantissa.  */
#define BIN_DIGITS_IN_MSW                (20)  /*  Amount of binary digits in msw of the fraction part of mantissa.  */
#define NO_SIGNIFICANT_DIGITS_IN_LSW(x)  ((x) < BIN_DIGITS_IN_MSW)
#define IS_INF_OR_NAN(x)                 ((x) > MAX_BIN_EXPONENT)
#define MAGNITUDE_IS_LESS_THAN_ONE(x)    ((x) < 0)


#ifdef __STDC__
double trunc(double x)
#else
double trunc(x)
double x;
#endif
{
  _double_union_t ieee_value;
  int unbiased_exponent;


  ieee_value.d = x;
  unbiased_exponent = ieee_value.dt.exponent - DOUBLE_BIAS;

  if (NO_SIGNIFICANT_DIGITS_IN_LSW(unbiased_exponent))
  {
    /* All significant digits are in msw. */
    if (MAGNITUDE_IS_LESS_THAN_ONE(unbiased_exponent))
    {
      /* The magnitude of the number is < 1 so the result is +-0.  */
      int sign = ieee_value.dt.sign;
      ieee_value.d = 0;
      ieee_value.dt.sign = sign;
    }
    else
    {
      ieee_value.dt.mantissah &= ~(0x000FFFFFU >> unbiased_exponent);
      ieee_value.dt.mantissal = 0;
    }

    return ieee_value.d;
  }
  else if (unbiased_exponent > BIN_DIGITS_IN_FRACTION)
  {
    if (IS_INF_OR_NAN(unbiased_exponent))
      return x + x;  /* Trigger an exception.  */
  }
  else
  {
    /*
     *  All fraction bits in msw are relevant.
     *  Truncate irrelevant bits from lsw.
     */
    ieee_value.dt.mantissal &= ~(0xFFFFFFFFU >> (unbiased_exponent - BIN_DIGITS_IN_MSW));
    return ieee_value.d;
  }


  /*
   *  All bits in the fraction fields of the msw
   *  and lsw are needed in the result.
   */
  return x;
}
