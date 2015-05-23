#include "../../../../include/stdio.h"
#include "../../../../include/math.h"
#include "../../../../include/libc/ieee.h"

#define TEST_TYPE  0  /* Allowed 0 or 1 */

typedef enum {
  false = 0, true = 1
} bool;

typedef union
{
  long_double_t ldt;
  long double ld;
} test_long_double_union_t;

typedef struct {
  const char *class;
  const char *type;
  test_long_double_union_t number;
} test_long_double_t;


typedef union
{
  double_t dt;
  double d;
} test_double_union_t;

typedef struct { 
  const char *class;
  const char *type;
  test_double_union_t number;
} test_double_t;


typedef union
{
  float_t ft;
  float f;
} test_float_union_t;

typedef struct { 
  const char *class;
  const char *type;
  test_float_union_t number;
} test_float_t;


int main(void)
{
  bool test_passed;
  int i;

  test_long_double_t test_ld[] = {
    /*  Normals.  (4)  */
    {"long double", "small positive normal", {{1, 0x80000000, 1, 0}}},  /*  Smallest possible > 0.  */
    {"long double", "small negative normal", {{1, 0x80000000, 1, 1}}},  /*  Biggest possible < 0.  */
    {"long double", "big positive normal", {{0xFFFFFFFF, 0xFFFFFFFF, 0x7FFE, 0}}},  /* Big  number.  */
    {"long double", "big negative normal", {{0xFFFFFFFF, 0xFFFFFFFF, 0x7FFE, 1}}},  /* Big -number.  */

    /*  Subnormals aka denormals.  (4 + 2)  */
    {"long double", "positive subnormal", {{1, 0, 0, 0}}},  /*  Smallest possible > 0. */
    {"long double", "negative subnormal", {{1, 0, 0, 1}}},  /*  Biggest possible < 0. */

    /* Zeros. (4 + 2 + 2)  */
    {"long double", "positive zero", {{0, 0, 0, 0 }}},  /*  0.0  */
    {"long double", "negative zero", {{0, 0, 0, 1 }}},  /*  -0.0  */

    /* Infinities. (4 + 2 + 2 + 2)  */
    {"long double", "positive infinity", {{0, 0x80000000, 0x7FFF, 0}}},  /*  Inf.  */
    {"long double", "negative infinity", {{0, 0x80000000, 0x7FFF, 1}}},  /* -Inf.  */

    /* NaNs. (4 + 2 + 2 + 2 + 4)  */
    {"long double", "positive signalling NaN", {{1, 0x80000000, 0x7FFF, 0}}},  /*  SNaN  */
    {"long double", "negative signalling NaN", {{1, 0x80000000, 0x7FFF, 1}}},  /* -SNaN  */
    {"long double", "positive quiet NaN", {{0, 0xFFFFFFFF, 0x7FFF, 0}}},  /*  QNaN  */
    {"long double", "negative quiet NaN", {{0, 0xFFFFFFFF, 0x7FFF, 1}}}   /* -QNaN  */
  };

  test_double_t test_d[] = {
    /*  Normals.  */
    {"double", "small positive normal", {{1, 0, 1, 0}}},  /*  Small  number.  */
    {"double", "small negative normal", {{1, 0, 1, 1}}},  /*  Small -number.  */
    {"double", "big positive normal", {{0xFFFFFFFF, 0x7FFFF, 0x7FE, 0}}},  /*  Big  number.  */
    {"double", "big negative normal", {{0xFFFFFFFF, 0x7FFFF, 0x7FE, 1}}},  /*  Big -number.  */

    /*  Subnormals aka denormals.  */
    {"double", "positive subnormal", {{1, 0, 0, 0}}},  /*  Very small  number.  */
    {"double", "negative subnormal", {{1, 0, 0, 1}}},  /*  Very small -number.  */

    /* Zeros. */
    {"double", "positive zero", {{0, 0, 0, 0}}},  /*   0.0  */
    {"double", "negative zero", {{0, 0, 0, 1}}},  /*  -0.0  */

    /* Infinities. */
    {"double", "positive infinity", {{0, 0, 0x7FF, 0}}},  /*   Inf.  */
    {"double", "negative infinity", {{0, 0, 0x7FF, 1}}},  /*  -Inf.  */

    /* NaNs. */
    {"double", "positive signalling NaN", {{1, 0, 0x7FF, 0}}},  /*   SNaN  */
    {"double", "negative signalling NaN", {{1, 0, 0x7FF, 1}}},  /*  -SNaN  */
    {"double", "positive quiet NaN", {{0, 0xFFFFF, 0x7FF, 0}}},  /*   QNaN  */
    {"double", "negative quiet NaN", {{0, 0xFFFFF, 0x7FF, 1}}}   /*  -QNaN  */
  };

  test_float_t test_f[] = {
    /*  Normals.  */
    {"float", "small positive normal", {{1, 1, 0}}},  /*  Small  number.  */
    {"float", "small negative normal", {{1, 1, 1}}},  /*  Small -number.  */
    {"float", "big positive normal", {{0xFF, 0xFE, 0}}},  /*  Big  number.  */
    {"float", "big negative normal", {{0xFF, 0xFE, 1}}},  /*  Big -number.  */

    /*  Subnormals aka denormals.  */
    {"float", "positive subnormal", {{1, 0, 0}}},  /*  Very small  number. */
    {"float", "negative subnormal", {{1, 0, 1}}},  /*  Very small -number. */

    /* Zeros. */
    {"float", "positive zero", {{0, 0, 0}}},  /*   0.0  */
    {"float", "negative zero", {{0, 0, 1}}},  /*  -0.0  */

    /* Infinities. */
    {"float", "positive infinity", {{0, 0xFF, 0}}},  /*   Inf  */
    {"float", "negative infinity", {{0, 0xFF, 1}}},  /*  -Inf  */

    /* NaNs. */
    {"float", "positive signalling NaN", {{1, 0xFF, 0}}},  /*   SNaN  */
    {"float", "negative signalling NaN", {{1, 0xFF, 1}}},  /*  -SNaN  */
    {"float", "positive quiet NaN", {{0x7FFFFF, 0xFF, 0}}},  /*   QNaN  */
    {"float", "negative quiet NaN", {{0x7FFFFF, 0xFF, 1}}}   /*  -QNaN  */
  };



  /*
   *  isfinite must return non-zero for normals, subnormals and zero,
   *  and zero for infinity and NaN.
   */
  test_passed = true;
  for (i = 0; i < 4; i++)
  {
    if (!isfinite(test_ld[i].number.ld))
    {
      printf("Test failed.  Testing isfinite(%La) for a %s %s: %d\n",  test_ld[i].number.ld, test_ld[i].class, test_ld[i].type, isfinite(test_ld[i].number.ld));
      break;
    }
    else if (!isfinite(test_d[i].number.d))
    {
      printf("Test failed.  Testing isfinite(%a) for a %s %s: %d\n",   test_d[i].number.d,   test_d[i].class,  test_d[i].type,  isfinite(test_d[i].number.d));
      break;
    }
    else if (!isfinite(test_f[i].number.f))
    {
      printf("Test failed.  Testing isfinite(%a) for a %s %s: %d\n\n", test_f[i].number.f,   test_f[i].class,  test_f[i].type,  isfinite(test_f[i].number.f));
      break;
    }
  }
  if (i < 4)
  {
    test_passed = false;
    printf("isfinite(x) failed for normals.\n");
  }

  for (i = 4; i < 4 + 2; i++)
  {
    if (!isfinite(test_ld[i].number.ld))
    {
      printf("Test failed.  Testing isfinite(%La) for a %s %s: %d\n",  test_ld[i].number.ld, test_ld[i].class, test_ld[i].type, isfinite(test_ld[i].number.ld));
      break;
    }
    else if (!isfinite(test_d[i].number.d))
    {
      printf("Test failed.  Testing isfinite(%a) for a %s %s: %d\n",   test_d[i].number.d,   test_d[i].class,  test_d[i].type,  isfinite(test_d[i].number.d));
      break;
    }
    else if (!isfinite(test_f[i].number.f))
    {
      printf("Test failed.  Testing isfinite(%a) for a %s %s: %d\n\n", test_f[i].number.f,   test_f[i].class,  test_f[i].type,  isfinite(test_f[i].number.f));
      break;
    }
  }
  if (i < 4 + 2)
  {
    test_passed = false;
    printf("isfinite(x) failed for subnormals aka denormals.\n");
  }

  for (i = 4 + 2; i < 4 + 2 + 2; i++)
  {
    if (!isfinite(test_ld[i].number.ld))
    {
      printf("Test failed.  Testing isfinite(%La) for a %s %s: %d\n",  test_ld[i].number.ld, test_ld[i].class, test_ld[i].type, isfinite(test_ld[i].number.ld));
      break;
    }
    else if (!isfinite(test_d[i].number.d))
    {
      printf("Test failed.  Testing isfinite(%a) for a %s %s: %d\n",   test_d[i].number.d,   test_d[i].class,  test_d[i].type,  isfinite(test_d[i].number.d));
      break;
    }
    else if (!isfinite(test_f[i].number.f))
    {
      printf("Test failed.  Testing isfinite(%a) for a %s %s: %d\n\n", test_f[i].number.f,   test_f[i].class,  test_f[i].type,  isfinite(test_f[i].number.f));
      break;
    }
  }
  if (i < 4 + 2 + 2)
  {
    test_passed = false;
    printf("isfinite(x) failed for zero.\n");
  }

  for (i = 4 + 2 + 2; i < 4 + 2 + 2 + 2; i++)
  {
    if (isfinite(test_ld[i].number.ld))
    {
      printf("Test failed.  Testing isfinite(%La) for a %s %s: %d\n",  test_ld[i].number.ld, test_ld[i].class, test_ld[i].type, isfinite(test_ld[i].number.ld));
      break;
    }
    else if (isfinite(test_d[i].number.d))
    {
      printf("Test failed.  Testing isfinite(%a) for a %s %s: %d\n",   test_d[i].number.d,   test_d[i].class,  test_d[i].type,  isfinite(test_d[i].number.d));
      break;
    }
    else if (isfinite(test_f[i].number.f))
    {
      printf("Test failed.  Testing isfinite(%a) for a %s %s: %d\n\n", test_f[i].number.f,   test_f[i].class,  test_f[i].type,  isfinite(test_f[i].number.f));
      break;
    }
  }
  if (i < 4 + 2 + 2 + 2)
  {
    test_passed = false;
    printf("isfinite(x) failed for infinity.\n");
  }

  for (i = 4 + 2 + 2 + 2; i < 4 + 2 + 2 + 2 + 4; i++)
  {
    if (isfinite(test_ld[i].number.ld))
    {
      printf("Test failed.  Testing isfinite(%La) for a %s %s: %d\n",  test_ld[i].number.ld, test_ld[i].class, test_ld[i].type, isfinite(test_ld[i].number.ld));
      break;
    }
    else if (isfinite(test_d[i].number.d))
    {
      printf("Test failed.  Testing isfinite(%a) for a %s %s: %d\n",   test_d[i].number.d,   test_d[i].class,  test_d[i].type,  isfinite(test_d[i].number.d));
      break;
    }
    else if (isfinite(test_f[i].number.f))
    {
      printf("Test failed.  Testing isfinite(%a) for a %s %s: %d\n\n", test_f[i].number.f,   test_f[i].class,  test_f[i].type,  isfinite(test_f[i].number.f));
      break;
    }
  }
  if (i < 4 + 2 + 2 + 2 + 4)
  {
    test_passed = false;
    printf("isfinite(x) failed for SNaN and/or QNaN.\n");
  }
  else if (test_passed)
    printf("isfinite(x) passed all tests.\n");
  printf("----------------------------------------------------------------------\n");



  /*
   *  isnormal must return non-zero for normals
   *  and zero for subnormals, zero, infinity and NaN.
   */
  test_passed = true;
  for (i = 0; i < 4; i++)
  {
    if (!isnormal(test_ld[i].number.ld))
    {
      printf("Test failed.  Testing isnormal(%La) for a %s %s: %d\n",  test_ld[i].number.ld, test_ld[i].class, test_ld[i].type, isnormal(test_ld[i].number.ld));
      break;
    }
    else if (!isnormal(test_d[i].number.d))
    {
      printf("Test failed.  Testing isnormal(%a) for a %s %s: %d\n",   test_d[i].number.d,   test_d[i].class,  test_d[i].type,  isnormal(test_d[i].number.d));
      break;
    }
    else if (!isnormal(test_f[i].number.f))
    {
      printf("Test failed.  Testing isnormal(%a) for a %s %s: %d\n\n", test_f[i].number.f,   test_f[i].class,  test_f[i].type,  isnormal(test_f[i].number.f));
      break;
    }
  }
  if (i < 4)
  {
    test_passed = false;
    printf("isnormal(x) failed for normals.\n");
  }

  for (i = 4; i < 4 + 2; i++)
  {
    if (isnormal(test_ld[i].number.ld))
    {
      printf("Test failed.  Testing isnormal(%La) for a %s %s: %d\n",  test_ld[i].number.ld, test_ld[i].class, test_ld[i].type, isnormal(test_ld[i].number.ld));
      break;
    }
    else if (isnormal(test_d[i].number.d))
    {
      printf("Test failed.  Testing isnormal(%a) for a %s %s: %d\n",   test_d[i].number.d,   test_d[i].class,  test_d[i].type,  isnormal(test_d[i].number.d));
      break;
    }
    else if (isnormal(test_f[i].number.f))
    {
      printf("Test failed.  Testing isnormal(%a) for a %s %s: %d\n\n", test_f[i].number.f,   test_f[i].class,  test_f[i].type,  isnormal(test_f[i].number.f));
      break;
    }
  }
  if (i < 4 + 2)
  {
    test_passed = false;
    printf("isnormal(x) failed for subnormals aka denormals.\n");
  }

  for (i = 4 + 2; i < 4 + 2 + 2; i++)
  {
    if (isnormal(test_ld[i].number.ld))
    {
      printf("Test failed.  Testing isnormal(%La) for a %s %s: %d\n",  test_ld[i].number.ld, test_ld[i].class, test_ld[i].type, isnormal(test_ld[i].number.ld));
      break;
    }
    else if (isnormal(test_d[i].number.d))
    {
      printf("Test failed.  Testing isnormal(%a) for a %s %s: %d\n",   test_d[i].number.d,   test_d[i].class,  test_d[i].type,  isnormal(test_d[i].number.d));
      break;
    }
    else if (isnormal(test_f[i].number.f))
    {
      printf("Test failed.  Testing isnormal(%a) for a %s %s: %d\n\n", test_f[i].number.f,   test_f[i].class,  test_f[i].type,  isnormal(test_f[i].number.f));
      break;
    }
  }
  if (i < 4 + 2 + 2)
  {
    test_passed = false;
    printf("isnormal(x) failed for zero.\n");
  }

  for (i = 4 + 2 + 2; i < 4 + 2 + 2 + 2; i++)
  {
    if (isnormal(test_ld[i].number.ld))
    {
      printf("Test failed.  Testing isnormal(%La) for a %s %s: %d\n",  test_ld[i].number.ld, test_ld[i].class, test_ld[i].type, isnormal(test_ld[i].number.ld));
      break;
    }
    else if (isnormal(test_d[i].number.d))
    {
      printf("Test failed.  Testing isnormal(%a) for a %s %s: %d\n",   test_d[i].number.d,   test_d[i].class,  test_d[i].type,  isnormal(test_d[i].number.d));
      break;
    }
    else if (isnormal(test_f[i].number.f))
    {
      printf("Test failed.  Testing isnormal(%a) for a %s %s: %d\n\n", test_f[i].number.f,   test_f[i].class,  test_f[i].type,  isnormal(test_f[i].number.f));
      break;
    }
  }
  if (i < 4 + 2 + 2 + 2)
  {
    test_passed = false;
    printf("isnormal(x) failed for infinity.\n");
  }

  for (i = 4 + 2 + 2 + 2; i < 4 + 2 + 2 + 2 + 4; i++)
  {
    if (isnormal(test_ld[i].number.ld))
    {
      printf("Test failed.  Testing isnormal(%La) for a %s %s: %d\n",  test_ld[i].number.ld, test_ld[i].class, test_ld[i].type, isnormal(test_ld[i].number.ld));
      break;
    }
    else if (isnormal(test_d[i].number.d))
    {
      printf("Test failed.  Testing isnormal(%a) for a %s %s: %d\n",   test_d[i].number.d,   test_d[i].class,  test_d[i].type,  isnormal(test_d[i].number.d));
      break;
    }
    else if (isnormal(test_f[i].number.f))
    {
      printf("Test failed.  Testing isnormal(%a) for a %s %s: %d\n\n", test_f[i].number.f,   test_f[i].class,  test_f[i].type,  isnormal(test_f[i].number.f));
      break;
    }
  }
  if (i < 4 + 2 + 2 + 2 + 4)
  {
    test_passed = false;
    printf("isnormal(x) failed for SNaN and/or QNaN.\n");
  }
  else if (test_passed)
    printf("isnormal(x) passed all tests.\n");
  printf("----------------------------------------------------------------------\n");



  /*
   *  isinf must return non-zero for infinity (positive or negative)
   *  and zero else.
   */
  test_passed = true;
  for (i = 0; i < 4; i++)
  {
    if (isinf(test_ld[i].number.ld))
    {
      printf("Test failed.  Testing isinf(%La) for a %s %s: %d\n",  test_ld[i].number.ld, test_ld[i].class, test_ld[i].type, isinf(test_ld[i].number.ld));
      break;
    }
    else if (isinf(test_d[i].number.d))
    {
      printf("Test failed.  Testing isinf(%a) for a %s %s: %d\n",   test_d[i].number.d,   test_d[i].class,  test_d[i].type,  isinf(test_d[i].number.d));
      break;
    }
    else if (isinf(test_f[i].number.f))
    {
      printf("Test failed.  Testing isinf(%a) for a %s %s: %d\n\n", test_f[i].number.f,   test_f[i].class,  test_f[i].type,  isinf(test_f[i].number.f));
      break;
    }
  }
  if (i < 4)
  {
    test_passed = false;
    printf("isinf(x) failed for normals.\n");
  }

  for (i = 4; i < 4 + 2; i++)
  {
    if (isinf(test_ld[i].number.ld))
    {
      printf("Test failed.  Testing isinf(%La) for a %s %s: %d\n",  test_ld[i].number.ld, test_ld[i].class, test_ld[i].type, isinf(test_ld[i].number.ld));
      break;
    }
    else if (isinf(test_d[i].number.d))
    {
      printf("Test failed.  Testing isinf(%a) for a %s %s: %d\n",   test_d[i].number.d,   test_d[i].class,  test_d[i].type,  isinf(test_d[i].number.d));
      break;
    }
    else if (isinf(test_f[i].number.f))
    {
      printf("Test failed.  Testing isinf(%a) for a %s %s: %d\n\n", test_f[i].number.f,   test_f[i].class,  test_f[i].type,  isinf(test_f[i].number.f));
      break;
    }
  }
  if (i < 4 + 2)
  {
    test_passed = false;
    printf("isinf(x) failed for subnormals aka denormals.\n");
  }

  for (i = 4 + 2; i < 4 + 2 + 2; i++)
  {
    if (isinf(test_ld[i].number.ld))
    {
      printf("Test failed.  Testing isinf(%La) for a %s %s: %d\n",  test_ld[i].number.ld, test_ld[i].class, test_ld[i].type, isinf(test_ld[i].number.ld));
      break;
    }
    else if (isinf(test_d[i].number.d))
    {
      printf("Test failed.  Testing isinf(%a) for a %s %s: %d\n",   test_d[i].number.d,   test_d[i].class,  test_d[i].type,  isinf(test_d[i].number.d));
      break;
    }
    else if (isinf(test_f[i].number.f))
    {
      printf("Test failed.  Testing isinf(%a) for a %s %s: %d\n\n", test_f[i].number.f,   test_f[i].class,  test_f[i].type,  isinf(test_f[i].number.f));
      break;
    }
  }
  if (i < 4 + 2 + 2)
  {
    test_passed = false;
    printf("isinf(x) failed for zero.\n");
  }

  for (i = 4 + 2 + 2; i < 4 + 2 + 2 + 2; i++)
  {
    if (!isinf(test_ld[i].number.ld))
    {
      printf("Test failed.  Testing isinf(%La) for a %s %s: %d\n",  test_ld[i].number.ld, test_ld[i].class, test_ld[i].type, isinf(test_ld[i].number.ld));
      break;
    }
    else if (!isinf(test_d[i].number.d))
    {
      printf("Test failed.  Testing isinf(%a) for a %s %s: %d\n",   test_d[i].number.d,   test_d[i].class,  test_d[i].type,  isinf(test_d[i].number.d));
      break;
    }
    else if (!isinf(test_f[i].number.f))
    {
      printf("Test failed.  Testing isinf(%a) for a %s %s: %d\n\n", test_f[i].number.f,   test_f[i].class,  test_f[i].type,  isinf(test_f[i].number.f));
      break;
    }
  }
  if (i < 4 + 2 + 2 + 2)
  {
    test_passed = false;
    printf("isinf(x) failed for infinity.\n");
  }

  for (i = 4 + 2 + 2 + 2; i < 4 + 2 + 2 + 2 + 4; i++)
  {
    if (isinf(test_ld[i].number.ld))
    {
      printf("Test failed.  Testing isinf(%La) for a %s %s: %d\n",  test_ld[i].number.ld, test_ld[i].class, test_ld[i].type, isinf(test_ld[i].number.ld));
      break;
    }
    else if (isinf(test_d[i].number.d))
    {
      printf("Test failed.  Testing isinf(%a) for a %s %s: %d\n",   test_d[i].number.d,   test_d[i].class,  test_d[i].type,  isinf(test_d[i].number.d));
      break;
    }
    else if (isinf(test_f[i].number.f))
    {
      printf("Test failed.  Testing isinf(%a) for a %s %s: %d\n\n", test_f[i].number.f,   test_f[i].class,  test_f[i].type,  isinf(test_f[i].number.f));
      break;
    }
  }
  if (i < 4 + 2 + 2 + 2 + 4)
  {
    test_passed = false;
    printf("isinf(x) failed for SNaN and/or QNaN.\n");
  }
  else if (test_passed)
    printf("isinf(x) passed all tests.\n");
  printf("----------------------------------------------------------------------\n");



  /*
   *  isnan must return non-zero for [SQ]NaN (positive or negative)
   *  and zero else.
   */
  test_passed = true;
  for (i = 0; i < 4; i++)
  {
    if (isnan(test_ld[i].number.ld))
    {
      printf("Test failed.  Testing isnan(%La) for a %s %s: %d\n",  test_ld[i].number.ld, test_ld[i].class, test_ld[i].type, isnan(test_ld[i].number.ld));
      break;
    }
    else if (isnan(test_d[i].number.d))
    {
      printf("Test failed.  Testing isnan(%a) for a %s %s: %d\n",   test_d[i].number.d,   test_d[i].class,  test_d[i].type,  isnan(test_d[i].number.d));
      break;
    }
    else if (isnan(test_f[i].number.f))
    {
      printf("Test failed.  Testing isnan(%a) for a %s %s: %d\n\n", test_f[i].number.f,   test_f[i].class,  test_f[i].type,  isnan(test_f[i].number.f));
      break;
    }
  }
  if (i < 4)
  {
    test_passed = false;
    printf("isnan(x) failed for normals.\n");
  }

  for (i = 4; i < 4 + 2; i++)
  {
    if (isnan(test_ld[i].number.ld))
    {
      printf("Test failed.  Testing isnan(%La) for a %s %s: %d\n",  test_ld[i].number.ld, test_ld[i].class, test_ld[i].type, isnan(test_ld[i].number.ld));
      break;
    }
    else if (isnan(test_d[i].number.d))
    {
      printf("Test failed.  Testing isnan(%a) for a %s %s: %d\n",   test_d[i].number.d,   test_d[i].class,  test_d[i].type,  isnan(test_d[i].number.d));
      break;
    }
    else if (isnan(test_f[i].number.f))
    {
      printf("Test failed.  Testing isnan(%a) for a %s %s: %d\n\n", test_f[i].number.f,   test_f[i].class,  test_f[i].type,  isnan(test_f[i].number.f));
      break;
    }
  }
  if (i < 4 + 2)
  {
    test_passed = false;
    printf("isnan(x) failed for subnormals aka denormals.\n");
  }

  for (i = 4 + 2; i < 4 + 2 + 2; i++)
  {
    if (isnan(test_ld[i].number.ld))
    {
      printf("Test failed.  Testing isnan(%La) for a %s %s: %d\n",  test_ld[i].number.ld, test_ld[i].class, test_ld[i].type, isnan(test_ld[i].number.ld));
      break;
    }
    else if (isnan(test_d[i].number.d))
    {
      printf("Test failed.  Testing isnan(%a) for a %s %s: %d\n",   test_d[i].number.d,   test_d[i].class,  test_d[i].type,  isnan(test_d[i].number.d));
      break;
    }
    else if (isnan(test_f[i].number.f))
    {
      printf("Test failed.  Testing isnan(%a) for a %s %s: %d\n\n", test_f[i].number.f,   test_f[i].class,  test_f[i].type,  isnan(test_f[i].number.f));
      break;
    }
  }
  if (i < 4 + 2 + 2)
  {
    test_passed = false;
    printf("isnan(x) failed for zero.\n");
  }

  for (i = 4 + 2 + 2; i < 4 + 2 + 2 + 2; i++)
  {
    if (isnan(test_ld[i].number.ld))
    {
      printf("Test failed.  Testing isnan(%La) for a %s %s: %d\n",  test_ld[i].number.ld, test_ld[i].class, test_ld[i].type, isnan(test_ld[i].number.ld));
      break;
    }
    else if (isnan(test_d[i].number.d))
    {
      printf("Test failed.  Testing isnan(%a) for a %s %s: %d\n",   test_d[i].number.d,   test_d[i].class,  test_d[i].type,  isnan(test_d[i].number.d));
      break;
    }
    else if (isnan(test_f[i].number.f))
    {
      printf("Test failed.  Testing isnan(%a) for a %s %s: %d\n\n", test_f[i].number.f,   test_f[i].class,  test_f[i].type,  isnan(test_f[i].number.f));
      break;
    }
  }
  if (i < 4 + 2 + 2 + 2)
  {
    test_passed = false;
    printf("isnan(x) failed for infinity.\n");
  }

  for (i = 4 + 2 + 2 + 2; i < 4 + 2 + 2 + 2 + 4; i++)
  {
    if (!isnan(test_ld[i].number.ld))
    {
      printf("Test failed.  Testing isnan(%La) for a %s %s: %d\n",  test_ld[i].number.ld, test_ld[i].class, test_ld[i].type, isnan(test_ld[i].number.ld));
      break;
    }
    else if (!isnan(test_d[i].number.d))
    {
      printf("Test failed.  Testing isnan(%a) for a %s %s: %d\n",   test_d[i].number.d,   test_d[i].class,  test_d[i].type,  isnan(test_d[i].number.d));
      break;
    }
    else if (!isnan(test_f[i].number.f))
    {
      printf("Test failed.  Testing isnan(%a) for a %s %s: %d\n\n", test_f[i].number.f,   test_f[i].class,  test_f[i].type,  isnan(test_f[i].number.f));
      break;
    }
  }
  if (i < 4 + 2 + 2 + 2 + 4)
  {
    test_passed = false;
    printf("isnan(x) failed for SNaN and/or QNaN.\n");
  }
  else if (test_passed)
    printf("isnan(x) passed all tests.\n");
  printf("----------------------------------------------------------------------\n");



  /*
   *  isgreater must return zero if x or y is [SQ]NaN (positive or negative)
   *  and else the result of x > y.
   */
  test_passed = true;
  for (i = 0; i < 4; i += 3)
  {
    if (isgreater(test_ld[i].number.ld, test_d[i].number.d))
    {
      printf("Test failed.  Testing isgreater(%La, %a) for a %s %s and %s %s: %d\n",   test_ld[i].number.ld, test_d[i].number.d,   test_ld[i].class, test_ld[i].type, test_d[i].class,  test_d[i].type,  isgreater(test_ld[i].number.ld, test_d[i].number.d));
      break;
    }
    else if (isgreater(test_d[i].number.d, test_f[i].number.f))
    {
      printf("Test failed.  Testing isgreater(%a, %a) for a %s %s and %s %s: %d\n",    test_d[i].number.d,   test_f[i].number.f,   test_d[i].class,  test_d[i].type,  test_f[i].class,  test_f[i].type,  isgreater(test_d[i].number.d, test_f[i].number.f));
      break;
    }
    else if (!isgreater(test_f[i].number.f, test_ld[i].number.ld))
    {
      printf("Test failed.  Testing isgreater(%a, %La) for a %s %s and %s %s: %d\n\n", test_f[i].number.f,   test_ld[i].number.ld, test_f[i].class,  test_f[i].type,  test_ld[i].class, test_ld[i].type, isgreater(test_f[i].number.f, test_ld[i].number.ld));
      break;
    }
  }
  if (i < 4)
  {
    test_passed = false;
    printf("isgreater(x, y) failed for normals.\n");
  }

  for (i = 1; i < 4 - 1; i++)
  {
    if (!isgreater(test_ld[i].number.ld, test_d[i].number.d))
    {
      printf("Test failed.  Testing isgreater(%La, %a) for a %s %s and %s %s: %d\n",   test_ld[i].number.ld, test_d[i].number.d,   test_ld[i].class, test_ld[i].type, test_d[i].class,  test_d[i].type,  isgreater(test_ld[i].number.ld, test_d[i].number.d));
      break;
    }
    else if (!isgreater(test_d[i].number.d, test_f[i].number.f))
    {
      printf("Test failed.  Testing isgreater(%a, %a) for a %s %s and %s %s: %d\n",    test_d[i].number.d,   test_f[i].number.f,   test_d[i].class,  test_d[i].type,  test_f[i].class,  test_f[i].type,  isgreater(test_d[i].number.d, test_f[i].number.f));
      break;
    }
    else if (isgreater(test_f[i].number.f, test_ld[i].number.ld))
    {
      printf("Test failed.  Testing isgreater(%a, %La) for a %s %s and %s %s: %d\n\n", test_f[i].number.f,   test_ld[i].number.ld, test_f[i].class,  test_f[i].type,  test_ld[i].class, test_ld[i].type, isgreater(test_f[i].number.f, test_ld[i].number.ld));
      break;
    }
  }
  if (i < 4 - 1)
  {
    test_passed = false;
    printf("isgreater(x, y) failed for normals.\n");
  }

  for (i = 4; i < 4 + 1; i++)
  {
    if (isgreater(test_ld[i].number.ld, test_d[i].number.d))
    {
      printf("Test failed.  Testing isgreater(%La, %a) for a %s %s and %s %s: %d\n",   test_ld[i].number.ld, test_d[i].number.d,   test_ld[i].class, test_ld[i].type, test_d[i].class,  test_d[i].type,  isgreater(test_ld[i].number.ld, test_d[i].number.d));
      break;
    }
    else if (isgreater(test_d[i].number.d, test_f[i].number.f))
    {
      printf("Test failed.  Testing isgreater(%a, %a) for a %s %s and %s %s: %d\n",    test_d[i].number.d,   test_f[i].number.f,   test_d[i].class,  test_d[i].type,  test_f[i].class,  test_f[i].type,  isgreater(test_d[i].number.d, test_f[i].number.f));
      break;
    }
    else if (!isgreater(test_f[i].number.f, test_ld[i].number.ld))
    {
      printf("Test failed.  Testing isgreater(%a, %La) for a %s %s and %s %s: %d\n\n", test_f[i].number.f,   test_ld[i].number.ld, test_f[i].class,  test_f[i].type,  test_ld[i].class, test_ld[i].type, isgreater(test_f[i].number.f, test_ld[i].number.ld));
      break;
    }
  }
  if (i < 4 + 1)
  {
    test_passed = false;
    printf("isgreater(x, y) failed for subnormals.\n");
  }

  for (i = 4 + 1; i < 4 + 2; i++)
  {
    if (!isgreater(test_ld[i].number.ld, test_d[i].number.d))
    {
      printf("Test failed.  Testing isgreater(%La, %a) for a %s %s and %s %s: %d\n",   test_ld[i].number.ld, test_d[i].number.d,   test_ld[i].class, test_ld[i].type, test_d[i].class,  test_d[i].type,  isgreater(test_ld[i].number.ld, test_d[i].number.d));
      break;
    }
    else if (!isgreater(test_d[i].number.d, test_f[i].number.f))
    {
      printf("Test failed.  Testing isgreater(%a, %a) for a %s %s and %s %s: %d\n",    test_d[i].number.d,   test_f[i].number.f,   test_d[i].class,  test_d[i].type,  test_f[i].class,  test_f[i].type,  isgreater(test_d[i].number.d, test_f[i].number.f));
      break;
    }
    else if (isgreater(test_f[i].number.f, test_ld[i].number.ld))
    {
      printf("Test failed.  Testing isgreater(%a, %La) for a %s %s and %s %s: %d\n\n", test_f[i].number.f,   test_ld[i].number.ld, test_f[i].class,  test_f[i].type,  test_ld[i].class, test_ld[i].type, isgreater(test_f[i].number.f, test_ld[i].number.ld));
      break;
    }
  }
  if (i < 4 + 2)
  {
    test_passed = false;
    printf("isgreater(x, y) failed for subnormals.\n");
  }

  for (i = 4 + 2; i < 4 + 2 + 2; i++)
  {
    if (isgreater(test_ld[i].number.ld, test_d[i].number.d))
    {
      printf("Test failed.  Testing isgreater(%La, %a) for a %s %s and %s %s: %d\n",   test_ld[i].number.ld, test_d[i].number.d,   test_ld[i].class, test_ld[i].type, test_d[i].class,  test_d[i].type,  isgreater(test_ld[i].number.ld, test_d[i].number.d));
      break;
    }
    else if (isgreater(test_d[i].number.d, test_f[i].number.f))
    {
      printf("Test failed.  Testing isgreater(%a, %a) for a %s %s and %s %s: %d\n",    test_d[i].number.d,   test_f[i].number.f,   test_d[i].class,  test_d[i].type,  test_f[i].class,  test_f[i].type,  isgreater(test_d[i].number.d, test_f[i].number.f));
      break;
    }
    else if (isgreater(test_f[i].number.f, test_ld[i].number.ld))
    {
      printf("Test failed.  Testing isgreater(%a, %La) for a %s %s and %s %s: %d\n\n", test_f[i].number.f,   test_ld[i].number.ld, test_f[i].class,  test_f[i].type,  test_ld[i].class, test_ld[i].type, isgreater(test_f[i].number.f, test_ld[i].number.ld));
      break;
    }
  }
  if (i < 4 + 2 + 2)
  {
    test_passed = false;
    printf("isgreater(x, y) failed for zero.\n");
  }

  for (i = 4 + 2 + 2; i < 4 + 2 + 2 + 2; i++)
  {
    if (isgreater(test_ld[i].number.ld, test_d[i].number.d))
    {
      printf("Test failed.  Testing isgreater(%La, %a) for a %s %s and %s %s: %d\n",   test_ld[i].number.ld, test_d[i].number.d,   test_ld[i].class, test_ld[i].type, test_d[i].class,  test_d[i].type,  isgreater(test_ld[i].number.ld, test_d[i].number.d));
      break;
    }
    else if (isgreater(test_d[i].number.d, test_f[i].number.f))
    {
      printf("Test failed.  Testing isgreater(%a, %a) for a %s %s and %s %s: %d\n",    test_d[i].number.d,   test_f[i].number.f,   test_d[i].class,  test_d[i].type,  test_f[i].class,  test_f[i].type,  isgreater(test_d[i].number.d, test_f[i].number.f));
      break;
    }
    else if (isgreater(test_f[i].number.f, test_ld[i].number.ld))
    {
      printf("Test failed.  Testing isgreater(%a, %La) for a %s %s and %s %s: %d\n\n", test_f[i].number.f,   test_ld[i].number.ld, test_f[i].class,  test_f[i].type,  test_ld[i].class, test_ld[i].type, isgreater(test_f[i].number.f, test_ld[i].number.ld));
      break;
    }
  }
  if (i < 4 + 2 + 2 + 2)
  {
    test_passed = false;
    printf("isgreater(x, y) failed for infinity.\n");
  }

  for (i = 4 + 2 + 2 + 2; i < 4 + 2 + 2 + 2 + 4; i++)
  {
    if (isgreater(test_ld[i].number.ld, test_d[i].number.d))
    {
      printf("Test failed.  Testing isgreater(%La, %a) for a %s %s and %s %s: %d\n",   test_ld[i].number.ld, test_d[i].number.d,   test_ld[i].class, test_ld[i].type, test_d[i].class,  test_d[i].type,  isgreater(test_ld[i].number.ld, test_d[i].number.d));
      break;
    }
    else if (isgreater(test_d[i].number.d, test_f[i].number.f))
    {
      printf("Test failed.  Testing isgreater(%a, %a) for a %s %s and %s %s: %d\n",    test_d[i].number.d,   test_f[i].number.f,   test_d[i].class,  test_d[i].type,  test_f[i].class,  test_f[i].type,  isgreater(test_d[i].number.d, test_f[i].number.f));
      break;
    }
    else if (isgreater(test_f[i].number.f, test_ld[i].number.ld))
    {
      printf("Test failed.  Testing isgreater(%a, %La) for a %s %s and %s %s: %d\n\n", test_f[i].number.f,   test_ld[i].number.ld, test_f[i].class,  test_f[i].type,  test_ld[i].class, test_ld[i].type, isgreater(test_f[i].number.f, test_ld[i].number.ld));
      break;
    }
  }
  if (i < 4 + 2 + 2 + 2 + 4)
  {
    test_passed = false;
    printf("isgreater(x, y) failed for SNaN and/or QNaN.\n");
  }
  else if (test_passed)
    printf("isgreater(x, y) passed all tests.\n");
  printf("----------------------------------------------------------------------\n");



  /*
   *  isgreaterequal must return zero if x or y is [SQ]NaN (positive or negative)
   *  and else the result of x >= y.
   */
  test_passed = true;
  for (i = 0; i < 4; i += 3)
  {
    if (isgreaterequal(test_ld[i].number.ld, test_d[i].number.d))
    {
      printf("Test failed.  Testing isgreaterequal(%La, %a) for a %s %s and %s %s: %d\n",   test_ld[i].number.ld, test_d[i].number.d,   test_ld[i].class, test_ld[i].type, test_d[i].class,  test_d[i].type,  isgreaterequal(test_ld[i].number.ld, test_d[i].number.d));
      break;
    }
    else if (isgreaterequal(test_d[i].number.d, test_f[i].number.f))
    {
      printf("Test failed.  Testing isgreaterequal(%a, %a) for a %s %s and %s %s: %d\n",    test_d[i].number.d,   test_f[i].number.f,   test_d[i].class,  test_d[i].type,  test_f[i].class,  test_f[i].type,  isgreaterequal(test_d[i].number.d, test_f[i].number.f));
      break;
    }
    else if (!isgreaterequal(test_f[i].number.f, test_ld[i].number.ld))
    {
      printf("Test failed.  Testing isgreaterequal(%a, %La) for a %s %s and %s %s: %d\n\n", test_f[i].number.f,   test_ld[i].number.ld, test_f[i].class,  test_f[i].type,  test_ld[i].class, test_ld[i].type, isgreaterequal(test_f[i].number.f, test_ld[i].number.ld));
      break;
    }
  }
  if (i < 4)
  {
    test_passed = false;
    printf("isgreaterequal(x, y) failed for normals.\n");
  }

  for (i = 1; i < 4 - 1; i++)
  {
    if (!isgreaterequal(test_ld[i].number.ld, test_d[i].number.d))
    {
      printf("Test failed.  Testing isgreaterequal(%La, %a) for a %s %s and %s %s: %d\n",   test_ld[i].number.ld, test_d[i].number.d,   test_ld[i].class, test_ld[i].type, test_d[i].class,  test_d[i].type,  isgreaterequal(test_ld[i].number.ld, test_d[i].number.d));
      break;
    }
    else if (!isgreaterequal(test_d[i].number.d, test_f[i].number.f))
    {
      printf("Test failed.  Testing isgreaterequal(%a, %a) for a %s %s and %s %s: %d\n",    test_d[i].number.d,   test_f[i].number.f,   test_d[i].class,  test_d[i].type,  test_f[i].class,  test_f[i].type,  isgreaterequal(test_d[i].number.d, test_f[i].number.f));
      break;
    }
    else if (isgreaterequal(test_f[i].number.f, test_ld[i].number.ld))
    {
      printf("Test failed.  Testing isgreaterequal(%a, %La) for a %s %s and %s %s: %d\n\n", test_f[i].number.f,   test_ld[i].number.ld, test_f[i].class,  test_f[i].type,  test_ld[i].class, test_ld[i].type, isgreaterequal(test_f[i].number.f, test_ld[i].number.ld));
      break;
    }
  }
  if (i < 4 - 1)
  {
    test_passed = false;
    printf("isgreaterequal(x, y) failed for normals.\n");
  }

  for (i = 4; i < 4 + 1; i++)
  {
    if (isgreaterequal(test_ld[i].number.ld, test_d[i].number.d))
    {
      printf("Test failed.  Testing isgreaterequal(%La, %a) for a %s %s and %s %s: %d\n",   test_ld[i].number.ld, test_d[i].number.d,   test_ld[i].class, test_ld[i].type, test_d[i].class,  test_d[i].type,  isgreaterequal(test_ld[i].number.ld, test_d[i].number.d));
      break;
    }
    else if (isgreaterequal(test_d[i].number.d, test_f[i].number.f))
    {
      printf("Test failed.  Testing isgreaterequal(%a, %a) for a %s %s and %s %s: %d\n",    test_d[i].number.d,   test_f[i].number.f,   test_d[i].class,  test_d[i].type,  test_f[i].class,  test_f[i].type,  isgreaterequal(test_d[i].number.d, test_f[i].number.f));
      break;
    }
    else if (!isgreaterequal(test_f[i].number.f, test_ld[i].number.ld))
    {
      printf("Test failed.  Testing isgreaterequal(%a, %La) for a %s %s and %s %s: %d\n\n", test_f[i].number.f,   test_ld[i].number.ld, test_f[i].class,  test_f[i].type,  test_ld[i].class, test_ld[i].type, isgreaterequal(test_f[i].number.f, test_ld[i].number.ld));
      break;
    }
  }
  if (i < 4 + 1)
  {
    test_passed = false;
    printf("isgreaterequal(x, y) failed for subnormals.\n");
  }

  for (i = 4 + 1; i < 4 + 2; i++)
  {
    if (!isgreaterequal(test_ld[i].number.ld, test_d[i].number.d))
    {
      printf("Test failed.  Testing isgreaterequal(%La, %a) for a %s %s and %s %s: %d\n",   test_ld[i].number.ld, test_d[i].number.d,   test_ld[i].class, test_ld[i].type, test_d[i].class,  test_d[i].type,  isgreaterequal(test_ld[i].number.ld, test_d[i].number.d));
      break;
    }
    else if (!isgreaterequal(test_d[i].number.d, test_f[i].number.f))
    {
      printf("Test failed.  Testing isgreaterequal(%a, %a) for a %s %s and %s %s: %d\n",    test_d[i].number.d,   test_f[i].number.f,   test_d[i].class,  test_d[i].type,  test_f[i].class,  test_f[i].type,  isgreaterequal(test_d[i].number.d, test_f[i].number.f));
      break;
    }
    else if (isgreaterequal(test_f[i].number.f, test_ld[i].number.ld))
    {
      printf("Test failed.  Testing isgreaterequal(%a, %La) for a %s %s and %s %s: %d\n\n", test_f[i].number.f,   test_ld[i].number.ld, test_f[i].class,  test_f[i].type,  test_ld[i].class, test_ld[i].type, isgreaterequal(test_f[i].number.f, test_ld[i].number.ld));
      break;
    }
  }
  if (i < 4 + 2)
  {
    test_passed = false;
    printf("isgreaterequal(x, y) failed for subnormals.\n");
  }

  for (i = 4 + 2; i < 4 + 2 + 2; i++)
  {
    if (!isgreaterequal(test_ld[i].number.ld, test_d[i].number.d))
    {
      printf("Test failed.  Testing isgreaterequal(%La, %a) for a %s %s and %s %s: %d\n",   test_ld[i].number.ld, test_d[i].number.d,   test_ld[i].class, test_ld[i].type, test_d[i].class,  test_d[i].type,  isgreaterequal(test_ld[i].number.ld, test_d[i].number.d));
      break;
    }
    else if (!isgreaterequal(test_d[i].number.d, test_f[i].number.f))
    {
      printf("Test failed.  Testing isgreaterequal(%a, %a) for a %s %s and %s %s: %d\n",    test_d[i].number.d,   test_f[i].number.f,   test_d[i].class,  test_d[i].type,  test_f[i].class,  test_f[i].type,  isgreaterequal(test_d[i].number.d, test_f[i].number.f));
      break;
    }
    else if (!isgreaterequal(test_f[i].number.f, test_ld[i].number.ld))
    {
      printf("Test failed.  Testing isgreaterequal(%a, %La) for a %s %s and %s %s: %d\n\n", test_f[i].number.f,   test_ld[i].number.ld, test_f[i].class,  test_f[i].type,  test_ld[i].class, test_ld[i].type, isgreaterequal(test_f[i].number.f, test_ld[i].number.ld));
      break;
    }
  }
  if (i < 4 + 2 + 2)
  {
    test_passed = false;
    printf("isgreaterequal(x, y) failed for zero.\n");
  }

  for (i = 4 + 2 + 2; i < 4 + 2 + 2 + 2; i++)
  {
    if (!isgreaterequal(test_ld[i].number.ld, test_d[i].number.d))
    {
      printf("Test failed.  Testing isgreaterequal(%La, %a) for a %s %s and %s %s: %d\n",   test_ld[i].number.ld, test_d[i].number.d,   test_ld[i].class, test_ld[i].type, test_d[i].class,  test_d[i].type,  isgreaterequal(test_ld[i].number.ld, test_d[i].number.d));
      break;
    }
    else if (!isgreaterequal(test_d[i].number.d, test_f[i].number.f))
    {
      printf("Test failed.  Testing isgreaterequal(%a, %a) for a %s %s and %s %s: %d\n",    test_d[i].number.d,   test_f[i].number.f,   test_d[i].class,  test_d[i].type,  test_f[i].class,  test_f[i].type,  isgreaterequal(test_d[i].number.d, test_f[i].number.f));
      break;
    }
    else if (!isgreaterequal(test_f[i].number.f, test_ld[i].number.ld))
    {
      printf("Test failed.  Testing isgreaterequal(%a, %La) for a %s %s and %s %s: %d\n\n", test_f[i].number.f,   test_ld[i].number.ld, test_f[i].class,  test_f[i].type,  test_ld[i].class, test_ld[i].type, isgreaterequal(test_f[i].number.f, test_ld[i].number.ld));
      break;
    }
  }
  if (i < 4 + 2 + 2 + 2)
  {
    test_passed = false;
    printf("isgreaterequal(x, y) failed for infinity.\n");
  }

  for (i = 4 + 2 + 2 + 2; i < 4 + 2 + 2 + 2 + 4; i++)
  {
    if (isgreaterequal(test_ld[i].number.ld, test_d[i].number.d))
    {
      printf("Test failed.  Testing isgreaterequal(%La, %a) for a %s %s and %s %s: %d\n",   test_ld[i].number.ld, test_d[i].number.d,   test_ld[i].class, test_ld[i].type, test_d[i].class,  test_d[i].type,  isgreaterequal(test_ld[i].number.ld, test_d[i].number.d));
      break;
    }
    else if (isgreaterequal(test_d[i].number.d, test_f[i].number.f))
    {
      printf("Test failed.  Testing isgreaterequal(%a, %a) for a %s %s and %s %s: %d\n",    test_d[i].number.d,   test_f[i].number.f,   test_d[i].class,  test_d[i].type,  test_f[i].class,  test_f[i].type,  isgreaterequal(test_d[i].number.d, test_f[i].number.f));
      break;
    }
    else if (isgreaterequal(test_f[i].number.f, test_ld[i].number.ld))
    {
      printf("Test failed.  Testing isgreaterequal(%a, %La) for a %s %s and %s %s: %d\n\n", test_f[i].number.f,   test_ld[i].number.ld, test_f[i].class,  test_f[i].type,  test_ld[i].class, test_ld[i].type, isgreaterequal(test_f[i].number.f, test_ld[i].number.ld));
      break;
    }
  }
  if (i < 4 + 2 + 2 + 2 + 4)
  {
    test_passed = false;
    printf("isgreaterequal(x, y) failed for SNaN and/or QNaN.\n");
  }
  else if (test_passed)
    printf("isgreaterequal(x, y) passed all tests.\n");
  printf("----------------------------------------------------------------------\n");



  /*
   *  isless must return zero if x or y is [SQ]NaN (positive or negative)
   *  and else the result of x < y.
   */
  test_passed = true;
  for (i = 0; i < 4; i += 3)
  {
    if (!isless(test_ld[i].number.ld, test_d[i].number.d))
    {
      printf("Test failed.  Testing isless(%La, %a) for a %s %s and %s %s: %d\n",   test_ld[i].number.ld, test_d[i].number.d,   test_ld[i].class, test_ld[i].type, test_d[i].class,  test_d[i].type,  isless(test_ld[i].number.ld, test_d[i].number.d));
      break;
    }
    else if (!isless(test_d[i].number.d, test_f[i].number.f))
    {
      printf("Test failed.  Testing isless(%a, %a) for a %s %s and %s %s: %d\n",    test_d[i].number.d,   test_f[i].number.f,   test_d[i].class,  test_d[i].type,  test_f[i].class,  test_f[i].type,  isless(test_d[i].number.d, test_f[i].number.f));
      break;
    }
    else if (isless(test_f[i].number.f, test_ld[i].number.ld))
    {
      printf("Test failed.  Testing isless(%a, %La) for a %s %s and %s %s: %d\n\n", test_f[i].number.f,   test_ld[i].number.ld, test_f[i].class,  test_f[i].type,  test_ld[i].class, test_ld[i].type, isless(test_f[i].number.f, test_ld[i].number.ld));
      break;
    }
  }
  if (i < 4)
  {
    test_passed = false;
    printf("isless(x, y) failed for normals.\n");
  }

  for (i = 1; i < 4 - 1; i++)
  {
    if (isless(test_ld[i].number.ld, test_d[i].number.d))
    {
      printf("Test failed.  Testing isless(%La, %a) for a %s %s and %s %s: %d\n",   test_ld[i].number.ld, test_d[i].number.d,   test_ld[i].class, test_ld[i].type, test_d[i].class,  test_d[i].type,  isless(test_ld[i].number.ld, test_d[i].number.d));
      break;
    }
    else if (isless(test_d[i].number.d, test_f[i].number.f))
    {
      printf("Test failed.  Testing isless(%a, %a) for a %s %s and %s %s: %d\n",    test_d[i].number.d,   test_f[i].number.f,   test_d[i].class,  test_d[i].type,  test_f[i].class,  test_f[i].type,  isless(test_d[i].number.d, test_f[i].number.f));
      break;
    }
    else if (!isless(test_f[i].number.f, test_ld[i].number.ld))
    {
      printf("Test failed.  Testing isless(%a, %La) for a %s %s and %s %s: %d\n\n", test_f[i].number.f,   test_ld[i].number.ld, test_f[i].class,  test_f[i].type,  test_ld[i].class, test_ld[i].type, isless(test_f[i].number.f, test_ld[i].number.ld));
      break;
    }
  }
  if (i < 4 - 1)
  {
    test_passed = false;
    printf("isless(x, y) failed for normals.\n");
  }

  for (i = 4; i < 4 + 1; i++)
  {
    if (!isless(test_ld[i].number.ld, test_d[i].number.d))
    {
      printf("Test failed.  Testing isless(%La, %a) for a %s %s and %s %s: %d\n",   test_ld[i].number.ld, test_d[i].number.d,   test_ld[i].class, test_ld[i].type, test_d[i].class,  test_d[i].type,  isless(test_ld[i].number.ld, test_d[i].number.d));
      break;
    }
    else if (!isless(test_d[i].number.d, test_f[i].number.f))
    {
      printf("Test failed.  Testing isless(%a, %a) for a %s %s and %s %s: %d\n",    test_d[i].number.d,   test_f[i].number.f,   test_d[i].class,  test_d[i].type,  test_f[i].class,  test_f[i].type,  isless(test_d[i].number.d, test_f[i].number.f));
      break;
    }
    else if (isless(test_f[i].number.f, test_ld[i].number.ld))
    {
      printf("Test failed.  Testing isless(%a, %La) for a %s %s and %s %s: %d\n\n", test_f[i].number.f,   test_ld[i].number.ld, test_f[i].class,  test_f[i].type,  test_ld[i].class, test_ld[i].type, isless(test_f[i].number.f, test_ld[i].number.ld));
      break;
    }
  }
  if (i < 4 + 1)
  {
    test_passed = false;
    printf("isless(x, y) failed for subnormals.\n");
  }

  for (i = 4 + 1; i < 4 + 2; i++)
  {
    if (isless(test_ld[i].number.ld, test_d[i].number.d))
    {
      printf("Test failed.  Testing isless(%La, %a) for a %s %s and %s %s: %d\n",   test_ld[i].number.ld, test_d[i].number.d,   test_ld[i].class, test_ld[i].type, test_d[i].class,  test_d[i].type,  isless(test_ld[i].number.ld, test_d[i].number.d));
      break;
    }
    else if (isless(test_d[i].number.d, test_f[i].number.f))
    {
      printf("Test failed.  Testing isless(%a, %a) for a %s %s and %s %s: %d\n",    test_d[i].number.d,   test_f[i].number.f,   test_d[i].class,  test_d[i].type,  test_f[i].class,  test_f[i].type,  isless(test_d[i].number.d, test_f[i].number.f));
      break;
    }
    else if (!isless(test_f[i].number.f, test_ld[i].number.ld))
    {
      printf("Test failed.  Testing isless(%a, %La) for a %s %s and %s %s: %d\n\n", test_f[i].number.f,   test_ld[i].number.ld, test_f[i].class,  test_f[i].type,  test_ld[i].class, test_ld[i].type, isless(test_f[i].number.f, test_ld[i].number.ld));
      break;
    }
  }
  if (i < 4 + 2)
  {
    test_passed = false;
    printf("isless(x, y) failed for subnormals.\n");
  }

  for (i = 4 + 2; i < 4 + 2 + 2; i++)
  {
    if (isless(test_ld[i].number.ld, test_d[i].number.d))
    {
      printf("Test failed.  Testing isless(%La, %a) for a %s %s and %s %s: %d\n",   test_ld[i].number.ld, test_d[i].number.d,   test_ld[i].class, test_ld[i].type, test_d[i].class,  test_d[i].type,  isless(test_ld[i].number.ld, test_d[i].number.d));
      break;
    }
    else if (isless(test_d[i].number.d, test_f[i].number.f))
    {
      printf("Test failed.  Testing isless(%a, %a) for a %s %s and %s %s: %d\n",    test_d[i].number.d,   test_f[i].number.f,   test_d[i].class,  test_d[i].type,  test_f[i].class,  test_f[i].type,  isless(test_d[i].number.d, test_f[i].number.f));
      break;
    }
    else if (isless(test_f[i].number.f, test_ld[i].number.ld))
    {
      printf("Test failed.  Testing isless(%a, %La) for a %s %s and %s %s: %d\n\n", test_f[i].number.f,   test_ld[i].number.ld, test_f[i].class,  test_f[i].type,  test_ld[i].class, test_ld[i].type, isless(test_f[i].number.f, test_ld[i].number.ld));
      break;
    }
  }
  if (i < 4 + 2 + 2)
  {
    test_passed = false;
    printf("isless(x, y) failed for zero.\n");
  }

  for (i = 4 + 2 + 2; i < 4 + 2 + 2 + 2; i++)
  {
    if (isless(test_ld[i].number.ld, test_d[i].number.d))
    {
      printf("Test failed.  Testing isless(%La, %a) for a %s %s and %s %s: %d\n",   test_ld[i].number.ld, test_d[i].number.d,   test_ld[i].class, test_ld[i].type, test_d[i].class,  test_d[i].type,  isless(test_ld[i].number.ld, test_d[i].number.d));
      break;
    }
    else if (isless(test_d[i].number.d, test_f[i].number.f))
    {
      printf("Test failed.  Testing isless(%a, %a) for a %s %s and %s %s: %d\n",    test_d[i].number.d,   test_f[i].number.f,   test_d[i].class,  test_d[i].type,  test_f[i].class,  test_f[i].type,  isless(test_d[i].number.d, test_f[i].number.f));
      break;
    }
    else if (isless(test_f[i].number.f, test_ld[i].number.ld))
    {
      printf("Test failed.  Testing isless(%a, %La) for a %s %s and %s %s: %d\n\n", test_f[i].number.f,   test_ld[i].number.ld, test_f[i].class,  test_f[i].type,  test_ld[i].class, test_ld[i].type, isless(test_f[i].number.f, test_ld[i].number.ld));
      break;
    }
  }
  if (i < 4 + 2 + 2 + 2)
  {
    test_passed = false;
    printf("isless(x, y) failed for infinity.\n");
  }

  for (i = 4 + 2 + 2 + 2; i < 4 + 2 + 2 + 2 + 4; i++)
  {
    if (isless(test_ld[i].number.ld, test_d[i].number.d))
    {
      printf("Test failed.  Testing isless(%La, %a) for a %s %s and %s %s: %d\n",   test_ld[i].number.ld, test_d[i].number.d,   test_ld[i].class, test_ld[i].type, test_d[i].class,  test_d[i].type,  isless(test_ld[i].number.ld, test_d[i].number.d));
      break;
    }
    else if (isless(test_d[i].number.d, test_f[i].number.f))
    {
      printf("Test failed.  Testing isless(%a, %a) for a %s %s and %s %s: %d\n",    test_d[i].number.d,   test_f[i].number.f,   test_d[i].class,  test_d[i].type,  test_f[i].class,  test_f[i].type,  isless(test_d[i].number.d, test_f[i].number.f));
      break;
    }
    else if (isless(test_f[i].number.f, test_ld[i].number.ld))
    {
      printf("Test failed.  Testing isless(%a, %La) for a %s %s and %s %s: %d\n\n", test_f[i].number.f,   test_ld[i].number.ld, test_f[i].class,  test_f[i].type,  test_ld[i].class, test_ld[i].type, isless(test_f[i].number.f, test_ld[i].number.ld));
      break;
    }
  }
  if (i < 4 + 2 + 2 + 2 + 4)
  {
    test_passed = false;
    printf("isless(x, y) failed for SNaN and/or QNaN.\n");
  }
  else if (test_passed)
    printf("isless(x, y) passed all tests.\n");
  printf("----------------------------------------------------------------------\n");



  /*
   *  islessequal must return zero if x or y is [SQ]NaN (positive or negative)
   *  and else the result of x <= y.
   */
  test_passed = true;
  for (i = 0; i < 4; i += 3)
  {
    if (!islessequal(test_ld[i].number.ld, test_d[i].number.d))
    {
      printf("Test failed.  Testing islessequal(%La, %a) for a %s %s and %s %s: %d\n",   test_ld[i].number.ld, test_d[i].number.d,   test_ld[i].class, test_ld[i].type, test_d[i].class,  test_d[i].type,  islessequal(test_ld[i].number.ld, test_d[i].number.d));
      break;
    }
    else if (!islessequal(test_d[i].number.d, test_f[i].number.f))
    {
      printf("Test failed.  Testing islessequal(%a, %a) for a %s %s and %s %s: %d\n",    test_d[i].number.d,   test_f[i].number.f,   test_d[i].class,  test_d[i].type,  test_f[i].class,  test_f[i].type,  islessequal(test_d[i].number.d, test_f[i].number.f));
      break;
    }
    else if (islessequal(test_f[i].number.f, test_ld[i].number.ld))
    {
      printf("Test failed.  Testing islessequal(%a, %La) for a %s %s and %s %s: %d\n\n", test_f[i].number.f,   test_ld[i].number.ld, test_f[i].class,  test_f[i].type,  test_ld[i].class, test_ld[i].type, islessequal(test_f[i].number.f, test_ld[i].number.ld));
      break;
    }
  }
  if (i < 4)
  {
    test_passed = false;
    printf("islessequal(x, y) failed for normals.\n");
  }

  for (i = 1; i < 4 - 1; i++)
  {
    if (islessequal(test_ld[i].number.ld, test_d[i].number.d))
    {
      printf("Test failed.  Testing islessequal(%La, %a) for a %s %s and %s %s: %d\n",   test_ld[i].number.ld, test_d[i].number.d,   test_ld[i].class, test_ld[i].type, test_d[i].class,  test_d[i].type,  islessequal(test_ld[i].number.ld, test_d[i].number.d));
      break;
    }
    else if (islessequal(test_d[i].number.d, test_f[i].number.f))
    {
      printf("Test failed.  Testing islessequal(%a, %a) for a %s %s and %s %s: %d\n",    test_d[i].number.d,   test_f[i].number.f,   test_d[i].class,  test_d[i].type,  test_f[i].class,  test_f[i].type,  islessequal(test_d[i].number.d, test_f[i].number.f));
      break;
    }
    else if (!islessequal(test_f[i].number.f, test_ld[i].number.ld))
    {
      printf("Test failed.  Testing islessequal(%a, %La) for a %s %s and %s %s: %d\n\n", test_f[i].number.f,   test_ld[i].number.ld, test_f[i].class,  test_f[i].type,  test_ld[i].class, test_ld[i].type, islessequal(test_f[i].number.f, test_ld[i].number.ld));
      break;
    }
  }
  if (i < 4 - 1)
  {
    test_passed = false;
    printf("islessequal(x, y) failed for normals.\n");
  }

  for (i = 4; i < 4 + 1; i++)
  {
    if (!islessequal(test_ld[i].number.ld, test_d[i].number.d))
    {
      printf("Test failed.  Testing islessequal(%La, %a) for a %s %s and %s %s: %d\n",   test_ld[i].number.ld, test_d[i].number.d,   test_ld[i].class, test_ld[i].type, test_d[i].class,  test_d[i].type,  islessequal(test_ld[i].number.ld, test_d[i].number.d));
      break;
    }
    else if (!islessequal(test_d[i].number.d, test_f[i].number.f))
    {
      printf("Test failed.  Testing islessequal(%a, %a) for a %s %s and %s %s: %d\n",    test_d[i].number.d,   test_f[i].number.f,   test_d[i].class,  test_d[i].type,  test_f[i].class,  test_f[i].type,  islessequal(test_d[i].number.d, test_f[i].number.f));
      break;
    }
    else if (islessequal(test_f[i].number.f, test_ld[i].number.ld))
    {
      printf("Test failed.  Testing islessequal(%a, %La) for a %s %s and %s %s: %d\n\n", test_f[i].number.f,   test_ld[i].number.ld, test_f[i].class,  test_f[i].type,  test_ld[i].class, test_ld[i].type, islessequal(test_f[i].number.f, test_ld[i].number.ld));
      break;
    }
  }
  if (i < 4 + 1)
  {
    test_passed = false;
    printf("islessequal(x, y) failed for subnormals.\n");
  }

  for (i = 4 + 1; i < 4 + 2; i++)
  {
    if (islessequal(test_ld[i].number.ld, test_d[i].number.d))
    {
      printf("Test failed.  Testing islessequal(%La, %a) for a %s %s and %s %s: %d\n",   test_ld[i].number.ld, test_d[i].number.d,   test_ld[i].class, test_ld[i].type, test_d[i].class,  test_d[i].type,  islessequal(test_ld[i].number.ld, test_d[i].number.d));
      break;
    }
    else if (islessequal(test_d[i].number.d, test_f[i].number.f))
    {
      printf("Test failed.  Testing islessequal(%a, %a) for a %s %s and %s %s: %d\n",    test_d[i].number.d,   test_f[i].number.f,   test_d[i].class,  test_d[i].type,  test_f[i].class,  test_f[i].type,  islessequal(test_d[i].number.d, test_f[i].number.f));
      break;
    }
    else if (!islessequal(test_f[i].number.f, test_ld[i].number.ld))
    {
      printf("Test failed.  Testing islessequal(%a, %La) for a %s %s and %s %s: %d\n\n", test_f[i].number.f,   test_ld[i].number.ld, test_f[i].class,  test_f[i].type,  test_ld[i].class, test_ld[i].type, islessequal(test_f[i].number.f, test_ld[i].number.ld));
      break;
    }
  }
  if (i < 4 + 2)
  {
    test_passed = false;
    printf("islessequal(x, y) failed for subnormals.\n");
  }

  for (i = 4 + 2; i < 4 + 2 + 2; i++)
  {
    if (!islessequal(test_ld[i].number.ld, test_d[i].number.d))
    {
      printf("Test failed.  Testing islessequal(%La, %a) for a %s %s and %s %s: %d\n",   test_ld[i].number.ld, test_d[i].number.d,   test_ld[i].class, test_ld[i].type, test_d[i].class,  test_d[i].type,  islessequal(test_ld[i].number.ld, test_d[i].number.d));
      break;
    }
    else if (!islessequal(test_d[i].number.d, test_f[i].number.f))
    {
      printf("Test failed.  Testing islessequal(%a, %a) for a %s %s and %s %s: %d\n",    test_d[i].number.d,   test_f[i].number.f,   test_d[i].class,  test_d[i].type,  test_f[i].class,  test_f[i].type,  islessequal(test_d[i].number.d, test_f[i].number.f));
      break;
    }
    else if (!islessequal(test_f[i].number.f, test_ld[i].number.ld))
    {
      printf("Test failed.  Testing islessequal(%a, %La) for a %s %s and %s %s: %d\n\n", test_f[i].number.f,   test_ld[i].number.ld, test_f[i].class,  test_f[i].type,  test_ld[i].class, test_ld[i].type, islessequal(test_f[i].number.f, test_ld[i].number.ld));
      break;
    }
  }
  if (i < 4 + 2 + 2)
  {
    test_passed = false;
    printf("islessequal(x, y) failed for zero.\n");
  }

  for (i = 4 + 2 + 2; i < 4 + 2 + 2 + 2; i++)
  {
    if (!islessequal(test_ld[i].number.ld, test_d[i].number.d))
    {
      printf("Test failed.  Testing islessequal(%La, %a) for a %s %s and %s %s: %d\n",   test_ld[i].number.ld, test_d[i].number.d,   test_ld[i].class, test_ld[i].type, test_d[i].class,  test_d[i].type,  islessequal(test_ld[i].number.ld, test_d[i].number.d));
      break;
    }
    else if (!islessequal(test_d[i].number.d, test_f[i].number.f))
    {
      printf("Test failed.  Testing islessequal(%a, %a) for a %s %s and %s %s: %d\n",    test_d[i].number.d,   test_f[i].number.f,   test_d[i].class,  test_d[i].type,  test_f[i].class,  test_f[i].type,  islessequal(test_d[i].number.d, test_f[i].number.f));
      break;
    }
    else if (!islessequal(test_f[i].number.f, test_ld[i].number.ld))
    {
      printf("Test failed.  Testing islessequal(%a, %La) for a %s %s and %s %s: %d\n\n", test_f[i].number.f,   test_ld[i].number.ld, test_f[i].class,  test_f[i].type,  test_ld[i].class, test_ld[i].type, islessequal(test_f[i].number.f, test_ld[i].number.ld));
      break;
    }
  }
  if (i < 4 + 2 + 2 + 2)
  {
    test_passed = false;
    printf("islessequal(x, y) failed for infinity.\n");
  }

  for (i = 4 + 2 + 2 + 2; i < 4 + 2 + 2 + 2 + 4; i++)
  {
    if (islessequal(test_ld[i].number.ld, test_d[i].number.d))
    {
      printf("Test failed.  Testing islessequal(%La, %a) for a %s %s and %s %s: %d\n",   test_ld[i].number.ld, test_d[i].number.d,   test_ld[i].class, test_ld[i].type, test_d[i].class,  test_d[i].type,  islessequal(test_ld[i].number.ld, test_d[i].number.d));
      break;
    }
    else if (islessequal(test_d[i].number.d, test_f[i].number.f))
    {
      printf("Test failed.  Testing islessequal(%a, %a) for a %s %s and %s %s: %d\n",    test_d[i].number.d,   test_f[i].number.f,   test_d[i].class,  test_d[i].type,  test_f[i].class,  test_f[i].type,  islessequal(test_d[i].number.d, test_f[i].number.f));
      break;
    }
    else if (islessequal(test_f[i].number.f, test_ld[i].number.ld))
    {
      printf("Test failed.  Testing islessequal(%a, %La) for a %s %s and %s %s: %d\n\n", test_f[i].number.f,   test_ld[i].number.ld, test_f[i].class,  test_f[i].type,  test_ld[i].class, test_ld[i].type, islessequal(test_f[i].number.f, test_ld[i].number.ld));
      break;
    }
  }
  if (i < 4 + 2 + 2 + 2 + 4)
  {
    test_passed = false;
    printf("islessequal(x, y) failed for SNaN and/or QNaN.\n");
  }
  else if (test_passed)
    printf("islessequal(x, y) passed all tests.\n");
  printf("----------------------------------------------------------------------\n");



  /*
   *  islessgreater must return zero if x or y is [SQ]NaN (positive or negative)
   *  and else the result of (x < y) || x > y).
   */
  test_passed = true;
  for (i = 0; i < 4; i++)
  {
    if (!islessgreater(test_ld[i].number.ld, test_d[i].number.d))
    {
      printf("Test failed.  Testing islessgreater(%La, %a) for a %s %s and %s %s: %d\n",   test_ld[i].number.ld, test_d[i].number.d,   test_ld[i].class, test_ld[i].type, test_d[i].class,  test_d[i].type,  islessgreater(test_ld[i].number.ld, test_d[i].number.d));
      break;
    }
    else if (!islessgreater(test_d[i].number.d, test_f[i].number.f))
    {
      printf("Test failed.  Testing islessgreater(%a, %a) for a %s %s and %s %s: %d\n",    test_d[i].number.d,   test_f[i].number.f,   test_d[i].class,  test_d[i].type,  test_f[i].class,  test_f[i].type,  islessgreater(test_d[i].number.d, test_f[i].number.f));
      break;
    }
    else if (!islessgreater(test_f[i].number.f, test_ld[i].number.ld))
    {
      printf("Test failed.  Testing islessgreater(%a, %La) for a %s %s and %s %s: %d\n\n", test_f[i].number.f,   test_ld[i].number.ld, test_f[i].class,  test_f[i].type,  test_ld[i].class, test_ld[i].type, islessgreater(test_f[i].number.f, test_ld[i].number.ld));
      break;
    }
  }
  if (i < 4)
  {
    test_passed = false;
    printf("islessgreater(x, y) failed for normals.\n");
  }

  for (i = 4; i < 4 + 2; i++)
  {
    if (!islessgreater(test_ld[i].number.ld, test_d[i].number.d))
    {
      printf("Test failed.  Testing islessgreater(%La, %a) for a %s %s and %s %s: %d\n",   test_ld[i].number.ld, test_d[i].number.d,   test_ld[i].class, test_ld[i].type, test_d[i].class,  test_d[i].type,  islessgreater(test_ld[i].number.ld, test_d[i].number.d));
      break;
    }
    else if (!islessgreater(test_d[i].number.d, test_f[i].number.f))
    {
      printf("Test failed.  Testing islessgreater(%a, %a) for a %s %s and %s %s: %d\n",    test_d[i].number.d,   test_f[i].number.f,   test_d[i].class,  test_d[i].type,  test_f[i].class,  test_f[i].type,  islessgreater(test_d[i].number.d, test_f[i].number.f));
      break;
    }
    else if (!islessgreater(test_f[i].number.f, test_ld[i].number.ld))
    {
      printf("Test failed.  Testing islessgreater(%a, %La) for a %s %s and %s %s: %d\n\n", test_f[i].number.f,   test_ld[i].number.ld, test_f[i].class,  test_f[i].type,  test_ld[i].class, test_ld[i].type, islessgreater(test_f[i].number.f, test_ld[i].number.ld));
      break;
    }
  }
  if (i < 4 + 2)
  {
    test_passed = false;
    printf("islessgreater(x, y) failed for subnormals.\n");
  }

  for (i = 4 + 2; i < 4 + 2 + 2; i++)
  {
    if (islessgreater(test_ld[i].number.ld, test_d[i].number.d))
    {
      printf("Test failed.  Testing islessgreater(%La, %a) for a %s %s and %s %s: %d\n",   test_ld[i].number.ld, test_d[i].number.d,   test_ld[i].class, test_ld[i].type, test_d[i].class,  test_d[i].type,  islessgreater(test_ld[i].number.ld, test_d[i].number.d));
      break;
    }
    else if (islessgreater(test_d[i].number.d, test_f[i].number.f))
    {
      printf("Test failed.  Testing islessgreater(%a, %a) for a %s %s and %s %s: %d\n",    test_d[i].number.d,   test_f[i].number.f,   test_d[i].class,  test_d[i].type,  test_f[i].class,  test_f[i].type,  islessgreater(test_d[i].number.d, test_f[i].number.f));
      break;
    }
    else if (islessgreater(test_f[i].number.f, test_ld[i].number.ld))
    {
      printf("Test failed.  Testing islessgreater(%a, %La) for a %s %s and %s %s: %d\n\n", test_f[i].number.f,   test_ld[i].number.ld, test_f[i].class,  test_f[i].type,  test_ld[i].class, test_ld[i].type, islessgreater(test_f[i].number.f, test_ld[i].number.ld));
      break;
    }
  }
  if (i < 4 + 2 + 2)
  {
    test_passed = false;
    printf("islessgreater(x, y) failed for zero.\n");
  }

  for (i = 4 + 2 + 2; i < 4 + 2 + 2 + 2; i++)
  {
    if (islessgreater(test_ld[i].number.ld, test_d[i].number.d))
    {
      printf("Test failed.  Testing islessgreater(%La, %a) for a %s %s and %s %s: %d\n",   test_ld[i].number.ld, test_d[i].number.d,   test_ld[i].class, test_ld[i].type, test_d[i].class,  test_d[i].type,  islessgreater(test_ld[i].number.ld, test_d[i].number.d));
      break;
    }
    else if (islessgreater(test_d[i].number.d, test_f[i].number.f))
    {
      printf("Test failed.  Testing islessgreater(%a, %a) for a %s %s and %s %s: %d\n",    test_d[i].number.d,   test_f[i].number.f,   test_d[i].class,  test_d[i].type,  test_f[i].class,  test_f[i].type,  islessgreater(test_d[i].number.d, test_f[i].number.f));
      break;
    }
    else if (islessgreater(test_f[i].number.f, test_ld[i].number.ld))
    {
      printf("Test failed.  Testing islessgreater(%a, %La) for a %s %s and %s %s: %d\n\n", test_f[i].number.f,   test_ld[i].number.ld, test_f[i].class,  test_f[i].type,  test_ld[i].class, test_ld[i].type, islessgreater(test_f[i].number.f, test_ld[i].number.ld));
      break;
    }
  }
  if (i < 4 + 2 + 2 + 2)
  {
    test_passed = false;
    printf("islessgreater(x, y) failed for infinity.\n");
  }

  for (i = 4 + 2 + 2 + 2; i < 4 + 2 + 2 + 2 + 4; i++)
  {
    if (islessgreater(test_ld[i].number.ld, test_d[i].number.d))
    {
      printf("Test failed.  Testing islessgreater(%La, %a) for a %s %s and %s %s: %d\n",   test_ld[i].number.ld, test_d[i].number.d,   test_ld[i].class, test_ld[i].type, test_d[i].class,  test_d[i].type,  islessgreater(test_ld[i].number.ld, test_d[i].number.d));
      break;
    }
    else if (islessgreater(test_d[i].number.d, test_f[i].number.f))
    {
      printf("Test failed.  Testing islessgreater(%a, %a) for a %s %s and %s %s: %d\n",    test_d[i].number.d,   test_f[i].number.f,   test_d[i].class,  test_d[i].type,  test_f[i].class,  test_f[i].type,  islessgreater(test_d[i].number.d, test_f[i].number.f));
      break;
    }
    else if (islessgreater(test_f[i].number.f, test_ld[i].number.ld))
    {
      printf("Test failed.  Testing islessgreater(%a, %La) for a %s %s and %s %s: %d\n\n", test_f[i].number.f,   test_ld[i].number.ld, test_f[i].class,  test_f[i].type,  test_ld[i].class, test_ld[i].type, islessgreater(test_f[i].number.f, test_ld[i].number.ld));
      break;
    }
  }
  if (i < 4 + 2 + 2 + 2 + 4)
  {
    test_passed = false;
    printf("islessgreater(x, y) failed for SNaN and/or QNaN.\n");
  }
  else if (test_passed)
    printf("islessgreater(x, y) passed all tests.\n");
  printf("----------------------------------------------------------------------\n");



  /*
   *  isunordered must return non-zero if x or y is [SQ]NaN (positive or negative)
   *  and zero else.
   */
  test_passed = true;
  for (i = 0; i < 4; i++)
  {
    if (isunordered(test_ld[i].number.ld, test_d[i].number.d))
    {
      printf("Test failed.  Testing isunordered(%La, %a) for a %s %s and %s %s: %d\n",   test_ld[i].number.ld, test_d[i].number.d,   test_ld[i].class, test_ld[i].type, test_d[i].class,  test_d[i].type,  isunordered(test_ld[i].number.ld, test_d[i].number.d));
      break;
    }
    else if (isunordered(test_d[i].number.d, test_f[i].number.f))
    {
      printf("Test failed.  Testing isunordered(%a, %a) for a %s %s and %s %s: %d\n",    test_d[i].number.d,   test_f[i].number.f,   test_d[i].class,  test_d[i].type,  test_f[i].class,  test_f[i].type,  isunordered(test_d[i].number.d, test_f[i].number.f));
      break;
    }
    else if (isunordered(test_f[i].number.f, test_ld[i].number.ld))
    {
      printf("Test failed.  Testing isunordered(%a, %La) for a %s %s and %s %s: %d\n\n", test_f[i].number.f,   test_ld[i].number.ld, test_f[i].class,  test_f[i].type,  test_ld[i].class, test_ld[i].type, isunordered(test_f[i].number.f, test_ld[i].number.ld));
      break;
    }
  }
  if (i < 4)
  {
    test_passed = false;
    printf("isunordered(x) failed for normals.\n");
  }

  for (i = 4; i < 4 + 2; i++)
  {
    if (isunordered(test_ld[i].number.ld, test_d[i].number.d))
    {
      printf("Test failed.  Testing isunordered(%La, %a) for a %s %s and %s %s: %d\n",   test_ld[i].number.ld, test_d[i].number.d,   test_ld[i].class, test_ld[i].type, test_d[i].class,  test_d[i].type,  isunordered(test_ld[i].number.ld, test_d[i].number.d));
      break;
    }
    else if (isunordered(test_d[i].number.d, test_f[i].number.f))
    {
      printf("Test failed.  Testing isunordered(%a, %a) for a %s %s and %s %s: %d\n",    test_d[i].number.d,   test_f[i].number.f,   test_d[i].class,  test_d[i].type,  test_f[i].class,  test_f[i].type,  isunordered(test_d[i].number.d, test_f[i].number.f));
      break;
    }
    else if (isunordered(test_f[i].number.f, test_ld[i].number.ld))
    {
      printf("Test failed.  Testing isunordered(%a, %La) for a %s %s and %s %s: %d\n\n", test_f[i].number.f,   test_ld[i].number.ld, test_f[i].class,  test_f[i].type,  test_ld[i].class, test_ld[i].type, isunordered(test_f[i].number.f, test_ld[i].number.ld));
      break;
    }
  }
  if (i < 4 + 2)
  {
    test_passed = false;
    printf("isunordered(x) failed for subnormals aka denormals.\n");
  }

  for (i = 4 + 2; i < 4 + 2 + 2; i++)
  {
    if (isunordered(test_ld[i].number.ld, test_d[i].number.d))
    {
      printf("Test failed.  Testing isunordered(%La, %a) for a %s %s and %s %s: %d\n",   test_ld[i].number.ld, test_d[i].number.d,   test_ld[i].class, test_ld[i].type, test_d[i].class,  test_d[i].type,  isunordered(test_ld[i].number.ld, test_d[i].number.d));
      break;
    }
    else if (isunordered(test_d[i].number.d, test_f[i].number.f))
    {
      printf("Test failed.  Testing isunordered(%a, %a) for a %s %s and %s %s: %d\n",    test_d[i].number.d,   test_f[i].number.f,   test_d[i].class,  test_d[i].type,  test_f[i].class,  test_f[i].type,  isunordered(test_d[i].number.d, test_f[i].number.f));
      break;
    }
    else if (isunordered(test_f[i].number.f, test_ld[i].number.ld))
    {
      printf("Test failed.  Testing isunordered(%a, %La) for a %s %s and %s %s: %d\n\n", test_f[i].number.f,   test_ld[i].number.ld, test_f[i].class,  test_f[i].type,  test_ld[i].class, test_ld[i].type, isunordered(test_f[i].number.f, test_ld[i].number.ld));
      break;
    }
  }
  if (i < 4 + 2 + 2)
  {
    test_passed = false;
    printf("isunordered(x) failed for zero.\n");
  }

  for (i = 4 + 2 + 2; i < 4 + 2 + 2 + 2; i++)
  {
    if (isunordered(test_ld[i].number.ld, test_d[i].number.d))
    {
      printf("Test failed.  Testing isunordered(%La, %a) for a %s %s and %s %s: %d\n",   test_ld[i].number.ld, test_d[i].number.d,   test_ld[i].class, test_ld[i].type, test_d[i].class,  test_d[i].type,  isunordered(test_ld[i].number.ld, test_d[i].number.d));
      break;
    }
    else if (isunordered(test_d[i].number.d, test_f[i].number.f))
    {
      printf("Test failed.  Testing isunordered(%a, %a) for a %s %s and %s %s: %d\n",    test_d[i].number.d,   test_f[i].number.f,   test_d[i].class,  test_d[i].type,  test_f[i].class,  test_f[i].type,  isunordered(test_d[i].number.d, test_f[i].number.f));
      break;
    }
    else if (isunordered(test_f[i].number.f, test_ld[i].number.ld))
    {
      printf("Test failed.  Testing isunordered(%a, %La) for a %s %s and %s %s: %d\n\n", test_f[i].number.f,   test_ld[i].number.ld, test_f[i].class,  test_f[i].type,  test_ld[i].class, test_ld[i].type, isunordered(test_f[i].number.f, test_ld[i].number.ld));
      break;
    }
  }
  if (i < 4 + 2 + 2 + 2)
  {
    test_passed = false;
    printf("isunordered(x) failed for infinity.\n");
  }

  for (i = 4 + 2 + 2 + 2; i < 4 + 2 + 2 + 2 + 4; i++)
  {
    if (!isunordered(test_ld[i].number.ld, test_d[i].number.d))
    {
      printf("Test failed.  Testing isunordered(%La, %a) for a %s %s and %s %s: %d\n",   test_ld[i].number.ld, test_d[i].number.d,   test_ld[i].class, test_ld[i].type, test_d[i].class,  test_d[i].type,  isunordered(test_ld[i].number.ld, test_d[i].number.d));
      break;
    }
    else if (!isunordered(test_d[i].number.d, test_f[i].number.f))
    {
      printf("Test failed.  Testing isunordered(%a, %a) for a %s %s and %s %s: %d\n",    test_d[i].number.d,   test_f[i].number.f,   test_d[i].class,  test_d[i].type,  test_f[i].class,  test_f[i].type,  isunordered(test_d[i].number.d, test_f[i].number.f));
      break;
    }
    else if (!isunordered(test_f[i].number.f, test_ld[i].number.ld))
    {
      printf("Test failed.  Testing isunordered(%a, %La) for a %s %s and %s %s: %d\n\n", test_f[i].number.f,   test_ld[i].number.ld, test_f[i].class,  test_f[i].type,  test_ld[i].class, test_ld[i].type, isunordered(test_f[i].number.f, test_ld[i].number.ld));
      break;
    }
  }
  if (i < 4 + 2 + 2 + 2 + 4)
  {
    test_passed = false;
    printf("isunordered(x, y) failed for SNaN and/or QNaN.\n");
  }
  else if (test_passed)
    printf("isunordered(x, y) passed all tests.\n");
  printf("----------------------------------------------------------------------\n");

  return 0;
}
