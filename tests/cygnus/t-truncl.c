/* Copyright (C) 2013 DJ Delorie, see COPYING.DJ for details */

#include <stdio.h>
#include <math.h>
#include <libc/ieee.h>

static const long_double_t tests_long_double[][2] =
{
  /*    value             should be        */
  /* Zeros. */
  { { 0x0U, 0x0U, 0x0U, 0 }, { 0x0U, 0x0U, 0x0U, 0 } }, /* 0.0 */
  { { 0x0U, 0x0U, 0x0U, 1 }, { 0x0U, 0x0U, 0x0U, 1 } }, /* -0.0 */

  /* Subnormals aka denormals. */
  { { 0x1U, 0x0U, 0x0U, 0 }, { 0x0U, 0x0U, 0x0U, 0 } }, /* Very small number. */
  { { 0x1U, 0x0U, 0x0U, 1 }, { 0x0U, 0x0U, 0x0U, 1 } }, /* Very small -number. */

  /* Normals. */
  { { 0x0U, 0x80000000U, 0x1U, 0 }, { 0x0U, 0x0U, 0x0U, 0 } }, /* Small number. */
  { { 0x0U, 0x80000000U, 0x1U, 1 }, { 0x0U, 0x0U, 0x0U, 1 } }, /* Small -number. */
  { { 0xFFFFFFFFU, 0xFFFFFFFFU, 0x7FFEU, 0 }, { 0xFFFFFFFFU, 0xFFFFFFFFU, 0x7FFEU, 0 } }, /* Big number. */
  { { 0xFFFFFFFFU, 0xFFFFFFFFU, 0x7FFEU, 1 }, { 0xFFFFFFFFU, 0xFFFFFFFFU, 0x7FFEU, 1 } }, /* Big -number. */

  /* Infs. */
  { { 0x0U, 0x80000000U, 0x7FFFU, 0 }, { 0x0U, 0x80000000U, 0x7FFFU, 0 } }, /* Inf */
  { { 0x0U, 0x80000000U, 0x7FFFU, 1 }, { 0x0U, 0x80000000U, 0x7FFFU, 1 } }, /* -Inf */

  /* NaNs. */
  { { 0x1U, 0x80000000U, 0x7FFFU, 0 }, { 0x1U, 0xC0000000U, 0x7FFFU, 0 } }, /* SNaN */
  { { 0x1U, 0x80000000U, 0x7FFFU, 1 }, { 0x1U, 0xC0000000U, 0x7FFFU, 1 } }, /* -SNaN */
  { { 0x0U, 0xFFFFFFFFU, 0x7FFFU, 0 }, { 0x0U, 0xFFFFFFFFU, 0x7FFFU, 0 } }, /* QNaN */
  { { 0x0U, 0xFFFFFFFFU, 0x7FFFU, 1 }, { 0x0U, 0xFFFFFFFFU, 0x7FFFU, 1 } }, /* -QNaN */

  /* Number. */
  { { 0x2168C000U, 0xC90FDAA2U, 0x4000U, 0 }, { 0x0U, 0xC0000000U, 0x4000U, 0 } }, /* PI */
  { { 0x2168C000U, 0xC90FDAA2U, 0x4000U, 1 }, { 0x0U, 0xC0000000U, 0x4000U, 1 } }, /* -PI */

  /* Different mantissa patterns. */
  { { 0xFFFFFFFFU, 0xF0000003U, 0x3FFFU + 0x0021U, 0 }, { 0xC0000000U, 0xF0000003U, 0x4020U, 0 } },
  { { 0xFFFFFFFFU, 0xF0000003U, 0x3FFFU + 0x0020U, 0 }, { 0x80000000U, 0xF0000003U, 0x401FU, 0 } },
  { { 0xFFFFFFFFU, 0xF0000003U, 0x3FFFU + 0x001FU, 0 }, { 0x00000000U, 0xF0000003U, 0x401EU, 0 } },
  { { 0xFFFFFFFFU, 0xF0000003U, 0x3FFFU + 0x001EU, 0 }, { 0x00000000U, 0xF0000002U, 0x401DU, 0 } }
};

static const size_t n_tests_long_double = sizeof(tests_long_double) / sizeof(tests_long_double[0]);



int main(void)
{
  int i, counter;

  for (counter = i = 0; i < n_tests_long_double; i++)
  {
    _longdouble_union_t value, result, should_be;
    value.ldt = tests_long_double[i][0];
    result.ld = truncl(value.ld);
    should_be.ldt = tests_long_double[i][1];

    if (should_be.ldt.sign == result.ldt.sign &&
        should_be.ldt.exponent == result.ldt.exponent &&
        should_be.ldt.mantissah == result.ldt.mantissah &&
        should_be.ldt.mantissal == result.ldt.mantissal)
    {
      result.ld = truncl(result.ld);
      if (should_be.ldt.sign == result.ldt.sign &&
          should_be.ldt.exponent == result.ldt.exponent &&
          should_be.ldt.mantissah == result.ldt.mantissah &&
          should_be.ldt.mantissal == result.ldt.mantissal)
        counter++;
      else
        printf("truncl test failed:  value to truncate = %.15Lg  result = %.15Lg  should be = %.15Lg\n", value.ld, result.ld, should_be.ld);
    }
    else
      printf("truncl test failed:  value to truncate = %.15Lg  result = %.15Lg  should be = %.15Lg\n", value.ld, result.ld, should_be.ld);
  }
  printf("%s\n", (counter < n_tests_long_double) ? "truncl test failed." : "truncl test succeded.");

  return 0;
}
