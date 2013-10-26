/* Copyright (C) 2013 DJ Delorie, see COPYING.DJ for details */


#include "t-main.h"

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
  { { 0xFFFFFFFFU, 0xF0000003U, 0x3FFFU + 0x001EU, 0 }, { 0x00000000U, 0xF0000002U, 0x401DU, 0 } },

  /*  Number greater than 2**63 thus all digits are significant and will not be truncated.  */
  { { 0x00000000U, 0x80000000U, 0x3FFFU + 0x0040U, 0 }, { 0x00000000U, 0x80000000U, 0x3FFFU + 0x0040U, 0 } }, /* 18446744073709551616.0000000000 */
  { { 0x00000000U, 0x80000000U, 0x3FFFU + 0x0040U, 1 }, { 0x00000000U, 0x80000000U, 0x3FFFU + 0x0040U, 1 } }, /* 18446744073709551616.0000000000 */

  /*  Number less than 1 thus will be truncated to 0.  */
  { { 0xB4395810U, 0xFFBE76C8U, 0x3FFFU + 0xFFFFFFFFU, 0 }, { 0x0U, 0x0U, 0x0U, 0 } }, /* 0.9990000000 */
  { { 0xB4395810U, 0xFFBE76C8U, 0x3FFFU + 0xFFFFFFFFU, 1 }, { 0x0U, 0x0U, 0x0U, 1 } }, /* -0.9990000000 */
  { { 0x00000000U, 0x80000000U, 0x3FFFU + 0xFFFFFFFFU, 0 }, { 0x0U, 0x0U, 0x0U, 0 } }, /* 0.50000000 */
  { { 0x00000000U, 0x80000000U, 0x3FFFU + 0xFFFFFFFFU, 1 }, { 0x0U, 0x0U, 0x0U, 1 } }, /* -0.50000000 */
  { { 0xFA9C12F1U, 0xA1CEF240U, 0x3FFFU + 0xFFFFFFF6U, 0 }, { 0x0U, 0x0U, 0x0U, 0 } }, /* 0.0012345000 */
  { { 0xFA9C12F1U, 0xA1CEF240U, 0x3FFFU + 0xFFFFFFF6U, 1 }, { 0x0U, 0x0U, 0x0U, 1 } }, /* -0.0012345000 */

  /*  Number less than 2**31 thus all digits in the lower fraction part of the mantissa are insignificant and will be truncated accordingly.  */
  { { 0x5B9A6800U, 0xFFFFFFFFU, 0x3FFFU + 0x001EU, 0 }, { 0x00000000U, 0xFFFFFFFEU, 0x3FFFU + 0x001EU, 0 } }, /* 2147483647.6789124012 */
  { { 0x5B9A6800U, 0xFFFFFFFFU, 0x3FFFU + 0x001EU, 1 }, { 0x00000000U, 0xFFFFFFFEU, 0x3FFFU + 0x001EU, 1 } }, /* -2147483647.6789124012 */
  { { 0x9988C800U, 0x800056E6U, 0x3FFFU + 0x0010U, 0 }, { 0x00000000U, 0x80000000U, 0x3FFFU + 0x0010U, 0 } }, /* 65536.6789123457 */
  { { 0x9988C800U, 0x800056E6U, 0x3FFFU + 0x0010U, 1 }, { 0x00000000U, 0x80000000U, 0x3FFFU + 0x0010U, 1 } }, /* -65536.6789123457 */
  { { 0xF4025800U, 0x800005B1U, 0x3FFFU + 0x0000U, 0 }, { 0x00000000U, 0x80000000U, 0x3FFFU + 0x0000U, 0 } }, /* 1.0000006789 */
  { { 0xF4025800U, 0x800005B1U, 0x3FFFU + 0x0000U, 1 }, { 0x00000000U, 0x80000000U, 0x3FFFU + 0x0000U, 1 } }, /* -1.0000006789 */

  /*  Number greather than 2**31 and less than 2**63 thus all digits in the lower and higher fraction part of the mantissa are insignificant and will be truncated accordingly.  */
  { { 0x00000000U, 0x80000000U, 0x3FFFU + 0x001FU, 0 }, { 0x00000000U, 0x80000000U, 0x3FFFU + 0x001FU, 0 } }, /* 2147483648.0000000000 */
  { { 0x00000000U, 0x80000000U, 0x3FFFU + 0x001FU, 1 }, { 0x00000000U, 0x80000000U, 0x3FFFU + 0x001FU, 1 } }, /* -2147483648.0000000000 */
  { { 0x00041000U, 0x80000000U, 0x3FFFU + 0x002DU, 0 }, { 0x00040000U, 0x80000000U, 0x3FFFU + 0x002DU, 0 } }, /* 35184372088833.0156250000 */
  { { 0x00041000U, 0x80000000U, 0x3FFFU + 0x002DU, 1 }, { 0x00040000U, 0x80000000U, 0x3FFFU + 0x002DU, 1 } }, /* -35184372088833.0156250000 */
  { { 0x00000000U, 0x80000000U, 0x3FFFU + 0x003FU, 0 }, { 0x00000000U, 0x80000000U, 0x3FFFU + 0x003FU, 0 } }, /* 9223372036854775808.0000000000 */
  { { 0x00000000U, 0x80000000U, 0x3FFFU + 0x003FU, 1 }, { 0x00000000U, 0x80000000U, 0x3FFFU + 0x003FU, 1 } }, /* -9223372036854775808.0000000000 */
};

static const size_t n_tests_long_double = sizeof(tests_long_double) / sizeof(tests_long_double[0]);


int truncl_test(void)
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
        printf("truncl test failed:  value to truncate = %.12Lf  result = %.12Lf  should be = %.12Lf\n", value.ld, result.ld, should_be.ld);
    }
    else
      printf("truncl test failed:  value to truncate = %.12Lf  result = %.12Lf  should be = %.12Lf\n", value.ld, result.ld, should_be.ld);
  }
  printf("%s\n", (counter < n_tests_long_double) ? "truncl test failed." : "truncl test succeded.");

  return (counter < n_tests_long_double) ? 1 : 0;
}
