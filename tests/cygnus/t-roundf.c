/* Copyright (C) 2013 DJ Delorie, see COPYING.DJ for details */


#include "t-main.h"

typedef struct {
  const _float_union_t value;      /* test value */
  const _float_union_t should_be;  /* result */
} entry_t;

static const entry_t tests_float[] =
{
  /* test value */
  /*     value           should be   */

  /* Zeros. */
  {{.ft = {0x0U, 0x0U, 0}},   {0}}, /* 0.0 */
  {{.ft = {0x0U, 0x0U, 1}},   {-0}}, /* -0.0 */

  /* Subnormals aka denormals. */
  {{.ft = {0x1U, 0x0U, 0}},   {0}}, /* Very small number. */
  {{.ft = {0x1U, 0x0U, 1}},   {-0}}, /* Very small -number. */

  /* Normals. */ 
  {{.ft = {0x1U, 0x1U, 0}},   {0}}, /* Small number. */
  {{.ft = {0x1U, 0x1U, 1}},   {-0}}, /* Small -number. */
  {{.ft = {0xFFFFU, 0xFEU, 0}},   {1.71470391173844543952920686828913164288E+38}}, /* Big number. */
  {{.ft = {0xFFFFU, 0xFEU, 1}},   {-1.71470391173844543952920686828913164288E+38}}, /* Big -number. */

  /* Infs. */
  {{.ft = {0x0U, 0xFFU, 0}},   {.ft = {0x0U, 0xFFU, 0}}}, /* Inf */
  {{.ft = {0x0U, 0xFFU, 1}},   {.ft = {0x0U, 0xFFU, 1}}}, /* -Inf */

  /* NaNs. */
  {{.ft = {0x1U, 0xFFU, 0}},   {.ft = {0x1U, 0xFFU, 0}}}, /* SNaN */
  {{.ft = {0x1U, 0xFFU, 1}},   {.ft = {0x1U, 0xFFU, 1}}}, /* -SNaN */
  {{.ft = {0x7FFFFFU, 0xFFU, 0}},   {.ft = {0x7FFFFFU, 0xFFU, 0}}}, /* QNaN */
  {{.ft = {0x7FFFFFU, 0xFFU, 1}},   {.ft = {0x7FFFFFU, 0xFFU, 1}}}, /* -QNaN */

  /* Numbers. */
  {{.ft = {0x490FDBU, 0x80U, 0}},  {+3}}, /* PI */
  {{.ft = {0x490FDBU, 0x80U, 1}},  {-3}}, /* -PI */

  {{.ft = {0x700000U, 0x7FU, 0}},  {+2}}, /* 1.875000 */
  {{.ft = {0x700000U, 0x7FU, 1}},  {-2}}, /* -1.875000 */
  {{.ft = {0x500000U, 0x7FU, 0}},  {+2}}, /* 1.625000 */
  {{.ft = {0x500000U, 0x7FU, 1}},  {-2}}, /* -1.625000 */
  {{.ft = {0x40000FU, 0x7FU, 0}},  {+2}}, /* 1.500002 */
  {{.ft = {0x40000FU, 0x7FU, 1}},  {-2}}, /* -1.500002 */
  {{.ft = {0x400000U, 0x7FU, 0}},  {+2}}, /* 1.500000 */
  {{.ft = {0x400000U, 0x7FU, 1}},  {-2}}, /* -1.500000 */
  {{.ft = {0x3FFFF0U, 0x7FU, 0}},  {+1}}, /* 1.499998 */
  {{.ft = {0x3FFFF0U, 0x7FU, 1}},  {-1}}, /* -1.499998 */
  {{.ft = {0x300000U, 0x7FU, 0}},  {+1}}, /* 1.375000 */
  {{.ft = {0x300000U, 0x7FU, 1}},  {-1}}, /* -1.375000 */
  {{.ft = {0x100000U, 0x7FU, 0}},  {+1}}, /* 1.125000 */
  {{.ft = {0x100000U, 0x7FU, 1}},  {-1}}, /* -1.125000 */

  {{.ft = {0x000000U, 0x7FU + 0x16U, 0}},  {+4194304}}, /* 4194304.000000 */
  {{.ft = {0x000000U, 0x7FU + 0x16U, 1}},  {-4194304}}, /* -4194304.000000 */ 
  {{.ft = {0x000000U, 0x7FU + 0x17U, 0}},  {+8388608}}, /* 8388608.000000 */
  {{.ft = {0x000000U, 0x7FU + 0x17U, 1}},  {-8388608}}, /* -8388608.000000 */
  {{.ft = {0x000000U, 0x7FU + 0x18U, 0}},  {+16777216}}, /* 16777216.000000 */
  {{.ft = {0x000000U, 0x7FU + 0x18U, 1}},  {-16777216}}, /* -16777216.000000 */

  {{.ft = {0x000000U, 0x7FU + 0x1EU, 0}},  {+1073741824}}, /* 1073741824.000000 */
  {{.ft = {0x000000U, 0x7FU + 0x1EU, 1}},  {-1073741824}}, /* -1073741824.000000 */
  {{.ft = {0x000000U, 0x7FU + 0x1FU, 0}},  {+2147483648LL}}, /* 2147483648.000000 */
  {{.ft = {0x000000U, 0x7FU + 0x1FU, 1}},  {-2147483648LL}}, /* -2147483648.000000 */
  {{.ft = {0x000000U, 0x7FU + 0x20U, 0}},  {4294967296}}, /* 4294967296.000000 */
  {{.ft = {0x000000U, 0x7FU + 0x20U, 1}},  {-4294967296}}, /* -4294967296.000000 */

  /* Different mantissa patterns. */
  {{.ft = {0x7FFFFFU, 0x96U, 0}},  {+16777215}}, /* 16777215.000000 */
  {{.ft = {0x7FF000U, 0x95U, 0}},  {+8386560}}, /* 8386560.000000 */
  {{.ft = {0x1555FFU, 0x8DU, 0}},  {+19115}}, /* 19115.000000 */
  {{.ft = {0x7FF000U, 0x96U, 1}},  {-16773120}}, /* -16773120.000000 */
  {{.ft = {0x7FFFFEU, 0x95U, 1}},  {-8388607}}, /* -8388607.000000 */
  {{.ft = {0x1555FFU, 0x8DU, 1}},  {-19115}}  /* -19115.000000 */
};

static const size_t n_tests_float = sizeof(tests_float) / sizeof(tests_float[0]);


int roundf_test(void)
{
  unsigned int i, counter;

  for (counter = i = 0; i < n_tests_float; i++)
  {
    float result = roundf(tests_float[i].value.f);

    if (tests_float[i].should_be.f == result)
      counter++;
    else if ((i >= 10 || i <= 13) && isnan(result))
      counter++;
    else
      printf("roundf test failed:  value to round = %.12f  result = %.12f  should be = %.12f\n", tests_float[i].value.f, result, tests_float[i].should_be.f);
  }
  printf("%s\n", (counter < n_tests_float) ? "roundf test failed." : "roundf test succeded.");

  return (counter < n_tests_float) ? 1 : 0;
}
