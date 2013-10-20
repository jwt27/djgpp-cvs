/* Copyright (C) 2013 DJ Delorie, see COPYING.DJ for details */

/*  Shall give the same results than /djgpp/tests/cygnus/t-lrint.c  */


#include <stdio.h>
#include <math.h>
#include <libc/ieee.h>

typedef struct {
  const _double_union_t value;  /* test value */
  const long int should_be;     /* result */
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
  {{.dt = {0xFFFFFFFFU, 0x7FFFFU, 0x7FEU, 0}},   -2147483648UL}, /* Big number. */
  {{.dt = {0xFFFFFFFFU, 0x7FFFFU, 0x7FEU, 1}},   -2147483648UL}, /* Big -number. */

  /* Infs. */
  {{.dt = {0x0U, 0x0U, 0x7FFU, 0}},   -2147483648UL}, /* Inf */
  {{.dt = {0x0U, 0x0U, 0x7FFU, 1}},   -2147483648UL}, /* -Inf */

  /* NaNs. */
  {{.dt = {0x1U, 0x0U, 0x7FFU, 0}},   -2147483648UL}, /* SNaN */
  {{.dt = {0x1U, 0x0U, 0x7FFU, 1}},   -2147483648UL}, /* -SNaN */
  {{.dt = {0x0U, 0xFFFFFU, 0x7FFU, 1}},   -2147483648UL}, /* QNaN */
  {{.dt = {0x0U, 0xFFFFFU, 0x7FFU, 0}},   -2147483648UL}, /* -QNaN */


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
  {{.dt = {0x00000000U, 0x00000U, 0x3FFU + 0x01FU, 1}},   -2147483648UL}, /* -2147483648.000000 */
  {{.dt = {0x00000000U, 0x00000U, 0x3FFU + 0x020U, 0}},   -2147483648UL}, /* 4294967296.000000 */
  {{.dt = {0x00000000U, 0x00000U, 0x3FFU + 0x020U, 1}},   -2147483648UL} /* -4294967296.000000 */
};

static const size_t n_tests_double = sizeof(tests_double) / sizeof(tests_double[0]);


int main(void)
{
  unsigned int i, counter;

  for (counter = i = 0; i < n_tests_double; i++)
  {
    long int result = lrint(tests_double[i].value.d);

    if (tests_double[i].should_be == result)
      counter++;
    else
      printf("lrint test failed:  value to round = %.6g  result = %ld  should be = %ld\n", tests_double[i].value.d, result, tests_double[i].should_be);
  }
  printf("%s\n", (counter < n_tests_double) ? "lrint test failed." : "lrint test succeded.");

  return 0;
}
