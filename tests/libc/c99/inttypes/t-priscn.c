#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEST_T(x) \
typedef struct { \
  const int ##x ##_t n; \
  int ##x ##_t d_res; \
  int ##x ##_t i_res; \
} test_int ##x ##_t

#define TEST_U_T(x) \
typedef struct { \
  const uint ##x ##_t n; \
  uint ##x ##_t o_res; \
  uint ##x ##_t u_res; \
  uint ##x ##_t x_res; \
  uint ##x ##_t X_res; \
} test_uint ##x ##_t

TEST_T(8);
TEST_T(16);
TEST_T(32);
TEST_T(64);

TEST_T(_fast8);
TEST_T(_fast16);
TEST_T(_fast32);
TEST_T(_fast64);

TEST_T(_least8);
TEST_T(_least16);
TEST_T(_least32);
TEST_T(_least64);

TEST_T(max);
TEST_T(ptr);

TEST_U_T(8);
TEST_U_T(16);
TEST_U_T(32);
TEST_U_T(64);

TEST_U_T(_fast8);
TEST_U_T(_fast16);
TEST_U_T(_fast32);
TEST_U_T(_fast64);

TEST_U_T(_least8);
TEST_U_T(_least16);
TEST_U_T(_least32);
TEST_U_T(_least64);

TEST_U_T(max);
TEST_U_T(ptr);

#undef TEST_T
#undef TEST_U_T

#define TEST(x, y, z) \
  { \
    char buf[128]; \
    test_int ##x ##_t t = { INT ##y ##_MAX, 0, 0 }; \
    int ret; \
\
    sprintf(buf, "%" PRId ##z, t.n); \
    ret = sscanf(buf, "%" SCNd ##z, &t.d_res); \
    assert(ret > 0); \
    assert(t.d_res == t.n); \
\
    sprintf(buf, "%" PRIi ##z, t.n); \
    ret = sscanf(buf, "%" SCNi##z, &t.i_res); \
    assert(ret > 0); \
    assert(t.i_res == t.n); \
  }

#define TEST_U(x, y, z) \
  { \
    char buf[128]; \
    test_uint ##x ##_t t = { UINT ##y ##_MAX, 0, 0 }; \
    int ret; \
\
    sprintf(buf, "%" PRIu ##z, t.n); \
    ret = sscanf(buf, "%" SCNu ##z, &t.u_res); \
    assert(ret > 0); \
    assert(t.u_res == t.n); \
\
    sprintf(buf, "%" PRIo ##z, t.n); \
    ret = sscanf(buf, "%" SCNo ##z, &t.o_res); \
    assert(ret > 0); \
    assert(t.o_res == t.n); \
\
    sprintf(buf, "%" PRIx ##z, t.n); \
    ret = sscanf(buf, "%" SCNx ##z, &t.x_res); \
    assert(ret > 0); \
    assert(t.x_res == t.n); \
\
    sprintf(buf, "%" PRIX ##z, t.n); \
    ret = sscanf(buf, "%" SCNx ##z, &t.X_res); \
    assert(ret > 0); \
    assert(t.X_res == t.n); \
  }

int
main (void)
{
  TEST(8,  8,  8);
  TEST(16, 16, 16);
  TEST(32, 32, 32);
  TEST(64, 64, 64);

  TEST(_fast8,  _FAST8,  FAST8);
  TEST(_fast16, _FAST16, FAST16);
  TEST(_fast32, _FAST32, FAST32);
  TEST(_fast64, _FAST64, FAST64);

  TEST(_least8,  _LEAST8,  LEAST8);
  TEST(_least16, _LEAST16, LEAST16);
  TEST(_least32, _LEAST32, LEAST32);
  TEST(_least64, _LEAST64, LEAST64);

  TEST(max, MAX, MAX);
  TEST(ptr, PTR, PTR);

  TEST_U(8,  8,  8);
  TEST_U(16, 16, 16);
  TEST_U(32, 32, 32);
  TEST_U(64, 64, 64);

  TEST_U(_fast8,  _FAST8,  FAST8);
  TEST_U(_fast16, _FAST16, FAST16);
  TEST_U(_fast32, _FAST32, FAST32);
  TEST_U(_fast64, _FAST64, FAST64);

  TEST_U(_least8,  _LEAST8,  LEAST8);
  TEST_U(_least16, _LEAST16, LEAST16);
  TEST_U(_least32, _LEAST32, LEAST32);
  TEST_U(_least64, _LEAST64, LEAST64);

  TEST_U(max, MAX, MAX);
  TEST_U(ptr, PTR, PTR);

  puts("PASS");
  return(EXIT_SUCCESS);
}
