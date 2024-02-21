/* Copyright (C) 2013 DJ Delorie, see COPYING.DJ for details */

/*  Shall give the same results than /djgpp/tests/cygnus/lrintl-t.c  */


#include <stdio.h>
#include <math.h>
#include <libc/ieee.h>

typedef struct {
  const _longdouble_union_t value;  /* test value */
  const long int should_be;         /* result */
} entry_t;

static const entry_t tests_long_double[] =
{
  /* test value */
  /*     value           should be   */

  /* Zeros. */
  {{.ldt = {0x0U, 0x0U, 0x0U, 0}},   0}, /* 0.0 */
  {{.ldt = {0x0U, 0x0U, 0x0U, 1}},   0}, /* -0.0 */


  /* Subnormals aka denormals. */
  {{.ldt = {0x1U, 0x0U, 0x0U, 0}},   0}, /* Very small number. */
  {{.ldt = {0x1U, 0x0U, 0x0U, 1}},   0}, /* Very small -number. */

  /* Normals. */
  {{.ldt = {0x0U, 0x80000000U, 0x1U, 0}},   0}, /* Small number. */
  {{.ldt = {0x0U, 0x80000000U, 0x1U, 1}},   0}, /* Small -number. */
  {{.ldt = {0xFFFFFFFFU, 0xFFFFFFFFU, 0x7FFEU, 0}},   -2147483648UL}, /* Big number. */
  {{.ldt = {0xFFFFFFFFU, 0xFFFFFFFFU, 0x7FFEU, 1}},   -2147483648UL}, /* Big -number. */

  /* Infs. */
  {{.ldt = {0x0U, 0x80000000U, 0x7FFFU, 0}},   -2147483648UL}, /* Inf */
  {{.ldt = {0x0U, 0x80000000U, 0x7FFFU, 1}},   -2147483648UL}, /* -Inf */

  /* NaNs. */
  {{.ldt = {0x1U, 0x80000000U, 0x7FFFU, 0}},   -2147483648UL}, /* SNaN */
  {{.ldt = {0x1U, 0x80000000U, 0x7FFFU, 1}},   -2147483648UL}, /* -SNaN */
  {{.ldt = {0x0U, 0xFFFFFFFFU, 0x7FFFU, 0}},   -2147483648UL}, /* QNaN */
  {{.ldt = {0x0U, 0xFFFFFFFFU, 0x7FFFU, 1}},   -2147483648UL}, /* -QNaN */

  /* Number. */
  {{.ldt = {0x2168C000U, 0xC90FDAA2U, 0x3FFFU + 0x0001U, 0}},   +3}, /* PI */
  {{.ldt = {0x2168C000U, 0xC90FDAA2U, 0x3FFFU + 0x0001U, 1}},   -3}, /* -PI */


  {{.ldt = {0x00000000U, 0xF0000000U, 0x3FFFU + 0x0U, 0}},   +2}, /* 1.875000 */
  {{.ldt = {0x00000000U, 0xF0000000U, 0x3FFFU + 0x0U, 1}},   -2}, /* -1.875000 */
  {{.ldt = {0x00000000U, 0xD0000000U, 0x3FFFU + 0x0U, 0}},   +2}, /* 1.625000 */
  {{.ldt = {0x00000000U, 0xD0000000U, 0x3FFFU + 0x0U, 1}},   -2}, /* -1.625000 */
  {{.ldt = {0xF7A0B800U, 0xC00010C6U, 0x3FFFU + 0x0U, 0}},   +2}, /* 1.500002 */
  {{.ldt = {0xF7A0B800U, 0xC00010C6U, 0x3FFFU + 0x0U, 1}},   -2}, /* -1.500002 */
  {{.ldt = {0x00000000U, 0xC0000000U, 0x3FFFU + 0x0U, 0}},   +2}, /* 1.500000 */
  {{.ldt = {0x00000000U, 0xC0000000U, 0x3FFFU + 0x0U, 1}},   -2}, /* -1.500000 */
  {{.ldt = {0x085F4800U, 0xBFFFEF39U, 0x3FFFU + 0x0U, 0}},   +1}, /* 1.499998 */
  {{.ldt = {0x085F4800U, 0xBFFFEF39U, 0x3FFFU + 0x0U, 1}},   -1}, /* -1.499998 */
  {{.ldt = {0x00000000U, 0xB0000000U, 0x3FFFU + 0x0U, 0}},   +1}, /* 1.375000 */
  {{.ldt = {0x00000000U, 0xB0000000U, 0x3FFFU + 0x0U, 1}},   -1}, /* -1.375000 */
  {{.ldt = {0x00000000U, 0x90000000U, 0x3FFFU + 0x0U, 0}},   +1}, /* 1.125000 */
  {{.ldt = {0x00000000U, 0x90000000U, 0x3FFFU + 0x0U, 1}},   -1}, /* -1.125000 */

  {{.ldt = {0x00000000U, 0x80000000U, 0x3FFFU + 0x0016U, 0}},   +4194304}, /* 4194304.000000 */
  {{.ldt = {0x00000000U, 0x80000000U, 0x3FFFU + 0x0016U, 1}},   -4194304}, /* -4194304.000000 */
  {{.ldt = {0x00000000U, 0x80000000U, 0x3FFFU + 0x0017U, 0}},   +8388608}, /* 8388608.000000 */
  {{.ldt = {0x00000000U, 0x80000000U, 0x3FFFU + 0x0017U, 1}},   -8388608}, /* -8388608.000000 */
  {{.ldt = {0x00000000U, 0x80000000U, 0x3FFFU + 0x0018U, 0}},   +16777216}, /* 16777216.000000 */
  {{.ldt = {0x00000000U, 0x80000000U, 0x3FFFU + 0x0018U, 1}},   -16777216}, /* -16777216.000000 */
  {{.ldt = {0x00000000U, 0x80000000U, 0x3FFFU + 0x001EU, 0}},   +1073741824}, /* 1073741824.000000 */
  {{.ldt = {0x00000000U, 0x80000000U, 0x3FFFU + 0x001EU, 1}},   -1073741824}, /* -1073741824.000000 */
  {{.ldt = {0x00000000U, 0xFFFFFFFEU, 0x3FFFU + 0x001EU, 0}},   +2147483647LL}, /* 2147483647.000000 */
  {{.ldt = {0x00000000U, 0x80000000U, 0x3FFFU + 0x001FU, 1}},   -2147483648UL}, /* -2147483648.000000 */
  {{.ldt = {0x00000000U, 0x80000000U, 0x3FFFU + 0x0020U, 0}},   -2147483648UL}, /* 4294967296.000000 */
  {{.ldt = {0x00000000U, 0x80000000U, 0x3FFFU + 0x0020U, 1}},   -2147483648UL}  /* -4294967296.000000 */
};

static const size_t n_tests_long_double = sizeof(tests_long_double) / sizeof(tests_long_double[0]);



int main(void)
{
  unsigned int i, counter;

  for (counter = i = 0; i < n_tests_long_double; i++)
  {
    long int result = lrintl(tests_long_double[i].value.ld);

    if (tests_long_double[i].should_be == result)
      counter++;
    else
      printf("lrintl test failed:  value to round = %.6Lg  result = %ld  should be = %ld\n", tests_long_double[i].value.ld, result, tests_long_double[i].should_be);
  }
  printf("%s\n", (counter < n_tests_long_double) ? "lrintl test failed." : "lrintl test succeded.");

  return 0;
}
