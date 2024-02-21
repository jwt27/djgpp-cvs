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

typedef struct {
  const char * const str; /* String to run strtof() on. */
  const int diff; /* For endptr tests. How many characters from string start
		     endptr should be offset. */
} test4_t;

static const test4_t tests4[] = {
  { "inF",                 3 },
  { "-INf",                4 },
  { "infi",                3 },
  { "-infi",               4 },
  { "infinit",             3 },
  { "-infinit",            4 },
  { "INfINITY",            8 },
  { "-InfInIty",           9 },
  { "infinity0",           8 },
  { "-infinity5",          9 },
  { "infinity-1",          8 },
  { "-infinity-6",         9 },
};

static const size_t n_tests4 = sizeof(tests4) / sizeof(tests4[0]);

typedef struct {
  const char * const str; /* String to run strtof() on. */
  const int diff; /* For endptr tests. How many characters from string start
		     endptr should be offset. */
  const int sign; /* Sign bit. */
  const int mantissa; /* What mantissa should be set to after
			 conversion. 0 -> don't care as long as it's
			 non-zero. */
} test5_t;

static const test5_t tests5[] = {
  { "nAn",                         3,  0, 0 },
  { "-nAn",                        4,  1, 0 },
  { "nanny",                       3,  0, 0 },
  { "-nanny",                      4,  1, 0 },
  { "NAN()",                       5,  0, 0 },
  { "NAN( )",                      3,  0, 0 },
  { "-Nan()",                      6,  1, 0 },
  { "nAN(0)",                      6,  0, 0 },
  { "-nan(0)",                     7,  1, 0 },
  { "-nan(0x0)",                   9,  1, 0 },
  { "nan(4198964)",               12,  0, 0x401234 }, /* QNaN */
  { "nan(0x401234)",              13,  0, 0x401234 }, /* QNaN */
  { "-nan(0x400088)",             14,  1, 0x400088 }, /* QNaN */
  { "nan(1234)",                   9,  0, 1234 }, /* SNaN -> QNaN? */
  { "nan(0x1234)",                11,  0, 0x1234 }, /* SNaN -> QNaN? */
  { "-nan(88)",                    8,  1, 88 }, /* SNaN -> QNaN? */
  { "-nan(0x88)",                 10,  1, 0x88 }, /* SNaN -> QNaN? */
  { "-nan(12345678901234567890)", 26,  1, 0x7fffff }, /* Overflow. */
  { "-nan(0xaa7d7aa74)",          17,  1, 0x7fffff }, /* Overflow. */
  { "nan(0x12345678123456781)",   24,  0, 0x7fffff }, /* Overflow. */
  { "-nan(0x12345678123456781)",  25,  1, 0x7fffff }, /* Overflow. */
  { "naN(something)",              3,  0, 0 },
  { "-nAn(smurf)",                 4,  1, 0 },
  { "-nan(-nan)",                  4,  1, 0 },
  { "nan(nan(nan()))",             3,  0, 0 },
  { "NaN(0x1234oops)",             3,  0, 0 },
  { "nan()()",                     5,  0, 0 },
  { "NAN()nan",                    5,  0, 0 },
};

static const size_t n_tests5 = sizeof(tests5) / sizeof(tests5[0]);


static void inline
result (const size_t n, const float f_in, const float f_out)
{
  printf("Test %lu: %g -> %g\n", n, f_in, f_out);
}

int
main (int argc, char *argv[])
{
  char buf[128];
  float f_res;
  int eq;
  size_t i;
  int ok = 1;

  if ( 1 < argc ) {
    /* Fault on FPU exception. */
    _control87(0, 0x3f);
  }

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
    _float_union_t float_union;  /* Fix dereferencing type-punned pointer will break strict-aliasing rules error.  */
    float_union.ft = tests2[i];
    sprintf(buf, "%g", float_union.f);
    f_res = strtof(buf, NULL);
    result(i + 1, float_union.f, f_res);
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
      if (eq) {
	  puts("Yes - OK");
      } else {
	puts("No - FAIL");
	ok = 0;
      }
    } else {
      if (eq) {
	puts("Yes - FAIL");
	ok = 0;
      } else {
	puts("No - OK");
      }
    }
  }

  puts("Infinity tests:");
  for (i = 0; i < n_tests4; i++) {
    char *endptr;
    float_t float_bits;
    _float_union_t float_union;

    f_res = strtof(tests4[i].str, &endptr);

    printf("strtof(\"%s\", &endptr) -> %f, %td - ", tests4[i].str,
	   f_res, endptr - tests4[i].str);

    /* Need to do the Inf detection ourselves. */
    float_union.f = f_res;  /* Fix dereferencing type-punned pointer will break strict-aliasing rules error.  */
    float_bits = float_union.ft;
    if (float_bits.exponent != 0xff) {
      puts("exponent != 0xff - FAIL");
      ok = 0;
    } else if (float_bits.mantissa != 0) {
      puts("mantissa != 0 - FAIL");
      ok = 0;
    } else if ( (float_bits.sign && 0 < f_res ) ||
		(!float_bits.sign && f_res < 0) ) {
      puts("Wrong sign - FAIL");
      ok = 0;
    } else if ( endptr - tests4[i].str != tests4[i].diff) {
      printf("endptr-(start_of_string) == %td != %td - FAIL\n",
	     endptr - tests4[i].str, tests4[i].diff);
      ok = 0;
    } else {
      puts("OK");
    }
  }

  puts("Nan tests:");
  for (i = 0; i < n_tests5; i++) {
    char *endptr;
    float_t float_bits;
    _float_union_t float_union;

    f_res = strtof(tests5[i].str, &endptr);

    printf("strtof(\"%s\", &endptr) -> %f, %td - ", tests5[i].str,
	   f_res, endptr-tests5[i].str);

    /* Need to to the NaN detection ourselves. */
    float_union.f = f_res;
    float_bits = float_union.ft;
    if (float_bits.exponent != 0xff ) {
      puts("exponent != 0xff - FAIL");
      ok = 0;
    } else if (float_bits.mantissa == 0) {
      puts("mantissa == 0 - FAIL");
      ok = 0;
    } else if (tests5[i].sign != float_bits.sign) {
      printf("sign == %d != %d - FAIL\n", float_bits.sign,
	     tests5[i].sign);
      ok = 0;
    } else if ( endptr - tests5[i].str != tests5[i].diff) {
      printf("endptr-(start_of_string) == %td != %td - FAIL\n",
	     endptr - tests5[i].str, tests5[i].diff);
      ok = 0;
    } else if (tests5[i].mantissa &&
	       tests5[i].mantissa != float_bits.mantissa) {
      printf("(note: mantissa == 0x%x != 0x%x) - OK\n",
	     float_bits.mantissa, tests5[i].mantissa);
    } else {
      puts("OK");
    }
  }

  puts(ok ? "PASS" : "FAIL");
  return(ok ? EXIT_SUCCESS : EXIT_FAILURE);
}
