/* Copyright (C) 2013 DJ Delorie, see COPYING.DJ for details */


#include "t-main.h"

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
  { { 0xFFFFFFFFU, 0xF0003U, 0x3FFU + 0x013U, 0 }, { 0x00000000U, 0xF0002U, 0x412U, 0 } },

  /*  Number greater than 2**52 thus all digits are significant and will not be truncated.  */
  { { 0x89ABCDEFU, 0x12345U, 0x3FFU + 0x034U, 0 }, { 0x89ABCDEFU, 0x12345U, 0x3FFU + 0x034U, 0 } }, /* 4823856173534703.0000000000 */
  { { 0x89ABCDEFU, 0x12345U, 0x3FFU + 0x034U, 1 }, { 0x89ABCDEFU, 0x12345U, 0x3FFU + 0x034U, 1 } }, /* -4823856173534703.0000000000 */

  /*  Number less than 1 thus will be truncated to 0.  */
  { { 0x7AE147AEU, 0xFAE14U, 0x3FFU + 0xFFFFFFFFU, 0 }, { 0x0U, 0x0U, 0x0U, 0 } }, /* 0.99000 */
  { { 0x7AE147AEU, 0xFAE14U, 0x3FFU + 0xFFFFFFFFU, 1 }, { 0x0U, 0x0U, 0x0U, 1 } }, /* -0.99000 */
  { { 0x00000000U, 0x00000U, 0x3FFU + 0xFFFFFFFFU, 0 }, { 0x0U, 0x0U, 0x0U, 0 } }, /* 0.5000000000 */
  { { 0x00000000U, 0x00000U, 0x3FFU + 0xFFFFFFFFU, 1 }, { 0x0U, 0x0U, 0x0U, 1 } }, /* -0.5000000000 */
  { { 0xD2F1A9FCU, 0x0624DU, 0x3FFU + 0xFFFFFFF3U, 0 }, { 0x0U, 0x0U, 0x0U, 0 } }, /* 0.0001250000 */
  { { 0xD2F1A9FCU, 0x0624DU, 0x3FFU + 0xFFFFFFF3U, 1 }, { 0x0U, 0x0U, 0x0U, 1 } }, /* -0.0001250000 */

  /*  Number less than 2**20 thus all digits in the lower fraction part of the mantissa are insignificant and will be truncated accordingly.  */
  { { 0xF9ADD3C1U, 0xFFFFFU, 0x3FFU + 0x013U, 0 }, { 0x00000000U, 0xFFFFEU, 0x3FFU + 0x013U, 0 } }, /* 1048575.9876543210 */
  { { 0xF9ADD3C1U, 0xFFFFFU, 0x3FFU + 0x013U, 1 }, { 0x00000000U, 0xFFFFEU, 0x3FFU + 0x013U, 1 } }, /* -1048575.9876543210 */
  { { 0x9D658A12U, 0x0C6F7U, 0x3FFU + 0x000U, 0 }, { 0x00000000U, 0x00000U, 0x3FFU + 0x000U, 0 } }, /* 1.0485759877 */
  { { 0x9D658A12U, 0x0C6F7U, 0x3FFU + 0x000U, 1 }, { 0x00000000U, 0x00000U, 0x3FFU + 0x000U, 1 } }, /* -1.0485759877 */

  /*  Number greather than 2**20 and less than 2**52 thus all digits in the lower and higher fraction part of the mantissa are insignificant and will be truncated accordingly.  */
  { { 0x11B578C9U, 0x00000U, 0x3FFU + 0x015U, 0 }, { 0x00000000U, 0x00000U, 0x3FFU + 0x015U, 0 } }, /* 2097152.1383505804 */
  { { 0x11B578C9U, 0x00000U, 0x3FFU + 0x015U, 1 }, { 0x00000000U, 0x00000U, 0x3FFU + 0x015U, 1 } }, /* -2097152.1383505804 */
  { { 0xFFFFFFFEU, 0xFFFFFU, 0x3FFU + 0x033U, 0 }, { 0xFFFFFFFEU, 0xFFFFFU, 0x3FFU + 0x033U, 0 } }, /* 4503599627370495.0000000000 */
  { { 0xFFFFFFFEU, 0xFFFFFU, 0x3FFU + 0x033U, 1 }, { 0xFFFFFFFEU, 0xFFFFFU, 0x3FFU + 0x033U, 1 } }, /* -4503599627370495.0000000000 */
  { { 0xFF734ACAU, 0xFFFFFU, 0x3FFU + 0x01CU, 0 }, { 0xFF000000U, 0xFFFFFU, 0x3FFU + 0x01CU, 0 } }, /* 536870911.4503599405 */
  { { 0xFF734ACAU, 0xFFFFFU, 0x3FFU + 0x01CU, 1 }, { 0xFF000000U, 0xFFFFFU, 0x3FFU + 0x01CU, 1 } }, /* -536870911.4503599405 */
};

static const size_t n_tests_double = sizeof(tests_double) / sizeof(tests_double[0]);


int trunc_test(void)
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
        printf("trunc test failed:  value to truncate = %.12lf  result = %.12lf  should be = %.12lf\n", value.d, result.d, should_be.d);
    }
    else
      printf("trunc test failed:  value to truncate = %.12lf  result = %.12lf  should be = %.12lf\n", value.d, result.d, should_be.d);
  }
  printf("%s\n", (counter < n_tests_double) ? "trunc test failed." : "trunc test succeded.");

  return (counter < n_tests_double) ? 1 : 0;
}
