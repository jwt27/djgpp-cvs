/* Copyright (C) 2013 DJ Delorie, see COPYING.DJ for details */

#include <stdio.h>
#include <math.h>
#include <libc/ieee.h>

static const double_t tests_double[][2] =
{
  /*    value             should be        */
  /* Zeros. */
  { { 0x0U, 0x0U, 0x0U, 0 }, { 0x0U, 0x0U, 0x0U, 0 } }, /* 0.0 */
  { { 0x0U, 0x0U, 0x0U, 1 }, { 0x0U, 0x0U, 0x0U, 1 } }, /* -0.0 */

  /* Subnormals aka denormals. */
  { { 0x1U, 0x0U, 0x0U, 0 }, { 0x0U, 0x0U, 0x0U, 0 } }, /* Very small number. */
  { { 0x1U, 0x0U, 0x0U, 1 }, { 0x0U, 0x0U, 0x0U, 1 } }, /* Very small -number. */

  /* Normals. */
  { { 0x1U, 0x0U, 0x1U, 0 }, { 0x0U, 0x0U, 0x0U, 0 } }, /* Small number. */
  { { 0x1U, 0x0U, 0x1U, 1 }, { 0x0U, 0x0U, 0x0U, 1 } }, /* Small -number. */
  { { 0xFFFFFFFFU, 0x7FFFFU, 0x7FEU, 0 }, { 0xFFFFFFFFU, 0x7FFFFU, 0x7FEU, 0 } }, /* Big number. */
  { { 0xFFFFFFFFU, 0x7FFFFU, 0x7FEU, 1 }, { 0xFFFFFFFFU, 0x7FFFFU, 0x7FEU, 1 } }, /* Big -number. */

  /* Infs. */
  { { 0x0U, 0x0U, 0x7FFU, 0 }, { 0x0U, 0x0U, 0x7FFU, 0 } }, /* Inf */
  { { 0x0U, 0x0U, 0x7FFU, 1 }, { 0x0U, 0x0U, 0x7FFU, 1 } }, /* -Inf */

  /* NaNs. */
  { { 0x1U, 0x0U, 0x7FFU, 0 }, { 0x1U, 0x80000U, 0x7FFU, 0 } }, /* SNaN */
  { { 0x1U, 0x0U, 0x7FFU, 1 }, { 0x1U, 0x80000U, 0x7FFU, 1 } }, /* -SNaN */
  { { 0x0U, 0xFFFFFU, 0x7FFU, 0 }, { 0x0U, 0xFFFFFU, 0x7FFU, 0 } }, /* QNaN */
  { { 0x0U, 0xFFFFFU, 0x7FFU, 1 }, { 0x0U, 0xFFFFFU, 0x7FFU, 1 } }, /* -QNaN */

  /* Number. */
  { { 0x54442D18U, 0x921FBU, 0x400U, 0 }, { 0x0U, 0x80000U, 0x400U, 0 } }, /* PI */
  { { 0x54442D18U, 0x921FBU, 0x400U, 1 }, { 0x0U, 0x80000U, 0x400U, 1 } }, /* -PI */

  /* Different mantissa patterns. */
  { { 0xFFFFFFFFU, 0xF0003U, 0x3FFU + 0x016U, 0 }, { 0xC0000000U, 0xF0003U, 0x415U, 0 } },
  { { 0xFFFFFFFFU, 0xF0003U, 0x3FFU + 0x015U, 0 }, { 0x80000000U, 0xF0003U, 0x414U, 0 } },
  { { 0xFFFFFFFFU, 0xF0003U, 0x3FFU + 0x014U, 0 }, { 0x00000000U, 0xF0003U, 0x413U, 0 } },
  { { 0xFFFFFFFFU, 0xF0003U, 0x3FFU + 0x013U, 0 }, { 0x00000000U, 0xF0002U, 0x412U, 0 } }
};

static const size_t n_tests_double = sizeof(tests_double) / sizeof(tests_double[0]);



int main(void)
{
  int i, counter;

  for (counter = i = 0; i < n_tests_double; i++)
  {
    _double_union_t value, result, should_be;
    value.dt = tests_double[i][0];
    result.d = trunc(value.d);
    should_be.dt = tests_double[i][1];

    if (should_be.dt.sign == result.dt.sign &&
        should_be.dt.exponent == result.dt.exponent &&
        should_be.dt.mantissah == result.dt.mantissah &&
        should_be.dt.mantissal == result.dt.mantissal)
    {
      result.d = trunc(result.d);
      if (should_be.dt.sign == result.dt.sign &&
          should_be.dt.exponent == result.dt.exponent &&
          should_be.dt.mantissah == result.dt.mantissah &&
          should_be.dt.mantissal == result.dt.mantissal)
        counter++;
      else
        printf("trunc test failed:  value to truncate = %.12lg  result = %.12lg  should be = %.12lg\n", value.d, result.d, should_be.d);
    }
    else
      printf("trunc test failed:  value to truncate = %.12lg  result = %.12lg  should be = %.12lg\n", value.d, result.d, should_be.d);
  }
  printf("%s\n", (counter < n_tests_double) ? "trunc test failed." : "trunc test succeded.");

  return 0;
}
