/* Copyright (C) 2013 DJ Delorie, see COPYING.DJ for details */

#include <math.h>
#include <libc/ieee.h>

#define FLOAT_BIAS                     (0x7FU)
#define MAX_BIN_EXPONENT               (127)
#define MIN_BIN_EXPONENT               (-126)
#define BIN_DIGITS_IN_FRACTION         (23)    /*  Amount of binary digits in fraction part of mantissa.  */
#define IS_INF_OR_NAN(x)               ((x) > MAX_BIN_EXPONENT)
#define MAGNITUDE_IS_LESS_THAN_ONE(x)  ((x) < 0)


#ifdef __STDC__
float truncf(float x)
#else
float truncf(x)
float x;
#endif
{
  _float_union_t ieee_value;
  int unbiased_exponent;


  ieee_value.f = x;
  unbiased_exponent = ieee_value.ft.exponent - FLOAT_BIAS;

  if (unbiased_exponent < BIN_DIGITS_IN_FRACTION)
  {
    if (MAGNITUDE_IS_LESS_THAN_ONE(unbiased_exponent))
    {
      /* The magnitude of the number is < 1 so the result is +-0.  */
      int sign = ieee_value.ft.sign;
      ieee_value.f = 0;
      ieee_value.ft.sign = sign;
    }
    else
      ieee_value.ft.mantissa &= ~(0x7FFFFFU >> unbiased_exponent);

    return ieee_value.f;
  }
  else if (IS_INF_OR_NAN(unbiased_exponent))
    return x + x;  /* Trigger an exception.  */


  /* All bits in the fraction field are relevant. */
  return x;
}
