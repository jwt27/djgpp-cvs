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
  size_t i;

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

  return(EXIT_SUCCESS);
}
