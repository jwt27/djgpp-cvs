#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  const char *input;
  const char *fmt;

  const int expected;
  const char expected_c[2];
  const char expected_c2[2];

  char c[2];
  char c2[2];
} sscanf_testcase_t;

sscanf_testcase_t sscanf_testcases[] = {
  /* No assignment */
  { "",    "%*[0123456789]%*c",             EOF, "",  "" },
  { "X",   "%*[0123456789]%*c",             0,   "",  "" },
  { "1",   "%*[0123456789]%*c",             0,   "",  "" },
  { "1X2", "%*[0123456789]%*[0123456789]",  0,   "",  "" },
  { "1,2", "%*[0123456789],%*[0123456789]", 0,   "",  "" },

  /* Assign first */
  { "",    "%[0123456789]%*c",              EOF, "",  "" },
  { "X",   "%[0123456789]%*c",              0,   "",  "" },
  { "1",   "%[0123456789]%*c",              1,   "1", "" },
  { "1X2", "%[0123456789]%*[0123456789]",   1,   "1", "" },
  { "1,2", "%[0123456789],%*[0123456789]",  1,   "1", "" },

  /* Assign second */
  { "",    "%*[0123456789]%c",              EOF, "",  "" },
  { "X",   "%*[0123456789]%c",              0,   "",  "" },
  { "1",   "%*[0123456789]%c",              0,   "",  "" },
  { "1X2", "%*[0123456789]%[0123456789]",   0,   "",  "" },
  { "1,2", "%*[0123456789],%[0123456789]",  1,   "2", "" },

  /* Assign both */
  { "",    "%[0123456789]%c",               EOF, "",  "" },
  { "X",   "%[0123456789]%c",               0,   "",  "" },
  { "1",   "%[0123456789]%c",               1,   "1", "" },
  { "1X2", "%[0123456789]%[0123456789]",    1,   "1", "" },
  { "1,2", "%[0123456789],%[0123456789]",   2,   "1", "2" },

  /* Terminator */
  { NULL, NULL, 0 }
};

int
main (void)
{
  int ok = 1;
  int ret;
  int i;

  for (i = 0; sscanf_testcases[i].input != NULL; i++)
  {
    sscanf_testcases[i].c[0]
      = sscanf_testcases[i].c2[0]
      = 0;
    ret = sscanf(sscanf_testcases[i].input,
		 sscanf_testcases[i].fmt,
		 sscanf_testcases[i].c,
		 sscanf_testcases[i].c2);
    if (   (ret != sscanf_testcases[i].expected)
	|| (strcmp(sscanf_testcases[i].c, sscanf_testcases[i].expected_c))
	|| (strcmp(sscanf_testcases[i].c2, sscanf_testcases[i].expected_c2)))
    {
      printf("Test %d: FAIL: (\"%s\", \"%s\");\n"
	     "\texpected %d;\n"
	     "\texpected c1 '%s';\n"
	     "\texpected c2 '%s';\n"
	     "\tgot %d;\n"
	     "\tc == '%s'\n"
	     "\tc2 == '%s'\n",
	     i + 1,
	     sscanf_testcases[i].input,
	     sscanf_testcases[i].fmt,
	     sscanf_testcases[i].expected,
	     sscanf_testcases[i].expected_c,
	     sscanf_testcases[i].expected_c2,
	     ret,
	     sscanf_testcases[i].c,
	     sscanf_testcases[i].c2);
      ok = 0;
    }
  }

  puts(ok ? "PASS" : "FAIL");
  return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
