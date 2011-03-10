/*
 * File t-fpclas.c (8.3 of t-fpclassify.c).
 *
 * Copyright (C) 2003 Martin Str@"omberg <ams@ludd.luth.se>.
 *
 * This software may be used freely so long as this copyright notice is
 * left intact. There is no warranty on this software.
 *
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <libc/ieee.h>

#include "fp-union.h"


typedef struct { 
  float_t number; 	/* Number to test. */
  int class; 		/* Which class. */
} test_float_t;

static const test_float_t tests_float[] = 
{ 
  /* Zeros. */
  { { 0, 0, 0 }, FP_ZERO }, /* 0.0 */
  { { 0, 0, 1 }, FP_ZERO }, /* -0.0 */

  /* Subnormals aka denormals. */
  { { 1, 0, 0 }, FP_SUBNORMAL }, /* Very small number. */
  { { 1, 0, 1 }, FP_SUBNORMAL }, /* Very small -number. */

  /* Normals. */
  { { 1, 1, 0 }, FP_NORMAL }, /* Small number. */
  { { 1, 1, 1 }, FP_NORMAL }, /* Small -number. */
  { { 0xff, 0xfe, 0 }, FP_NORMAL }, /* Big number. */
  { { 0xff, 0xfe, 1 }, FP_NORMAL }, /* Big -number. */

  /* Infs. */
  { { 0, 0xff, 0 }, FP_INFINITE }, /* Inf */
  { { 0, 0xff, 1 }, FP_INFINITE }, /* -Inf */

  /* NaNs. */
  { { 1, 0xff, 0 }, FP_NAN }, /* SNaN */
  { { 1, 0xff, 1 }, FP_NAN }, /* -SNaN */
  { { 0x7fffff, 0xff, 0 }, FP_NAN }, /* QNaN */
  { { 0x7fffff, 0xff, 1 }, FP_NAN }, /* -QNaN */
};

static const size_t n_tests_float = sizeof(tests_float) / sizeof(tests_float[0]);

typedef struct { 
  double_t number; 	/* Number to test. */
  int class; 		/* Which class. */
} test_double_t;

static const test_double_t tests_double[] =
{ 
  /* Zeros. */
  { { 0, 0, 0, 0 }, FP_ZERO }, /* 0.0 */
  { { 0, 0, 0, 1 }, FP_ZERO }, /* -0.0 */

  /* Subnormals aka denormals. */
  { { 1, 0, 0, 0 }, FP_SUBNORMAL }, /* Very small number. */
  { { 1, 0, 0, 1 }, FP_SUBNORMAL }, /* Very small -number. */

  /* Normals. */
  { { 1, 0, 1, 0 }, FP_NORMAL }, /* Small number. */
  { { 1, 0, 1, 1 }, FP_NORMAL }, /* Small -number. */
  { { 0xffffffff, 0x7ffff, 0x7fe, 0 }, FP_NORMAL }, /* Big number. */
  { { 0xffffffff, 0x7ffff, 0x7fe, 1 }, FP_NORMAL }, /* Big -number. */

  /* Infs. */
  { { 0, 0, 0x7ff, 0 }, FP_INFINITE }, /* Inf */
  { { 0, 0, 0x7ff, 1 }, FP_INFINITE }, /* -Inf */

  /* NaNs. */
  { { 1, 0, 0x7ff, 0 }, FP_NAN }, /* SNaN */
  { { 1, 0, 0x7ff, 1 }, FP_NAN }, /* -SNaN */
  { { 0, 0xfffff, 0x7ff, 0 }, FP_NAN }, /* QNaN */
  { { 0, 0xfffff, 0x7ff, 1 }, FP_NAN }, /* -QNaN */
};

static const size_t n_tests_double = sizeof(tests_double) / sizeof(tests_double[0]);

typedef struct { 
  long_double_t number; /* Number to test. */
  int class; 		/* Which class. */
} test_long_double_t;

