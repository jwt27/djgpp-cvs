#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libc/ieee.h>

/* float tests */
static const float tests[] = {
  2.0, 20.0, 200.0, 2e38, 1e38,
  2.0e39, /* Larger than maximum float => "infinite" */
  2.0e-01, 2.0e-02, 2.0e-03, 2e-37,
  2e-38, 2e-39,
  1e-44, 1e-45
};

static const size_t n_tests = sizeof(tests) / sizeof(tests[0]);

/* float_t tests - so we can specify *exactly* what we want. */
static const float_t tests2[] = {
  /* Infinity */
  { 0U, 0xffU, 0x0 },

  /* NaN */
  { 1U, 0xffU, 0x0 }
};

static const size_t n_tests2 = sizeof(tests2) / sizeof(tests2[0]);

typedef struct {
  const long double *ld;
  const int sign;
  const char *str;
  const int overflow;
} test3_t;

static long double ld_float_max;
static long double ld_float_max_10;
static long double ld_double_max;

static const test3_t tests3[] = {
  { &ld_float_max,          1,  "FLT_MAX",       0 },
  { &ld_float_max,         -1, "-FLT_MAX",       0 },
  { &ld_float_max_10,       1,  "FLT_MAX * 10",  1 },
  { &ld_float_max_10,      -1, "-FLT_MAX * 10",  1 },
  { &ld_double_max,         1,  "DBL_MAX",       1 },
  { &ld_double_max,        -1, "-DBL_MAX",       1 },
  { &__dj_long_double_max,  1,  "LDBL_MAX",      1 },
  { &__dj_long_double_max, -1, "-LDBL_MAX",      1 }
};

static const size_t n_tests3 = sizeof(tests3) / sizeof(tests3[0]);

static void inline
result (const size_t n, const float f_in, const float f_out)
{
  printf("Test %lu: %g -> %g\n", n, f_in, f_out);
}

int
main (void)
{
  char buf[128];
  float f_res;
  int eq;
  size_t i;

  ld_float_max    = __dj_float_max;
  ld_float_max_10 = ld_float_max * 10;
  ld_double_max   = __dj_double_max;

  puts("float tests:");
  for (i = 0; i < n_tests; i++) {
    sprintf(buf, "%g", tests[i]);
    f_res = strtof(buf, NULL);
    result(i + 1, tests[i], f_res);
  }

  puts("float_t tests:");
  for (i = 0; i < n_tests2; i++) {
    sprintf(buf, "%g", *(const float *) &tests2[i]);
    f_res = strtof(buf, NULL);
    result(i + 1, *(const float *) &tests2[i], f_res);
  }

  puts("HUGE_VALF tests:");
  for (i = 0; i < n_tests3; i++) {
    sprintf(buf, "%Lg", *(tests3[i].ld) * tests3[i].sign);
    f_res = strtof(buf, NULL);

    printf("strtof(sprintf(..., %s)) == %sHUGE_VALF: ",
	   tests3[i].str,
	   (tests3[i].sign < 0) ? "-" : "");

    eq = (f_res == (HUGE_VALF * ((tests3[i].sign < 0) ? -1 : 1)));

    if (tests3[i].overflow) {
      if (eq)
	puts("Yes - OK");
      else
	puts("No - FAIL");
    } else {
      if (eq)
	puts("Yes - FAIL");
      else
	puts("No - OK");
    }
  }

  return(EXIT_SUCCESS);
}
