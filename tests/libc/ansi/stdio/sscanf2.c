#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  const char *str;
  const char *fmt;
  const signed char res;
} signed_testcase_t;

typedef struct {
  const char *str;
  const char *fmt;
  const unsigned char res;
} unsigned_testcase_t;

signed_testcase_t s_testcases[] = {
  /* Normal */
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

unsigned_testcase_t u_testcases[] = {
  /* Normal */
  { "255", "%hhu", 255 },

  /* Truncation */
  { "256",   "%hhu",   0 },
  { "65535", "%hhu", 255 },
  { "65536", "%hhu",   0 },

  /* Terminator */
  { NULL, NULL, 0 }
};

static void
fail (const int testnum,
      const char *input,
      const char *fmt,
      const char *reason,
      const long code)
{
  fprintf(stderr,
	  "FAIL: Test %d: %s %s: %s: %ld\n",
	  testnum, input, fmt, reason, code);
  exit(EXIT_FAILURE);
}

int
main (void)
{
  signed char   s_res;
  unsigned char u_res;
  int           testnum = 0;
  int           ret;
  int           i;

  /* Signed testcases */
  for (i = 0; s_testcases[i].str != NULL; i++) {
    testnum++;
    s_res = 0;

    ret = sscanf(s_testcases[i].str, s_testcases[i].fmt, &s_res);
    if ((ret == EOF) || (ret < 1)) {
      fail(testnum, s_testcases[i].str, s_testcases[i].fmt,
	   "sscanf failed", ret);
    }

    if (s_testcases[i].res != s_res) {
      fail(testnum, s_testcases[i].str, s_testcases[i].fmt,
	   "unexpected result", s_res);
    }
  }

  /* Unsigned testcases */
  for (i = 0; u_testcases[i].str != NULL; i++) {
    testnum++;
    u_res = 0;

    ret = sscanf(u_testcases[i].str, u_testcases[i].fmt, &u_res);
    if ((ret == EOF) || (ret < 1)) {
      fail(testnum, u_testcases[i].str, u_testcases[i].fmt,
	   "sscanf failed", ret);
    }

    if (u_testcases[i].res != u_res) {
      fail(testnum, u_testcases[i].str, u_testcases[i].fmt,
	   "unexpected result", u_res);
    }
  }

  puts("PASS");
  return(EXIT_SUCCESS);
}
