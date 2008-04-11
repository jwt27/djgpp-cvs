#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

static const char *testnum[] = {
  "0e2000",
  "1e2000",
  "0e2000000000",	/* suggested by Morten Welinder */
  "1e6000000000",	/* overflow */
  "1e5000",		/* ditto */
  "1e-5000",		/* underflow */
  "InF",		/* infinity */
  "-inf",		/* infinity */
  "infinity",		/* infinity */
  "-inFinitY",		/* infinity */
  "nAn",		/* nan */
  "-nan",		/* nan */
  "Nan()",		/* nan */
  "nan(0)",		/* nan */
  "Nan(1)",		/* nan */
  "-NaN(0xffFFFFFfff)",	/* nan */
  0
};

int main(void)
{
  int i;

  errno = 0;

  for (i = 0; testnum[i]; i++)
  {
    printf ("%20s  ->  %-20.15Lg\n", testnum[i], strtold (testnum[i], (char **)0));
    if (errno)
    {
      perror ("strtold");
      errno = 0;
    }
  }

  return 0;
}

