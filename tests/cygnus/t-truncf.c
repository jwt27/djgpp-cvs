/* Copyright (C) 2013 DJ Delorie, see COPYING.DJ for details */

#include <stdio.h>
#include <math.h>
#include <libc/ieee.h>

static const float_t tests_float[][2] =
{
  /*    value             should be        */
  /* Zeros. */
  { { 0x0U, 0x0U, 0 }, { 0x0U, 0x0U, 0 } }, /* 0.0 */
  { { 0x0U, 0x0U, 1 }, { 0x0U, 0x0U, 1 } }, /* -0.0 */

  /* Subnormals aka denormals. */
  { { 0x1U, 0x0U, 0 }, { 0x0U, 0x0U, 0 } }, /* Very small number. */
  { { 0x1U, 0x0U, 1 }, { 0x0U, 0x0U, 1 } }, /* Very small -number. */

  /* Normals. */
  { { 0x1U, 0x1U, 0 }, { 0x0U, 0x0U, 0 } }, /* Small number. */
  { { 0x1U, 0x1U, 1 }, { 0x0U, 0x0U, 1 } }, /* Small -number. */
  { { 0xFFFFU, 0xFEU, 0 }, { 0xFFFFU, 0xFEU, 0 } }, /* Big number. */
  { { 0xFFFFU, 0xFEU, 1 }, { 0xFFFFU, 0xFEU, 1 } }, /* Big -number. */

  /* Infs. */
  { { 0x0U, 0xFFU, 0 }, { 0x0U, 0xFFU, 0 } }, /* Inf */
  { { 0x0U, 0xFFU, 1 }, { 0x0U, 0xFFU, 1 } }, /* -Inf */

  /* NaNs. */
  { { 0x1U, 0xFFU, 0 }, { 0x400001U, 0xFFU, 0 } }, /* SNaN */
  { { 0x1U, 0xFFU, 1 }, { 0x400001U, 0xFFU, 1 } }, /* -SNaN */
  { { 0x7FFFFFU, 0xFFU, 0 }, { 0x7FFFFFU, 0xFFU, 0 } }, /* QNaN */
  { { 0x7FFFFFU, 0xFFU, 1 }, { 0x7FFFFFU, 0xFFU, 1 } }, /* -QNaN */

  /* Number. */
  { { 0x490FDBU, 0x80U, 0 }, { 0x400000U, 0x80U, 0 } }, /* PI */
  { { 0x490FDBU, 0x80U, 1 }, { 0x400000U, 0x80U, 1 } }, /* -PI */

  /* Different mantissa patterns. */
  { { 0x7FFFFFU, 0x96U, 0 }, { 0x7FFFFFU, 0x96U, 0 } },
  { { 0x7FFFFFU, 0x95U, 0 }, { 0x7FFFFEU, 0x95U, 0 } },
  { { 0x1555FFU, 0x8DU, 0 }, { 0x155400U, 0x8DU, 0 } }
};

static const size_t n_tests_float = sizeof(tests_float) / sizeof(tests_float[0]);


int main(void)
{
  int i, counter;

  for (counter = i = 0; i < n_tests_float; i++)
  {
    _float_union_t value, result, should_be;
    value.ft = tests_float[i][0];
    result.f = truncf(value.f);
    should_be.ft = tests_float[i][1];

    if (should_be.ft.sign == result.ft.sign &&
        should_be.ft.exponent == result.ft.exponent &&
        should_be.ft.mantissa == result.ft.mantissa)
    {
      result.f = truncf(result.f);
      if (should_be.ft.sign == result.ft.sign &&
          should_be.ft.exponent == result.ft.exponent &&
          should_be.ft.mantissa == result.ft.mantissa)
        counter++;     
      else
        printf("truncf test failed:  value to truncate = %.5g  result = %.5g  should be = %.5g\n", value.f, result.f, should_be.f);
    }
    else
      printf("truncf test failed:  value to truncate = %.5g  result = %.5g  should be = %.5g\n", value.f, result.f, should_be.f);
  }
  printf("%s\n", (counter < n_tests_float) ? "truncf test failed." : "truncf test succeded.");

  return 0;
}
