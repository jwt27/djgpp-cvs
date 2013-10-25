/* Copyright (C) 2013 DJ Delorie, see COPYING.DJ for details */


#include <stdio.h>
#include <math.h>
#include <libc/ieee.h>

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
  {{.dt = {0x6789ABCEU, 0x12345U, 0x3FFU + 0x035U, 1}},   -9647711201744796}, /* -9647711201744796.000000 */

  {{.dt = {0x00000000U, 0x00000U, 0x3FFU + 0x03FU, 0}},   9223372036854775808ULL}, /* 9223372036854775808.0000000000 */
  {{.dt = {0x00000000U, 0x00000U, 0x3FFU + 0x03FU, 1}},   -9.223372036854775808E+18}, /* -9223372036854775808.0000000000 */

  {{.dt = {0x00000000U, 0x00000U, 0x3FFU + 0x03EU, 0}},   4611686018427387904}, /* 4611686018427387904.0000000000 */
  {{.dt = {0x00000000U, 0x00000U, 0x3FFU + 0x03EU, 1}},   -4611686018427387904}, /* -4611686018427387904.0000000000 */
  {{.dt = {0x00000000U, 0x00000U, 0x3FFU + 0x034U, 0}},   4503599627370496}, /* 4503599627370496.0000000000 */
  {{.dt = {0x00000000U, 0x00000U, 0x3FFU + 0x034U, 1}},   -4503599627370496}, /* -4503599627370496.0000000000 */

  {{.dt = {0x00000000U, 0x00000U, 0x3FFU + 0x033U, 0}},   2251799813685248}, /* 2251799813685248.0000000000 */
  {{.dt = {0x00000000U, 0x00000U, 0x3FFU + 0x033U, 1}},   -2251799813685248}, /* -2251799813685248.0000000000 */
  {{.dt = {0x00000001U, 0x00000U, 0x3FFU + 0x033U, 0}},   2251799813685249}, /* 2251799813685248.5000000000 */
  {{.dt = {0x00000001U, 0x00000U, 0x3FFU + 0x033U, 1}},   -2251799813685249}, /* -2251799813685248.5000000000 */
  {{.dt = {0x00000002U, 0x00000U, 0x3FFU + 0x033U, 0}},   2251799813685249}, /* 2251799813685249.7500000000 */
  {{.dt = {0x00000002U, 0x00000U, 0x3FFU + 0x033U, 1}},   -2251799813685249}, /* -2251799813685249.7500000000 */

  {{.dt = {0x00000000U, 0xC0000U, 0x3FFU + 0x000U, 0}},   2}, /* 1.7500000000 */
  {{.dt = {0x00000000U, 0xC0000U, 0x3FFU + 0x000U, 1}},   -2}, /* -1.7500000000 */
  {{.dt = {0x00000000U, 0x80000U, 0x3FFU + 0x000U, 0}},   2}, /* 1.5000000000 */
  {{.dt = {0x00000000U, 0x80000U, 0x3FFU + 0x000U, 1}},   -2}, /* -1.5000000000 */
  {{.dt = {0x00000000U, 0x40000U, 0x3FFU + 0x000U, 0}},   1}, /* 1.2500000000 */
  {{.dt = {0x00000000U, 0x40000U, 0x3FFU + 0x000U, 1}},   -1}, /* -1.2500000000 */
  {{.dt = {0x00000000U, 0x00000U, 0x3FFU + 0x000U, 0}},   1}, /* 1.0000000000 */
  {{.dt = {0x00000000U, 0x00000U, 0x3FFU + 0x000U, 1}},   -1}, /* -1.0000000000 */
  {{.dt = {0x00000000U, 0x80000U, 0x3FFU + 0xFFFFFFFFU, 0}},   1}, /* 0.7500000000 */
  {{.dt = {0x00000000U, 0x80000U, 0x3FFU + 0xFFFFFFFFU, 1}},   -1}, /* -0.7500000000 */
  {{.dt = {0x00000000U, 0x00000U, 0x3FFU + 0xFFFFFFFFU, 0}},   1}, /* 0.5000000000 */
  {{.dt = {0x00000000U, 0x00000U, 0x3FFU + 0xFFFFFFFFU, 1}},   -1}, /* -0.5000000000 */
  {{.dt = {0x00000000U, 0x00000U, 0x3FFU + 0xFFFFFFFEU, 0}},   0}, /* 0.2500000000 */
  {{.dt = {0x00000000U, 0x00000U, 0x3FFU + 0xFFFFFFFEU, 1}},   -0}, /* -0.2500000000 */
  {{.dt = {0xF8FAB5E6U, 0x948B0U, 0x3FFU + 0xFFFFFFF9U, 0}},   0}, /* 0.0123456789 */
  {{.dt = {0xF8FAB5E6U, 0x948B0U, 0x3FFU + 0xFFFFFFF9U, 1}},   -0}, /* -0.0123456789 */
  {{.dt = {0x00000000U, 0x00000U, 0x3FFU + 0xFFFFFC01U, 0}},   0}, /* 0.0000000000 */
  {{.dt = {0x00000000U, 0x00000U, 0x3FFU + 0xFFFFFC01U, 1}},   -0}, /* -0.0000000000 */

  {{.dt = {0x00000000U, 0x00000U, 0x3FFU + 0x014U, 0}},   1048576}, /* 1048576.0000000000 */
  {{.dt = {0x00000000U, 0x00000U, 0x3FFU + 0x014U, 1}},   -1048576}, /* -1048576.0000000000 */
  {{.dt = {0x40000000U, 0x00000U, 0x3FFU + 0x014U, 0}},   1048576}, /* 1048576.2500000000 */
  {{.dt = {0x40000000U, 0x00000U, 0x3FFU + 0x014U, 1}},   -1048576}, /* -1048576.2500000000 */
  {{.dt = {0x80000000U, 0x00000U, 0x3FFU + 0x014U, 0}},   1048577}, /* 1048576.5000000000 */
  {{.dt = {0x80000000U, 0x00000U, 0x3FFU + 0x014U, 1}},   -1048577}, /* -1048576.5000000000 */
  {{.dt = {0xC0000000U, 0x00000U, 0x3FFU + 0x014U, 0}},   1048577}, /* 1048576.7500000000 */
  {{.dt = {0xC0000000U, 0x00000U, 0x3FFU + 0x014U, 1}},   -1048577}, /* -1048576.7500000000 */

  {{.dt = {0x00000000U, 0x00000U, 0x3FFU + 0x015U, 0}},   2097152}, /* 2097152.0000000000 */
  {{.dt = {0x00000000U, 0x00000U, 0x3FFU + 0x015U, 1}},   -2097152}, /* -2097152.0000000000 */
  {{.dt = {0x20000000U, 0x00000U, 0x3FFU + 0x015U, 0}},   2097152}, /* 2097152.2500000000 */
  {{.dt = {0x20000000U, 0x00000U, 0x3FFU + 0x015U, 1}},   -2097152}, /* -2097152.2500000000 */
  {{.dt = {0x40000000U, 0x00000U, 0x3FFU + 0x015U, 0}},   2097153}, /* 2097152.5000000000 */
  {{.dt = {0x40000000U, 0x00000U, 0x3FFU + 0x015U, 1}},   -2097153}, /* -2097152.5000000000 */
  {{.dt = {0x60000000U, 0x00000U, 0x3FFU + 0x015U, 0}},   2097153}, /* 2097152.7500000000 */
  {{.dt = {0x60000000U, 0x00000U, 0x3FFU + 0x015U, 1}},   -2097153}, /* -2097152.7500000000 */

  /*  Number greater than 2**63 exceeds long int size and will be casted in an implementation defined manner.  */
  {{.dt = {0x00000001U, 0x00000U, 0x3FFU + 0x03FU, 0}},   -9.223372036854775808E+18}, /* 9223372036854777856.0000000000 */
  {{.dt = {0x00000001U, 0x00000U, 0x3FFU + 0x03FU, 1}},   -9.223372036854775808E+18}, /* -9223372036854777856.0000000000 */

  /*  Number less than 0.5 will be truncated to 0.  */
  {{.dt = {0x1FF885F3U, 0xFF793U, 0x3FFU + 0xFFFFFFFEU, 0}},   0}, /* 0.4994857609 */
  {{.dt = {0x1FF885F3U, 0xFF793U, 0x3FFU + 0xFFFFFFFEU, 1}},   -0}, /* -0.4994857609 */
  {{.dt = {0xD2F1A9FCU, 0x0624DU, 0x3FFU + 0xFFFFFFF6U, 0}},   0}, /* 0.0010000000 */
  {{.dt = {0xD2F1A9FCU, 0x0624DU, 0x3FFU + 0xFFFFFFF6U, 1}},   -0}, /* -0.0010000000 */

  /*  Number greater than 0.5 and less than 1 will be rounded to 1.  */
  {{.dt = {0x8FB86F48U, 0xF9ADCU, 0x3FFU + 0xFFFFFFFFU, 0}},   1}, /* 0.9876540000 */
  {{.dt = {0x8FB86F48U, 0xF9ADCU, 0x3FFU + 0xFFFFFFFFU, 1}},   -1}, /* -0.9876540000 */
  {{.dt = {0x20365653U, 0x00000U, 0x3FFU + 0xFFFFFFFFU, 0}},   1}, /* 0.5000000600 */
  {{.dt = {0x20365653U, 0x00000U, 0x3FFU + 0xFFFFFFFFU, 1}},   -1}, /* -0.5000000600 */
  {{.dt = {0x00000000U, 0x00000U, 0x3FFU + 0xFFFFFFFFU, 0}},   1}, /* 0.5000000000 */
  {{.dt = {0x00000000U, 0x00000U, 0x3FFU + 0xFFFFFFFFU, 1}},   -1}, /* -0.5000000000 */

  /*  Number greather than 1 and less than 2**63 will be rounded accordingly.  */
  {{.dt = {0x00000000U, 0x00000U, 0x3FFU + 0x000U, 0}},   1}, /* 1.0000000000 */
  {{.dt = {0x00000000U, 0x00000U, 0x3FFU + 0x000U, 1}},   -1}, /* -1.0000000000 */
  {{.dt = {0x18DEF417U, 0x00002U, 0x3FFU + 0x000U, 0}},   1}, /* 1.0000020000 */
  {{.dt = {0x18DEF417U, 0x00002U, 0x3FFU + 0x000U, 1}},   -1}, /* -1.0000020000 */
  {{.dt = {0x00000000U, 0x00003U, 0x3FFU + 0x010U, 0}},   65536}, /* 65536.1875000000 */
  {{.dt = {0x00000000U, 0x00003U, 0x3FFU + 0x010U, 1}},   -65536}, /* -65536.1875000000 */
  {{.dt = {0x00000000U, 0x00008U, 0x3FFU + 0x010U, 0}},   65537}, /* 65536.5000000000 */
  {{.dt = {0x00000000U, 0x00008U, 0x3FFU + 0x010U, 1}},   -65537}, /* -65536.5000000000 */
  {{.dt = {0xA0000000U, 0x00009U, 0x3FFU + 0x010U, 0}},   65537}, /* 65536.6015625000 */
  {{.dt = {0xA0000000U, 0x00009U, 0x3FFU + 0x010U, 1}},   -65537}, /* -65536.6015625000 */
  {{.dt = {0xE0000000U, 0xFFFFFU, 0x3FFU + 0x016U, }},    8388608}, /* 8388607.5000000000 */
  {{.dt = {0xE0000000U, 0xFFFFFU, 0x3FFU + 0x016U, 1}},   -8388608}, /* -8388607.5000000000 */
  {{.dt = {0xA0000000U, 0x00000U, 0x3FFU + 0x014U, 0}},   1048577}, /* 1048576.6250000000 */
  {{.dt = {0xA0000000U, 0x00000U, 0x3FFU + 0x014U, 1}},   -1048577}, /* -1048576.6250000000 */
  {{.dt = {0xFFC00000U, 0xFFFFFU, 0x3FFU + 0x01EU, 0}},   2147483647}, /* 2147483647.0000000000 */
  {{.dt = {0xFFC00000U, 0xFFFFFU, 0x3FFU + 0x01EU, 1}},   -2147483647}, /* -2147483647.0000000000 */
  {{.dt = {0x00000000U, 0x00000U, 0x3FFU + 0x034U, 0}},   4503599627370496}, /* 4503599627370496.0000000000 */
  {{.dt = {0x00000000U, 0x00000U, 0x3FFU + 0x034U, 1}},   -4503599627370496}, /* -4503599627370496.0000000000 */
  {{.dt = {0xFFFFFFFEU, 0xFFFFFU, 0x3FFU + 0x03EU, 0}},   9223372036854773760}, /* 9223372036854773760.0000000000 */
  {{.dt = {0xFFFFFFFEU, 0xFFFFFU, 0x3FFU + 0x03EU, 1}},   -9223372036854773760}, /* -9223372036854773760.0000000000 */
};

static const size_t n_tests_double = sizeof(tests_double) / sizeof(tests_double[0]);


int main(void)
{
  unsigned int i, counter;

  for (counter = i = 0; i < n_tests_double; i++)
  {
    long long int result = llround(tests_double[i].value.d);

    if (tests_double[i].should_be == result)
      counter++;
    else
      printf("llround test failed:  value to round = %.6g  result = %lld  should be = %lld\n", tests_double[i].value.d, result, tests_double[i].should_be);
  }
  printf("%s\n", (counter < n_tests_double) ? "llround test failed." : "llround test succeded.");

  return 0;
}
