/* Copyright (C) 2013 DJ Delorie, see COPYING.DJ for details */


#include "main-t.h"

typedef struct {
  const _longdouble_union_t value;      /* test value */
  const _longdouble_union_t should_be;  /* result */
} entry_t;

static const entry_t tests_long_double[] =
{
  /* test value */
  /*     value           should be   */

  /* Zeros. */
  {{.ldt = {0x0U, 0x0U, 0x0U, 0}},  {0}}, /* 0.0 */
  {{.ldt = {0x0U, 0x0U, 0x0U, 1}},  {0}}, /* -0.0 */

  /* Subnormals aka denormals. */
  {{.ldt = {0x1U, 0x0U, 0x0U, 0}},  {0}}, /* Very small number. */
  {{.ldt = {0x1U, 0x0U, 0x0U, 1}},  {0}}, /* Very small -number. */

  /* Normals. */
  {{.ldt = {0x0U, 0x80000000U, 0x1U, 0}},  {0}}, /* Small number. */
  {{.ldt = {0x0U, 0x80000000U, 0x1U, 1}},  {0}}, /* Small -number. */
  {{.ldt = {0xFFFFFFFFU, 0xFFFFFFFFU, 0x7FFEU, 0}},  {.ldt = {0xFFFFFFFFU, 0xFFFFFFFFU, 0x7FFEU, 0}}}, /* Big number. */
  {{.ldt = {0xFFFFFFFFU, 0xFFFFFFFFU, 0x7FFEU, 1}},  {.ldt = {0xFFFFFFFFU, 0xFFFFFFFFU, 0x7FFEU, 1}}}, /* Big -number. */

  /* Infs. */
  {{.ldt = {0x0U, 0x80000000U, 0x7FFFU, 0}},  {.ldt = {0x0U, 0x80000000U, 0x7FFFU, 0}}}, /* Inf */
  {{.ldt = {0x0U, 0x80000000U, 0x7FFFU, 1}},  {.ldt = {0x0U, 0x80000000U, 0x7FFFU, 1}}}, /* -Inf */

  /* NaNs. */
  {{.ldt = {0x1U, 0x80000000U, 0x7FFFU, 0}},  {.ldt = {0x1U, 0x80000000U, 0x7FFFU, 0}}}, /* SNaN */
  {{.ldt = {0x1U, 0x80000000U, 0x7FFFU, 1}},  {.ldt = {0x1U, 0x80000000U, 0x7FFFU, 1}}}, /* -SNaN */
  {{.ldt = {0x0U, 0xFFFFFFFFU, 0x7FFFU, 0}},  {.ldt = {0x0U, 0xFFFFFFFFU, 0x7FFFU, 0}}}, /* QNaN */
  {{.ldt = {0x0U, 0xFFFFFFFFU, 0x7FFFU, 1}},  {.ldt = {0x0U, 0xFFFFFFFFU, 0x7FFFU, 1}}}, /* -QNaN */

  /* Number. */
  {{.ldt = {0x2168C000U, 0xC90FDAA2U, 0x3FFFU + 0x0001U, 0}},  {+3}}, /* PI */
  {{.ldt = {0x2168C000U, 0xC90FDAA2U, 0x3FFFU + 0x0001U, 1}},  {-3}}, /* -PI */


  {{.ldt = {0x00000000U, 0xF0000000U, 0x3FFFU + 0x0U, 0}},  {+2}}, /* 1.875000 */
  {{.ldt = {0x00000000U, 0xF0000000U, 0x3FFFU + 0x0U, 1}},  {-2}}, /* -1.875000 */
  {{.ldt = {0x00000000U, 0xD0000000U, 0x3FFFU + 0x0U, 0}},  {+2}}, /* 1.625000 */
  {{.ldt = {0x00000000U, 0xD0000000U, 0x3FFFU + 0x0U, 1}},  {-2}}, /* -1.625000 */
  {{.ldt = {0xF7A0B800U, 0xC00010C6U, 0x3FFFU + 0x0U, 0}},  {+2}}, /* 1.500002 */
  {{.ldt = {0xF7A0B800U, 0xC00010C6U, 0x3FFFU + 0x0U, 1}},  {-2}}, /* -1.500002 */
  {{.ldt = {0x00000000U, 0xC0000000U, 0x3FFFU + 0x0U, 0}},  {+2}}, /* 1.500000 */
  {{.ldt = {0x00000000U, 0xC0000000U, 0x3FFFU + 0x0U, 1}},  {-2}}, /* -1.500000 */
  {{.ldt = {0x085F4800U, 0xBFFFEF39U, 0x3FFFU + 0x0U, 0}},  {+1}}, /* 1.499998 */
  {{.ldt = {0x085F4800U, 0xBFFFEF39U, 0x3FFFU + 0x0U, 1}},  {-1}}, /* -1.499998 */
  {{.ldt = {0x00000000U, 0xB0000000U, 0x3FFFU + 0x0U, 0}},  {+1}}, /* 1.375000 */
  {{.ldt = {0x00000000U, 0xB0000000U, 0x3FFFU + 0x0U, 1}},  {-1}}, /* -1.375000 */
  {{.ldt = {0x00000000U, 0x90000000U, 0x3FFFU + 0x0U, 0}},  {+1}}, /* 1.125000 */
  {{.ldt = {0x00000000U, 0x90000000U, 0x3FFFU + 0x0U, 1}},  {-1}}, /* -1.125000 */

  {{.ldt = {0x00000000U, 0x80000000U, 0x3FFFU + 0x0016U, 0}},  {+4194304}}, /* 4194304.000000 */
  {{.ldt = {0x00000000U, 0x80000000U, 0x3FFFU + 0x0016U, 1}},  {-4194304}}, /* -4194304.000000 */
  {{.ldt = {0x00000000U, 0x80000000U, 0x3FFFU + 0x0017U, 0}},  {+8388608}}, /* 8388608.000000 */
  {{.ldt = {0x00000000U, 0x80000000U, 0x3FFFU + 0x0017U, 1}},  {-8388608}}, /* -8388608.000000 */
  {{.ldt = {0x00000000U, 0x80000000U, 0x3FFFU + 0x0018U, 0}},  {+16777216}}, /* 16777216.000000 */
  {{.ldt = {0x00000000U, 0x80000000U, 0x3FFFU + 0x0018U, 1}},  {-16777216}}, /* -16777216.000000 */
  {{.ldt = {0x00000000U, 0x80000000U, 0x3FFFU + 0x001EU, 0}},  {+1073741824}}, /* 1073741824.000000 */
  {{.ldt = {0x00000000U, 0x80000000U, 0x3FFFU + 0x001EU, 1}},  {-1073741824}}, /* -1073741824.000000 */
  {{.ldt = {0x00000000U, 0xFFFFFFFEU, 0x3FFFU + 0x001EU, 0}},  {+2147483647LL}}, /* 2147483647.000000 */
  {{.ldt = {0x00000000U, 0x80000000U, 0x3FFFU + 0x001FU, 1}},  {-2147483648LL}}, /* -2147483648.000000 */
  {{.ldt = {0x00000000U, 0x80000000U, 0x3FFFU + 0x0020U, 0}},  {+4294967296}}, /* 4294967296.000000 */
  {{.ldt = {0x00000000U, 0x80000000U, 0x3FFFU + 0x0020U, 1}},  {-4294967296}}, /* -4294967296.000000 */

  {{.ldt = {0x89A00000U, 0x81234567U, 0x3FFFU + 0x0033U, 0}},  {2271815812028928}}, /* 2271815812028928.000000 */
  {{.ldt = {0x89A00000U, 0x81234567U, 0x3FFFU + 0x0033U, 1}},  {-2271815812028928}}, /* -2271815812028928.000000 */
  {{.ldt = {0xC4D58000U, 0x8091A2B3U, 0x3FFFU + 0x0034U, 0}},  {4523615625714352}}, /* 4523615625714352.000000 */
  {{.ldt = {0xC4D58000U, 0x8091A2B3U, 0x3FFFU + 0x0034U, 1}},  {-4523615625714352}}, /* -4523615625714352.000000 */
  {{.ldt = {0x3B2A1800U, 0xFF6E5D4CU, 0x3FFFU + 0x0034U, 0}},  {8987183256397123}}, /* 8987183256397123.000000 */
  {{.ldt = {0x3B2A1800U, 0xFF6E5D4CU, 0x3FFFU + 0x0034U, 1}},  {-8987183256397123}}, /* -8987183256397123.000000 */
  {{.ldt = {0x00000000U, 0x80000000U, 0x3FFFU + 0x0035U, 0}},  {9007199254740992}}, /* 9007199254740992.000000 */
  {{.ldt = {0x00000000U, 0x80000000U, 0x3FFFU + 0x0035U, 1}},  {-9007199254740992}}, /* -9007199254740992.000000 */
  {{.ldt = {0x4D5E7000U, 0x891A2B3CU, 0x3FFFU + 0x0035U, 0}},  {9647711201744796}}, /* 9647711201744796.000000 */
  {{.ldt = {0x4D5E7000U, 0x891A2B3CU, 0x3FFFU + 0x0035U, 1}},  {-9647711201744796}}, /* -9647711201744796.000000 */
  {{.ldt = {0x76543000U, 0xFEDCBA98U, 0x3FFFU + 0x0041U, 0}},  {7.3459034177972256768E19}}, /* 73459034177972256768.000000 */
  {{.ldt = {0x76543000U, 0xFEDCBA98U, 0x3FFFU + 0x0041U, 1}},  {-7.3459034177972256768E19}}, /* -73459034177972256768.000000 */
  {{.ldt = {0x00000000U, 0x80000000U, 0x3FFFU + 0x003FU, 0}},  {9.223372036854775808E18}}, /* 9223372036854775808.000000 */
  {{.ldt = {0x00000000U, 0x80000000U, 0x3FFFU + 0x003FU, 1}},  {-9.223372036854775808E18}}, /* -9223372036854775808.000000 */
  {{.ldt = {0x00000000U, 0xC0000000U, 0x3FFFU + 0x003FU, 0}},  {1.3835058055282163712E19}}, /* 13835058055282163712.000000 */
  {{.ldt = {0x00000000U, 0xC0000000U, 0x3FFFU + 0x003FU, 1}},  {-1.3835058055282163712E19}}, /* -13835058055282163712.000000 */
  {{.ldt = {0x00000000U, 0xF0000000U, 0x3FFFU + 0x0042U, 0}},  {1.38350580552821637120E20}}, /* 138350580552821637120.000000 */
  {{.ldt = {0x00000000U, 0xF0000000U, 0x3FFFU + 0x0042U, 1}},  {-1.38350580552821637120E20}}, /* -138350580552821637120.000000 */
  {{.ldt = {0xBA987800U, 0xF7FFFEDCU, 0x3FFFU + 0x0042U, 0}},  {1.42962256563249856512E20}}, /* 142962256563249856512.000000 */
  {{.ldt = {0xBA987800U, 0xF7FFFEDCU, 0x3FFFU + 0x0042U, 1}},  {-1.42962256563249856512E20}}, /* -142962256563249856512.000000 */
  {{.ldt = {0x00000000U, 0xF8000000U, 0x3FFFU + 0x0042U, 0}},  {1.42962266571249025024E20}}, /* 142962266571249025024.000000 */
  {{.ldt = {0x00000000U, 0xF8000000U, 0x3FFFU + 0x0042U, 1}},  {-1.42962266571249025024E20}}, /* -142962266571249025024.000000 */
  {{.ldt = {0x00012000U, 0xF8000000U, 0x3FFFU + 0x0042U, 0}},  {1.42962266571249614848E20}}, /* 142962266571249614848.000000 */
  {{.ldt = {0x00012000U, 0xF8000000U, 0x3FFFU + 0x0042U, 1}},  {-1.42962266571249614848E20}}, /* -142962266571249614848.000000 */
  {{.ldt = {0x76543000U, 0xFEDCBA98U, 0x3FFFU + 0x0042U, 0}},  {1.46918068355944513536E20}}, /* 146918068355944513536.000000 */
  {{.ldt = {0x76543000U, 0xFEDCBA98U, 0x3FFFU + 0x0042U, 1}},  {-1.46918068355944513536E20}}, /* 147573952589676396544.000000 */
  {{.ldt = {0xFFFFF800U, 0xFFFFFFFFU, 0x3FFFU + 0x0042U, 0}},  {1.47573952589676396544E20}}, /* -147573952589676396544.000000 */
  {{.ldt = {0xFFFFF800U, 0xFFFFFFFFU, 0x3FFFU + 0x0042U, 1}},  {-1.47573952589676396544E20}}, /* -147573952589676396544.000000 */
  {{.ldt = {0x00000000U, 0x80000000U, 0x3FFFU + 0x0040U, 0}},  {1.8446744073709551616E19}}, /* 18446744073709551616.000000 */
  {{.ldt = {0x00000000U, 0x80000000U, 0x3FFFU + 0x0040U, 1}},  {-1.8446744073709551616E19}},  /* -18446744073709551616.000000 */

  /* Number. */
  {{.ldt = {0x80000F0CU, 0xFFFFFFFFU, 0x3FFFU + 0x001FU, 0}},  {+4294967296}}, /* 4294967295.500001 */
  {{.ldt = {0x8FFFFF00U, 0xFFFFFFFEU, 0x3FFFU + 0x001FU, 0}},  {+4294967295UL}}, /* 4294967294.562500 */
  {{.ldt = {0x8000AA00U, 0xFFFFFFFEU, 0x3FFFU + 0x001FU, 0}},  {+4294967295UL}}, /* 4294967294.500010 */
  {{.ldt = {0x707CD001U, 0xFFFFFFFEU, 0x3FFFU + 0x001FU, 0}},  {+4294967294UL}}, /* 4294967294.439404 */
  {{.ldt = {0x7FFEB00DU, 0xFFFFFFFEU, 0x3FFFU + 0x001FU, 0}},  {+4294967294UL}}, /* 4294967294.499980 */
  {{.ldt = {0x90123007U, 0xFFFFFFFFU, 0x3FFFU + 0x001FU, 0}},  {+4294967296}}, /* 4294967295.562778 */

  {{.ldt = {0x0000FFFFU, 0xFFFFFFFFU, 0x3FFFU + 0x003EU, 0}},  {+9223372034707324928}}, /* 9223372034707324927.500000 */
  {{.ldt = {0x7777777FU, 0xFFFFFFFEU, 0x3FFFU + 0x003EU, 0}},  {+9223372033561967552}}, /* 9223372033561967551.500000 */
  {{.ldt = {0x9EEEEEE0U, 0xFFFFFFFEU, 0x3FFFU + 0x003EU, 0}},  {+9223372033893037936}}, /* 9223372033893037936.000000 */
  {{.ldt = {0xEEEEEE07U, 0xFFFFFFFEU, 0x3FFFU + 0x003EU, 0}},  {+9223372034564126468}}, /* 9223372034564126467.500000 */
  {{.ldt = {0xEEEEEE0EU, 0xFFFFFFFEU, 0x3FFFU + 0x003EU, 0}},  {+9223372034564126471}}, /* 9223372034564126471.000000 */
  {{.ldt = {0xFFFFFF0FU, 0xFFFFFFFFU, 0x3FFFU + 0x003EU, 0}},  {+9223372036854775688}} /* 9223372036854775687.500000 */
};

static const size_t n_tests_long_double = sizeof(tests_long_double) / sizeof(tests_long_double[0]);


int roundl_test(void)
{
  unsigned int i, counter;

  for (counter = i = 0; i < n_tests_long_double; i++)
  {
    long double result = roundl(tests_long_double[i].value.ld);

    if (tests_long_double[i].should_be.ld == result)
      counter++;
    else if ((i >= 10 || i <= 13) && isnan(result))
      counter++;
    else
      printf("roundl test failed:  value to roundl = %.12Lf  result = %.12Lf  should be = %.12Lf\n", tests_long_double[i].value.ld, result, tests_long_double[i].should_be.ld);
  }
  printf("%s\n", (counter < n_tests_long_double) ? "roundl test failed." : "roundl test succeded.");

  return (counter < n_tests_long_double) ? 1 : 0;
}
