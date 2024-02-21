/* Copyright (C) 2013 DJ Delorie, see COPYING.DJ for details */

/*  Shall give the same results than /djgpp/tests/libc/c99/math/llrint-t.c  */


#include "main-t.h"

typedef struct {
  const _double_union_t value;    /* test value */
  const long long int should_be;  /* result */
} entry_t;

static const entry_t tests_double[] =
{
  /* test value */
  /*     value           should be   */

  /* Zeros. */
  {{.dt = {0x0U, 0x0U, 0x0U, 0}},   0}, /* 0.0 */
  {{.dt = {0x0U, 0x0U, 0x0U, 1}},   0}, /* -0.0 */

  /* Subnormals aka denormals. */
  {{.dt = {0x1U, 0x0U, 0x0U, 0}},   0}, /* Very small number. */
  {{.dt = {0x1U, 0x0U, 0x0U, 1}},   0}, /* Very small -number. */

  /* Normals. */
  {{.dt = {0x1U, 0x0U, 0x1U, 0}},   0}, /* Small number. */
  {{.dt = {0x1U, 0x0U, 0x1U, 1}},   0}, /* Small -number. */
  {{.dt = {0xFFFFFFFFU, 0x7FFFFU, 0x7FEU, 0}},   -9.223372036854775808E18}, /* Big number. */
  {{.dt = {0xFFFFFFFFU, 0x7FFFFU, 0x7FEU, 1}},   -9.223372036854775808E18}, /* Big -number. */

  /* Infs. */
  {{.dt = {0x0U, 0x0U, 0x7FFU, 0}},   -9.223372036854775808E18}, /* Inf */
  {{.dt = {0x0U, 0x0U, 0x7FFU, 1}},   -9.223372036854775808E18}, /* -Inf */

  /* NaNs. */
  {{.dt = {0x1U, 0x0U, 0x7FFU, 0}},   -9.223372036854775808E18}, /* SNaN */
  {{.dt = {0x1U, 0x0U, 0x7FFU, 1}},   -9.223372036854775808E18}, /* -SNaN */
  {{.dt = {0x0U, 0xFFFFFU, 0x7FFU, 1}},   -9.223372036854775808E18}, /* QNaN */
  {{.dt = {0x0U, 0xFFFFFU, 0x7FFU, 0}},   -9.223372036854775808E18}, /* -QNaN */


  /* Number. */
  {{.dt = {0x54442D18U, 0x921FBU, 0x3FFU + 0x001U, 0}},   +3}, /* PI */
  {{.dt = {0x54442D18U, 0x921FBU, 0x3FFU + 0x001U, 1}},   -3}, /* -PI */

  {{.dt = {0x00000000U, 0xE0000U, 0x3FFU + 0x0U, 0}},   +2}, /* 1.875000 */
  {{.dt = {0x00000000U, 0xE0000U, 0x3FFU + 0x0U, 1}},   -2}, /* -1.875000 */
  {{.dt = {0x00000000U, 0xA0000U, 0x3FFU + 0x0U, 0}},   +2}, /* 1.625000 */
  {{.dt = {0x00000000U, 0xA0000U, 0x3FFU + 0x0U, 1}},   -2}, /* -1.625000 */
  {{.dt = {0x18DEF417U, 0x80002U, 0x3FFU + 0x0U, 0}},   +2}, /* 1.500002 */
  {{.dt = {0x18DEF417U, 0x80002U, 0x3FFU + 0x0U, 1}},   -2}, /* -1.500002 */
  {{.dt = {0x00000000U, 0x80000U, 0x3FFU + 0x0U, 0}},   +2}, /* 1.500000 */
  {{.dt = {0x00000000U, 0x80000U, 0x3FFU + 0x0U, 1}},   -2}, /* -1.500000 */
  {{.dt = {0xE7210BE9U, 0x7F9FDU, 0x3FFU + 0x0U, 0}},   +1}, /* 1.499998 */
  {{.dt = {0xE7210BE9U, 0x7F9FDU, 0x3FFU + 0x0U, 1}},   -1}, /* -1.499998 */
  {{.dt = {0x00000000U, 0x60000U, 0x3FFU + 0x0U, 0}},   +1}, /* 1.375000 */
  {{.dt = {0x00000000U, 0x60000U, 0x3FFU + 0x0U, 1}},   -1}, /* -1.375000 */
  {{.dt = {0x00000000U, 0x20000U, 0x3FFU + 0x0U, 0}},   +1}, /* 1.125000 */
  {{.dt = {0x00000000U, 0x20000U, 0x3FFU + 0x0U, 1}},   -1}, /* -1.125000 */

  {{.dt = {0x00000000U, 0x00000U, 0x3FFU + 0x016U, 0}},   +4194304}, /* 4194304.000000 */
  {{.dt = {0x00000000U, 0x00000U, 0x3FFU + 0x016U, 1}},   -4194304}, /* -4194304.000000 */
  {{.dt = {0x00000000U, 0x00000U, 0x3FFU + 0x017U, 0}},   +8388608}, /* 8388608.000000 */
  {{.dt = {0x00000000U, 0x00000U, 0x3FFU + 0x017U, 1}},   -8388608}, /* -8388608.000000 */
  {{.dt = {0x00000000U, 0x00000U, 0x3FFU + 0x018U, 0}},   +16777216}, /* 16777216.000000 */
  {{.dt = {0x00000000U, 0x00000U, 0x3FFU + 0x018U, 1}},   -16777216}, /* -16777216.000000 */
  {{.dt = {0x00000000U, 0x00000U, 0x3FFU + 0x01EU, 0}},   +1073741824}, /* 1073741824.000000 */
  {{.dt = {0x00000000U, 0x00000U, 0x3FFU + 0x01EU, 1}},   -1073741824}, /* -1073741824.000000 */
  {{.dt = {0xFFC00000U, 0xFFFFFU, 0x3FFU + 0x01EU, 0}},   +2147483647LL}, /* 2147483647.000000 */
  {{.dt = {0x00000000U, 0x00000U, 0x3FFU + 0x01FU, 1}},   -2147483648LL}, /* -2147483648.000000 */
  {{.dt = {0x00000000U, 0x00000U, 0x3FFU + 0x020U, 0}},   +4294967296}, /* 4294967296.000000 */
  {{.dt = {0x00000000U, 0x00000U, 0x3FFU + 0x020U, 1}},   -4294967296}, /* -4294967296.000000 */
  {{.dt = {0xACF13400U, 0x02468U, 0x3FFU + 0x033U, 0}},   2271815812028928}, /* 2271815812028928.000000 */
  {{.dt = {0xACF13400U, 0x02468U, 0x3FFU + 0x033U, 1}},   -2271815812028928}, /* -2271815812028928.000000 */
  {{.dt = {0x56789AB0U, 0x01234U, 0x3FFU + 0x034U, 0}},   4523615625714352}, /* 4523615625714352.000000 */
  {{.dt = {0x56789AB0U, 0x01234U, 0x3FFU + 0x034U, 1}},   -4523615625714352}, /* -4523615625714352.000000 */
  {{.dt = {0xA9876543U, 0xFEDCBU, 0x3FFU + 0x034U, 0}},   8987183256397123}, /* 8987183256397123.000000 */
  {{.dt = {0xA9876543U, 0xFEDCBU, 0x3FFU + 0x034U, 1}},   -8987183256397123}, /* -8987183256397123.000000 */
  {{.dt = {0x00000000U, 0x00000U, 0x3FFU + 0x035U, 0}},   9007199254740992}, /* 9007199254740992.000000 */
  {{.dt = {0x00000000U, 0x00000U, 0x3FFU + 0x035U, 1}},   -9007199254740992}, /* -9007199254740992.000000 */
  {{.dt = {0x6789ABCEU, 0x12345U, 0x3FFU + 0x035U, 0}},   9647711201744796}, /* 9647711201744796.000000 */
  {{.dt = {0x6789ABCEU, 0x12345U, 0x3FFU + 0x035U, 1}},   -9647711201744796} /* -9647711201744796.000000 */
};

static const size_t n_tests_double = sizeof(tests_double) / sizeof(tests_double[0]);


int llrint_test(void)
{
  unsigned int i, counter;

  for (counter = i = 0; i < n_tests_double; i++)
  {
    long long int result = llrint(tests_double[i].value.d);

    if (tests_double[i].should_be == result)
      counter++;
    else
      printf("llrint test failed:  value to round = %.12f  result = %lld  should be = %lld\n", tests_double[i].value.d, result, tests_double[i].should_be);
  }
  printf("%s\n", (counter < n_tests_double) ? "llrint test failed." : "llrint test succeded.");

  return (counter < n_tests_double) ? 1 : 0;
}