static const test_long_double_t tests_long_double[] =
{ 
  /* Zeros. */
  { { 0, 0, 0, 0 }, FP_ZERO }, /* 0.0. */
  { { 0, 0, 0, 1 }, FP_ZERO }, /* -0.0. */

  /* Subnormals aka denormals. */
  { { 1, 0, 0, 0 }, FP_SUBNORMAL }, /* Subnormal, smallest possible > 0. */
  { { 1, 0, 0, 1 }, FP_SUBNORMAL }, /* -Subnormal, biggest possible < 0. */
  { { 0, 1, 0, 0 }, FP_SUBNORMAL }, /* Subnormal. */
  { { 0, 1, 0, 1 }, FP_SUBNORMAL }, /* -Subnormal. */

  /* Unnormals. */
  { { 0, 0x80000000, 0, 0 }, FP_UNNORMAL }, /* Unnormal, 0 with integer bit set. */
  { { 0, 0x80000000, 0, 1 }, FP_UNNORMAL }, /* Unnormal, -0 with integer bit set. */
  { { 1, 0x80000000, 0, 0 }, FP_UNNORMAL }, /* Unnormal, smallest possible > 0 with integer bit set. */
  { { 1, 0x80000000, 0, 1 }, FP_UNNORMAL }, /* Unnormal, biggest possible < 0 with integer bit set. */
  { { 1, 0, 1, 0 }, FP_UNNORMAL }, /* Small number with missing 1. */
  { { 1, 0, 1, 1 }, FP_UNNORMAL }, /* Small -number with missing 1. */
  { { 0xffffffff, 0x7fffffff, 0x7ffe, 0 }, FP_UNNORMAL }, /* Big number with missing 1. */
  { { 0xffffffff, 0x7fffffff, 0x7ffe, 1 }, FP_UNNORMAL }, /* Big -number with missing 1. */
  { { 0, 0, 0x7fff, 0 }, FP_UNNORMAL }, /* Inf with missing 1. */
  { { 0, 0, 0x7fff, 1 }, FP_UNNORMAL }, /* -Inf with missing 1. */
  { { 1, 0, 0x7fff, 0 }, FP_UNNORMAL }, /* SNaN with missing 1 */
  { { 1, 0, 0x7fff, 1 }, FP_UNNORMAL }, /* -SNaN with missing 1 */
  { { 0, 0x7fffffff, 0x7fff, 0 }, FP_UNNORMAL }, /* QNaN with missing 1 */
  { { 0, 0x7fffffff, 0x7fff, 1 }, FP_UNNORMAL }, /* -QNaN with missing 1 */

  /* Normals. */
  { { 1, 0x80000000, 1, 0 }, FP_NORMAL }, /* Normal, smallest possible > 0. */
  { { 1, 0x80000000, 1, 1 }, FP_NORMAL }, /* Normal, biggest possible < 0. */
  { { 0xffffffff, 0xffffffff, 0x7ffe, 0 }, FP_NORMAL }, /* Big number. */
  { { 0xffffffff, 0xffffffff, 0x7ffe, 1 }, FP_NORMAL }, /* Big -number. */

  /* Infs. */
  { { 0, 0x80000000, 0x7fff, 0 }, FP_INFINITE }, /* Inf. */
  { { 0, 0x80000000, 0x7fff, 1 }, FP_INFINITE }, /* -Inf. */

  /* NaNs. */
  { { 1, 0x80000000, 0x7fff, 0 }, FP_NAN }, /* SNaN */
  { { 1, 0x80000000, 0x7fff, 1 }, FP_NAN }, /* -SNaN */
  { { 0, 0xffffffff, 0x7fff, 0 }, FP_NAN }, /* QNaN */
  { { 0, 0xffffffff, 0x7fff, 1 }, FP_NAN }, /* -QNaN */
};

static const size_t n_tests_long_double = sizeof(tests_long_double) / sizeof(tests_long_double[0]);

int
main(void)
{
  int result;
  size_t i;

  printf("FP_ZERO=0x%x, FP_SUBNORMAL=0x%x, FP_NORMAL=0x%x, FP_INFINITE=0x%d,\n"
	 "FP_NAN=0x%x, FP_UNNORMAL=0x%x\n\n", 
	 FP_ZERO, FP_SUBNORMAL, FP_NORMAL, FP_INFINITE, 
	 FP_NAN, FP_UNNORMAL);

  puts("Float tests:");
  for(i = 0; i < n_tests_float; i++)
  {
    test_float_union_t fu;
    fu.ft = tests_float[i].number;
    result = fpclassify(fu.f);
    printf("\t%G -> 0x%x: ", fu.f, result);
    if( result == tests_float[i].class )
    {
      puts("Ok.");
    }
    else
    {
      puts("Fail.");
    }
  }

  puts("Double tests:");
  for(i = 0; i < n_tests_double; i++)
  {
    test_double_union_t du;
    du.dt = tests_double[i].number;
    result = fpclassify(du.d);
    printf("\t%G -> 0x%x: ", du.d, result);
    if( result == tests_double[i].class )
    {
      puts("Ok.");
    }
    else
    {
      puts("Fail.");
    }
  }

  puts("Long double tests:");
  for(i = 0; i < n_tests_long_double; i++)
  {
    test_long_double_union_t ldu;
    ldu.ldt = tests_long_double[i].number;
    result = fpclassify(ldu.ld);
    printf("\t%LG -> 0x%x: ", ldu.ld, result);
    if( result == tests_long_double[i].class )
    {
      puts("Ok.");
    }
    else
    {
      puts("Fail.");
    }
  }

  return(EXIT_SUCCESS);
}
