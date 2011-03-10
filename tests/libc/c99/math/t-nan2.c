/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */

/*
 * t-nan2.c
 * Tests for nan(), nanf(), nanl().
 * Written by Richard Dawe <rich@phekda.freeserve.co.uk>
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <libc/ieee.h>

#include "fp-union.h"


typedef struct {
  int valid;

  const char *input;

  int           ignore_mantissa;
  double_t      dt_expected;
  float_t       ft_expected;
  long_double_t ldt_expected;
} testcase_t;

testcase_t testcases[] = {
  /* Testcase 1: nan*(NULL) == strto*("NAN") */
  /*
   * TODO: This test is a little bogus, due to ignoring
   * the rounding errors.
   */
  {
    1,
    NULL,
    1, /* NB: Rounding errors mean we don't get the mantissas we want. */
    { 0xffffffffU, 0xfffffU, 0x7ffU, 0 },
    { 0x7fffffU, 0xffU, 0 },
    { 0xffffffffU, 0xffffffffU, 0x7fffU, 0 }
  },

  /* Testcase 2: nan*("<not-a-number>") == strto*("NAN") */
  /*
   * TODO: This test is a little bogus, due to ignoring
   * the rounding errors.
   */
  {
    1,
    "thisisn'tanumber",
    1, /* NB: Rounding errors mean we don't get the mantissas we want. */
    { 0xffffffffU, 0xfffffU, 0x7ffU, 0 },
    { 0x7fffffU, 0xffU, 0 },
    { 0xffffffffU, 0xffffffffU, 0x7fffU, 0 }
  },

  /* Testcase 3: nan*("") == strto*("NAN()") */
  /*
   * TODO: This test is a little bogus, due to ignoring
   * the rounding errors.
   */
  {
    1,
    "",
    1, /* NB: Rounding errors mean we don't get the mantissas we want. */
    { 0xffffffffU, 0xfffffU, 0x7ffU, 0 },
    { 0x7fffffU, 0xffU, 0 },
    { 0xffffffffU, 0xffffffffU, 0x7fffU, 0 }
  },

  /* Testcase 4 */
  /*
   * TODO: This test is a little bogus, due to ignoring
   * the rounding errors.
   */
  {
    1,
    "0",
    1, /* NB: Rounding errors mean we don't get the mantissas we want. */
    { 0, 0x80000U, 0x7ffU, 0 },
    { 0x400000, 0xffU, 0 },
    { 0, 0xc0000000U, 0x7fffU, 0 },
  },

  /*
   * Testcase 5:
   * nan*("<decimal-number>") == strto*("NAN(<decimal-number>)")
   */
  {
    1,
    "1",
    0,
    { 0x1U, 0x80000U, 0x7ffU, 0 },
    { 0x400001, 0xffU, 0 },
    { 1, 0xc0000000, 0x7fffU, 0 }
  },

  /*
   * Testcase 6:
   * nan*("<decimal-number>") == strto*("NAN(<decimal-number>)")
   */
  {
    1,
    "1234",
    0,
    { 0x4d2, 0x80000U, 0x7ffU, 0 },
    { 0x4004d2, 0xffU, 0 },
    { 0x4d2, 0xc0000000, 0x7fffU, 0 }
  },

  /*
   * Testcase 7:
   * nan*("<hexidecimal-number>") == strto*("NAN(<hexidecimal-number>)")
   */
  {
    1,
    "0x1",
    0,
    { 0x1U, 0x80000U, 0x7ffU, 0 },
    { 0x400001, 0xffU, 0 },
    { 1, 0xc0000000, 0x7fffU, 0 }
  },

  /*
   * Testcase 8:
   * nan*("<decimal-number>") == strto*("NAN(<decimal-number>)")
   */
  {
    1,
    "0x1234",
    0,
    { 0x1234, 0x80000U, 0x7ffU, 0 },
    { 0x401234, 0xffU, 0 },
    { 0x1234, 0xc0000000, 0x7fffU, 0 }
  },

  /*
   * Testcase 9:
   *    nan*("<overflowing-decimal-number>")
   * == strto*("NAN(<overflowing-decimal-number>")
   */
  /*
   * TODO: This test is a little bogus, due to ignoring
   * the rounding errors.
   */
  {
    1,
    "12345678901234567890",
    1, /* NB: Rounding errors mean we don't get the mantissas we want. */
    { 0xffffffffU, 0xfffffU, 0x7ffU, 0 },
    { 0x7fffffU, 0xffU, 0 },
    { 0xffffffffU, 0xffffffffU, 0x7fffU, 0 }
  },

  /*
   * Testcase 10:
   *    nan*("<overflowing-hexidecimal-number>")
   * == strto*("NAN(<overflowing-hexidecimal-number>")
   */
  /*
   * TODO: This test is a little bogus, due to ignoring
   * the rounding errors.
   */
  {
    1,
    "0x12345678901234567890",
    1, /* NB: Rounding errors mean we don't get the mantissas we want. */
    { 0xffffffffU, 0xfffffU, 0x7ffU, 0 },
    { 0x7fffffU, 0xffU, 0 },
    { 0xffffffffU, 0xffffffffU, 0x7fffU, 0 }
  },

  /* Terminator */
  {
    0,
    NULL,
    0,
    { 0, 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 0, 0, 0 },
  }
};

