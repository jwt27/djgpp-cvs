#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

static const char *testnum[] = {
  "0e20",
  "1e200",
  "0e2000000000",	/* suggested by Morten Welinder */
  "1e6000000000",	/* overflow */
  "1e400",		/* ditto */
  "1e-400",		/* underflow */
  0
};

int main (void)
{
  int i;

  errno = 0;

  for (i = 0; testnum[i]; i++)
  {
    printf ("%20s  ->  %-20.15g\n", testnum[i], strtod (testnum[i], (char **)0));
    if (errno)
    {
      perror ("strtod");
      errno = 0;
    }
  }

  return 0;
}
