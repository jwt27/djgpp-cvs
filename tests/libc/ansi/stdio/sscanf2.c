/*
 * sscanf2.c
 * Test cases for conversion specifiers new to the ANSI C99 standard.
 */

#include <assert.h>
#include <inttypes.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  const char *str;
  const char *fmt;
  const signed char res;
} signed_char_testcase_t;

typedef struct {
  const char *str;
  const char *fmt;
  const unsigned char res;
} unsigned_char_testcase_t;

typedef struct {
  const char *str;
  const char *fmt;
  const intmax_t res;
} intmax_testcase_t;

typedef struct {
  const char *str;
  const char *fmt;
  const uintmax_t res;
} uintmax_testcase_t;

typedef struct {
  const char *str;
  const char *fmt;
  const ptrdiff_t res;
} ptrdiff_testcase_t;

typedef struct {
  const char *str;
  const char *fmt;
  const size_t res;
} size_testcase_t;

signed_char_testcase_t sc_testcases[] = {
  /* Normal */
  { "127",  "%hhu", 127 },
  { "127",  "%hhd", 127 },
  { "127",  "%hhi", 127 },
  { "0x7f", "%hhx", 127 },
  { "0x7F", "%hhX", 127 },
  { "0x7f", "%hhx", 127 },
  { "0x7F", "%hhX", 127 },
  { "0177", "%hho", 127 },

  /* Truncation */
  { "255",     "%hhd", -1 },
  { "256",     "%hhd",  0 },
  { "0xff",    "%hhx", -1 },
  { "0x100",   "%hhx",  0 },
  { "0377",    "%hho", -1 },
  { "0400",    "%hho",  0 },
  { "65535",   "%hhd", -1 },
  { "65536",   "%hhd",  0 },
  { "0xffff",  "%hhx", -1 },
  { "0x10000", "%hhx",  0 },
  { "177777",  "%hho", -1 },
  { "200000",  "%hho",  0 },

  /* Terminator */
  { NULL, NULL, 0 }
};

unsigned_char_testcase_t uc_testcases[] = {
  /* Normal */
  { "255",  "%hhu", 255 },
  { "0xff", "%hhx", 255 },
  { "0xFF", "%hhX", 255 },
  { "377",  "%hho", 255 },

  /* Truncation */
  { "256",     "%hhu",   0 },
  { "65535",   "%hhu", 255 },
  { "65536",   "%hhu",   0 },
  { "0x100",   "%hhx",   0 },
  { "0xffff",  "%hhx", 255 },
  { "0x10000", "%hhx",   0 },
  { "0x100",   "%hhX",   0 },
  { "0xFFFF",  "%hhX", 255 },
  { "0x10000", "%hhX",   0 },
  { "400",     "%hho",   0 },
  { "177777",  "%hho", 255 },
  { "2000",    "%hho",   0 },

  /* Terminator */
  { NULL, NULL, 0 }
};

intmax_testcase_t im_testcases[] = {
  /* Normal */
  { "9223372036854775807",   "%ju", INTMAX_MAX },
  { "9223372036854775807",   "%jd", INTMAX_MAX },
  { "9223372036854775807",   "%ji", INTMAX_MAX },
  { "0x7fffffffffffffff",    "%jx", INTMAX_MAX },
  { "0X7FFFFFFFFFFFFFFF",    "%jX", INTMAX_MAX },
  { "777777777777777777777", "%jo", INTMAX_MAX },

  /* Truncation */
  { "18446744073709551615",   "%jd", -1 },
  { "18446744073709551615",   "%ji", -1 },
  { "0xffffffffffffffff",     "%jx", -1 },
  { "0XFFFFFFFFFFFFFFFF",     "%jX", -1 },
  { "1777777777777777777777", "%jo", -1 },

  /* Terminator */
  { NULL, NULL, 0 }
};

uintmax_testcase_t um_testcases[] = {
  /* Normal */
  { "18446744073709551615",   "%ju", UINTMAX_MAX },
  { "0xffffffffffffffff",     "%jx", UINTMAX_MAX },
  { "0XFFFFFFFFFFFFFFFF",     "%jX", UINTMAX_MAX },
  { "1777777777777777777777", "%jo", UINTMAX_MAX },

  /* Terminator */
  { NULL, NULL, 0 }
};

ptrdiff_testcase_t pd_testcases[] = {
  /* Normal */
  { "2147483647",  "%tu", INT_MAX },
  { "2147483647",  "%td", INT_MAX },
  { "2147483647",  "%ti", INT_MAX },
  { "0x7fffffff",  "%tx", INT_MAX },
  { "0X7FFFFFFF",  "%tx", INT_MAX },
  { "17777777777", "%to", INT_MAX },

  /* Truncation */
  { "4294967295",  "%td", -1 },
  { "4294967295",  "%ti", -1 },
  { "0xffffffff",  "%tx", -1 },
  { "0XFFFFFFFF",  "%tX", -1 },
  { "37777777777", "%to", -1 },

  /* Terminator */
  { NULL, NULL, 0 }
};