static int
double_t_equal (double_t *dt1, double_t *dt2, const int ignore_mantissa)
{
   int res = 1;

   if (!ignore_mantissa)
     {
       if (dt1->mantissal != dt2->mantissal)
	 res = 0;
       if (dt1->mantissah != dt2->mantissah)
	 res = 0;
     }
   if (dt1->exponent != dt2->exponent)
     res = 0;
   if (dt1->sign != dt2->sign)
     res = 0;

   return res;
}

static void
dump_double_t (double_t *dt)
{
  printf("(0x%x, 0x%x, 0x%x, 0x%x)",
	 dt->mantissal, dt->mantissah, dt->exponent, dt->sign);
}

static int
float_t_equal (float_t *ft1, float_t *ft2, const int ignore_mantissa)
{
   int res = 1;

   if (!ignore_mantissa && (ft1->mantissa != ft2->mantissa))
     res = 0;
   if (ft1->exponent != ft2->exponent)
     res = 0;
   if (ft1->sign != ft2->sign)
     res = 0;

   return res;
}

static void
dump_float_t (float_t *ft)
{
  printf("(0x%x, 0x%x, 0x%x)",
	 ft->mantissa, ft->exponent, ft->sign);
}

static int
long_double_t_equal (long_double_t *ldt1,
		     long_double_t *ldt2,
		     const int ignore_mantissa)
{
   int res = 1;

   if (!ignore_mantissa)
     {
       if (ldt1->mantissal != ldt2->mantissal)
	 res = 0;
       if (ldt1->mantissah != ldt2->mantissah)
	 res = 0;
     }
   if (ldt1->exponent != ldt2->exponent)
     res = 0;
   if (ldt1->sign != ldt2->sign)
     res = 0;

   return res;
}

static void
dump_long_double_t (long_double_t *ldt)
{
  printf("(0x%x, 0x%x, 0x%x, 0x%x)",
	 ldt->mantissal, ldt->mantissah, ldt->exponent, ldt->sign);
}

int
main (void)
{
  int           ok = 1;
  double_t      dt_res;
  float_t       ft_res;
  long_double_t ldt_res;
  int           i;
  int           testcase = 0;

  testcase++;

  for (i = 0; testcases[i].valid; testcase++, i++)
    {
      test_float_union_t fu;
      test_double_union_t du;
      test_long_double_union_t ldu;
      printf("Testcase %d: nan(): ", testcase);

      du.d = nan(testcases[i].input);
      dt_res = du.dt;

      if (!double_t_equal(&testcases[i].dt_expected,
			  &dt_res,
			  testcases[i].ignore_mantissa))
	{
	  puts("FAIL");
	  printf("Expected: ");
	  dump_double_t(&testcases[i].dt_expected);
	  printf("\nGot: ");
	  dump_double_t(&dt_res);
	  puts("");
	  ok = 0;
	}
      else
	{
	  puts("OK");
	}

      printf("Testcase %d: nanf(): ", testcase);

      fu.f = nanf(testcases[i].input);
      ft_res = fu.ft;

      if (!float_t_equal(&testcases[i].ft_expected,
			 &ft_res,
			 testcases[i].ignore_mantissa))
	{
	  puts("FAIL");
	  printf("Expected: ");
	  dump_float_t(&testcases[i].ft_expected);
	  printf("\nGot: ");
	  dump_float_t(&ft_res);
	  puts("");
	  ok = 0;
	}
      else
	{
	  puts("OK");
	}

      printf("Testcase %d: nanl(): ", testcase);

      ldu.ld = nanl(testcases[i].input);
      ldt_res = ldu.ldt;

      if (!long_double_t_equal(&testcases[i].ldt_expected,
			       &ldt_res,
			       testcases[i].ignore_mantissa))
	{
	  puts("FAIL");
	  printf("Expected: ");
	  dump_long_double_t(&testcases[i].ldt_expected);
	  printf("\nGot: ");
	  dump_long_double_t(&ldt_res);
	  puts("");
	  ok = 0;
	}
      else
	{
	  puts("OK");
	}
    }

  puts(ok ? "PASS" : "FAIL");
  return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
