/* Copyright (C) 2013 DJ Delorie, see COPYING.DJ for details */


#include <stdio.h>
#include <math.h>
#include <libc/ieee.h>

typedef struct {
  const _float_union_t value;     /* test value */
  const long long int should_be;  /* result */
} entry_t;

static const entry_t tests_float[] =
{
  /* test value */
  /*     value           should be   */

  /* Zeros. */
  {{.ft = {0x0U, 0x0U, 0}},   0}, /* 0.0 */
  {{.ft = {0x0U, 0x0U, 1}},   0}, /* -0.0 */

  /* Subnormals aka denormals. */
  {{.ft = {0x1U, 0x0U, 0}},   0}, /* Very small number. */
  {{.ft = {0x1U, 0x0U, 1}},   0}, /* Very small -number. */

  /* Normals. */ 
  {{.ft = {0x1U, 0x1U, 0}},   0}, /* Small number. */
  {{.ft = {0x1U, 0x1U, 1}},   0}, /* Small -number. */
  {{.ft = {0xFFFFU, 0xFEU, 0}},   -9223372036854775808ULL}, /* Big number. */
  {{.ft = {0xFFFFU, 0xFEU, 1}},   -9223372036854775808ULL}, /* Big -number. */

  /* Infs. */
  {{.ft = {0x0U, 0xFFU, 0}},   -9223372036854775808ULL}, /* Inf */
  {{.ft = {0x0U, 0xFFU, 1}},   -9223372036854775808ULL}, /* -Inf */

  /* NaNs. */
  {{.ft = {0x1U, 0xFFU, 0}},   -9223372036854775808ULL}, /* SNaN */
  {{.ft = {0x1U, 0xFFU, 1}},   -9223372036854775808ULL}, /* -SNaN */
  {{.ft = {0x7FFFFFU, 0xFFU, 0}},   -9223372036854775808ULL}, /* QNaN */
  {{.ft = {0x7FFFFFU, 0xFFU, 1}},   -9223372036854775808ULL}, /* -QNaN */

  /* Numbers. */
  {{.ft = {0x490FDBU, 0x80U, 0}},   +3}, /* PI */
  {{.ft = {0x490FDBU, 0x80U, 1}},   -3}, /* -PI */

  {{.ft = {0x700000U, 0x7FU, 0}},  +2},  /* 1.875000 */
  {{.ft = {0x700000U, 0x7FU, 1}},  -2},  /* -1.875000 */
  {{.ft = {0x500000U, 0x7FU, 0}},  +2},  /* 1.625000 */
  {{.ft = {0x500000U, 0x7FU, 1}},  -2},  /* -1.625000 */
  {{.ft = {0x40000FU, 0x7FU, 0}},  +2},  /* 1.500002 */
  {{.ft = {0x40000FU, 0x7FU, 1}},  -2},  /* -1.500002 */
  {{.ft = {0x400000U, 0x7FU, 0}},  +2},  /* 1.500000 */
  {{.ft = {0x400000U, 0x7FU, 1}},  -2},  /* -1.500000 */
  {{.ft = {0x3FFFF0U, 0x7FU, 0}},  +1},  /* 1.499998 */
  {{.ft = {0x3FFFF0U, 0x7FU, 1}},  -1},  /* -1.499998 */
  {{.ft = {0x300000U, 0x7FU, 0}},  +1},  /* 1.375000 */
  {{.ft = {0x300000U, 0x7FU, 1}},  -1},  /* -1.375000 */
  {{.ft = {0x100000U, 0x7FU, 0}},  +1},  /* 1.125000 */
  {{.ft = {0x100000U, 0x7FU, 1}},  -1},  /* -1.125000 */

  {{.ft = {0x000000U, 0x7FU + 0x16U, 0}},  +4194304},  /* 4194304.000000 */
  {{.ft = {0x000000U, 0x7FU + 0x16U, 1}},  -4194304},  /* -4194304.000000 */ 
  {{.ft = {0x000000U, 0x7FU + 0x17U, 0}},  +8388608},  /* 8388608.000000 */
  {{.ft = {0x000000U, 0x7FU + 0x17U, 1}},  -8388608},  /* -8388608.000000 */
  {{.ft = {0x000000U, 0x7FU + 0x18U, 0}},  +16777216},  /* 16777216.000000 */
  {{.ft = {0x000000U, 0x7FU + 0x18U, 1}},  -16777216},  /* -16777216.000000 */

  {{.ft = {0x000000U, 0x7FU + 0x1EU, 0}},  +1073741824},  /* 1073741824.000000 */
  {{.ft = {0x000000U, 0x7FU + 0x1EU, 1}},  -1073741824},  /* -1073741824.000000 */
  {{.ft = {0x000000U, 0x7FU + 0x1FU, 0}},  +2147483648LL},  /* 2147483648.000000 */
  {{.ft = {0x000000U, 0x7FU + 0x1FU, 1}},  -2147483648LL},  /* -2147483648.000000 */
  {{.ft = {0x000000U, 0x7FU + 0x20U, 0}},  4294967296},  /* 4294967296.000000 */
  {{.ft = {0x000000U, 0x7FU + 0x20U, 1}},  -4294967296},  /* -4294967296.000000 */

  /* Different mantissa patterns. */
  {{.ft = {0x7FFFFFU, 0x96U, 0}},  +16777215},  /* 16777215.000000 */
  {{.ft = {0x7FF000U, 0x95U, 0}},  +8386560},  /* 8386560.000000 */
  {{.ft = {0x1555FFU, 0x8DU, 0}},  +19115},  /* 19115.000000 */
  {{.ft = {0x7FF000U, 0x96U, 1}},  -16773120},  /* -16773120.000000 */
  {{.ft = {0x7FFFFEU, 0x95U, 1}},  -8388607},  /* -8388607.000000 */
  {{.ft = {0x1555FFU, 0x8DU, 1}},  -19115},  /* -19115.000000 */

  /*  Number greater than 2**63 exceeds long long int size and will be casted in an implementation defined manner.  */
  {{.ft = {0x000001U, 0x7FU + 0x3FU, 0}},  +9223372036854775808ULL},  /* 9223372036854775808.000000 */
  {{.ft = {0x000001U, 0x7FU + 0x3FU, 1}},  -9223372036854775808ULL},  /* -9223372036854775808.000000 */

  /*  Number less than 0.5 will be truncated to 0.  */
  {{.ft = {0x7FBC99U, 0x7FU + 0xFFFFFFFEU, 0}},  0},  /* 0.4994857609 */
  {{.ft = {0x7FBC99U, 0x7FU + 0xFFFFFFFEU, 1}},  -0},  /* -0.4994857609 */
  {{.ft = {0x03126FU, 0x7FU + 0xFFFFFFF6U, 0}},  0},  /* 0.001000 */
  {{.ft = {0x03126FU, 0x7FU + 0xFFFFFFF6U, 1}},  -0},  /* -0.001000 */

  /*  Number greater than 0.5 and less than 1 will be rounded to 1.  */
  {{.ft = {0x7CD6EAU, 0x7FU + 0xFFFFFFFFU, 0}},  1},  /* 0.987654 */
  {{.ft = {0x7CD6EAU, 0x7FU + 0xFFFFFFFFU, 1}},  -1},  /* -0.987654 */
  {{.ft = {0x000001U, 0x7FU + 0xFFFFFFFFU, 0}},  1},  /* 0.50000006 */
  {{.ft = {0x000001U, 0x7FU + 0xFFFFFFFFU, 1}},  -1},  /* -0.50000006 */
  {{.ft = {0x000000U, 0x7FU + 0xFFFFFFFFU, 0}},  1},  /* 0.500000 */
  {{.ft = {0x000000U, 0x7FU + 0xFFFFFFFFU, 1}},  -1},  /* -0.500000 */

  /*  Number greather than 1 and less than 2**23 will be rounded accordingly.  */
  {{.ft = {0x000000U, 0x7FU + 0x00U, 0}},  1},  /* 1.0000000000 */
  {{.ft = {0x000000U, 0x7FU + 0x00U, 1}},  -1},  /* 1.0000000000 */
  {{.ft = {0x00000FU, 0x7FU + 0x00U, 0}},  1},  /* 1.000002 */
  {{.ft = {0x00000FU, 0x7FU + 0x00U, 1}},  -1},  /* 1.000002 */
  {{.ft = {0x000018U, 0x7FU + 0x10U, 0}},  65536},  /* 65536.1875000000 */
  {{.ft = {0x000018U, 0x7FU + 0x10U, 1}},  -65536},  /* -65536.1875000000 */
  {{.ft = {0x000040U, 0x7FU + 0x10U, 0}},  65537},  /* 65536.5000000 */
  {{.ft = {0x000040U, 0x7FU + 0x10U, 1}},  -65537},  /* -65536.5000000 */
  {{.ft = {0x00004DU, 0x7FU + 0x10U, 0}},  65537},  /* 65536.6015625000 */
  {{.ft = {0x00004DU, 0x7FU + 0x10U, 1}},  -65537},  /* -65536.6015625000 */
  {{.ft = {0x7FFFFFU, 0x7FU + 0x16U, 0}},  8388608},  /* 8388607.5000000000 */
  {{.ft = {0x7FFFFFU, 0x7FU + 0x16U, 1}},  -8388608},  /* -8388607.5000000000 */
  {{.ft = {0x000005U, 0x7FU + 0x14U, 0}},  1048577},  /* 1048576.6250000000 */
  {{.ft = {0x000005U, 0x7FU + 0x14U, 1}},  -1048577},  /* -1048576.6250000000 */
};

static const size_t n_tests_float = sizeof(tests_float) / sizeof(tests_float[0]);


int main(void)
{
  unsigned int i, counter;

  for (counter = i = 0; i < n_tests_float; i++)
  {
    long long int result = llroundf(tests_float[i].value.f);

    if (tests_float[i].should_be == result)
      counter++;
    else
      printf("llroundf test failed:  value to round = %.12f  result = %lld  should be = %lld\n", tests_float[i].value.f, result, tests_float[i].should_be);
  }
  printf("%s\n", (counter < n_tests_float) ? "llroundf test failed." : "llroundf test succeded.");

  return 0;
}