size_testcase_t sz_testcases[] = {
  { "2147483647",  "%zu", SIZE_MAX },
  { "2147483647",  "%zd", SIZE_MAX },
  { "2147483647",  "%zi", SIZE_MAX },
  { "0x7fffffff",  "%zx", SIZE_MAX },
  { "0X7FFFFFFF",  "%zx", SIZE_MAX },
  { "17777777777", "%zo", SIZE_MAX },

  /* Truncation */
  { "4294967295",  "%zd", -1 },
  { "4294967295",  "%zi", -1 },
  { "0xffffffff",  "%zx", -1 },
  { "0XFFFFFFFF",  "%zX", -1 },
  { "37777777777", "%zo", -1 },

  /* Terminator */
  { NULL, NULL, 0 }
};

static void
fail (const int testnum,
      const char *input,
      const char *fmt,
      const char *reason,
      const long long code)
{
  fprintf(stderr,
	  "FAIL: Test %d: %s %s: %s: %lld\n",
	  testnum, input, fmt, reason, code);
  exit(EXIT_FAILURE);
}

int
main (void)
{
  signed char   sc_res;
  unsigned char uc_res;
  intmax_t      im_res;
  uintmax_t     um_res;
  ptrdiff_t     pd_res;
  int           testnum = 0;
  int           ret;
  int           i;

  /* Signed char testcases */
  for (i = 0; sc_testcases[i].str != NULL; i++) {
    testnum++;
    sc_res = 0;

    ret = sscanf(sc_testcases[i].str, sc_testcases[i].fmt, &sc_res);
    if ((ret == EOF) || (ret < 1)) {
      fail(testnum, sc_testcases[i].str, sc_testcases[i].fmt,
	   "sscanf failed", ret);
    }

    if (sc_testcases[i].res != sc_res) {
      fail(testnum, sc_testcases[i].str, sc_testcases[i].fmt,
	   "unexpected result", sc_res);
    }
  }

  /* Unsigned char testcases */
  for (i = 0; uc_testcases[i].str != NULL; i++) {
    testnum++;
    uc_res = 0;

    ret = sscanf(uc_testcases[i].str, uc_testcases[i].fmt, &uc_res);
    if ((ret == EOF) || (ret < 1)) {
      fail(testnum, uc_testcases[i].str, uc_testcases[i].fmt,
	   "sscanf failed", ret);
    }

    if (uc_testcases[i].res != uc_res) {
      fail(testnum, uc_testcases[i].str, uc_testcases[i].fmt,
	   "unexpected result", uc_res);
    }
  }

  /* intmax_t testcases */
  for (i = 0; im_testcases[i].str != NULL; i++) {
    testnum++;
    im_res = 0;

    ret = sscanf(im_testcases[i].str, im_testcases[i].fmt, &im_res);
    if ((ret == EOF) || (ret < 1)) {
      fail(testnum, im_testcases[i].str, im_testcases[i].fmt,
	   "sscanf failed", ret);
    }

    if (im_testcases[i].res != im_res) {
      fail(testnum, im_testcases[i].str, im_testcases[i].fmt,
	   "unexpected result", im_res);
    }
  }

  /* uintmax_t testcases */
  for (i = 0; um_testcases[i].str != NULL; i++) {
    testnum++;
    um_res = 0;

    ret = sscanf(um_testcases[i].str, um_testcases[i].fmt, &um_res);
    if ((ret == EOF) || (ret < 1)) {
      fail(testnum, um_testcases[i].str, um_testcases[i].fmt,
	   "sscanf failed", ret);
    }

    if (um_testcases[i].res != um_res) {
      fail(testnum, um_testcases[i].str, um_testcases[i].fmt,
	   "unexpected result", um_res);
    }
  }

  /* ptrdiff_t testcases */
  for (i = 0; pd_testcases[i].str != NULL; i++) {
    testnum++;
    pd_res = 0;

    ret = sscanf(pd_testcases[i].str, pd_testcases[i].fmt, &pd_res);
    if ((ret == EOF) || (ret < 1)) {
      fail(testnum, pd_testcases[i].str, pd_testcases[i].fmt,
	   "sscanf failed", ret);
    }

    if (pd_testcases[i].res != pd_res) {
      fail(testnum, pd_testcases[i].str, pd_testcases[i].fmt,
	   "unexpected result", pd_res);
    }
  }

  puts("PASS");
  return(EXIT_SUCCESS);
}
