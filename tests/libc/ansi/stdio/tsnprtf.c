/*
 * tsnprtf.c - Test for snprintf()
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

int
main (void)
{
  char BIG[] = "Hello this is a too big string for the buffer";
  char holder[24];
  size_t i, j;

  i = snprintf(holder, sizeof(holder), "%s\n", BIG);
  printf("%s\n", BIG);
  printf("%s\n", holder);
  /*
   * We are expecting :
   *   i == strlen(BIG) + 1
   * meaning the number that would have been written if the buffer was
   * large enough (see C9X).
   */
  if (i != strlen(BIG) + 1 /* nul */)
    {
      fprintf(stderr, "FAILED snprintf\n");
      fprintf(stderr,
	      "sizeof (%lu), snprintf(%zd), strlen(%lu)\n",
	      sizeof(holder), i, strlen(BIG)) ;
      exit(EXIT_FAILURE);
    }

  /*
   * We may have broken sscanf since it is also a string stream
   * Lets do a basic test.
   */
  {
    static char line[] = "25 December 2000\n";
    int day, year;
    char month[24];

    /* we are expecting to read 3 variables */
    if ((i = sscanf(line, "%d %s %d\n", &day, month, &year)) == 3)
      {
	i = snprintf(holder, sizeof(holder), "%d %s %d\n", day, month, year);
	printf(line);
	j = printf(holder);
	if (i != j)
	  {
	    fprintf(stderr, "FAILED snprintf\n");
	    fprintf(stderr, "snprintf (%zd) != printf (%zd)\n", i, j);
	    exit(EXIT_FAILURE);
	  }
      }
    else
      {
	printf("sscanf (%zd)\n", i);
	printf("FAILED sscanf\n");
	exit(EXIT_FAILURE);
      }
  }

  /* Test length estimation - once with buffer, once without. */
  *holder = '\0';
  i = snprintf(holder, 0, "%s", BIG);
  if ((i != strlen(BIG)) || (*holder != '\0') /* buffer touched */)
    {
      fprintf(stderr, "FAILED length estimation (with buffer)\n");
      exit(EXIT_FAILURE);
    }

  i = snprintf(NULL, 0, "%s", BIG);
  if (i != strlen(BIG))
    {
      fprintf(stderr, "FAILED length estimation (no buffer)\n");
      exit(EXIT_FAILURE);
    }

  /* Try writing to a 1 byte buffer */
  snprintf(holder, sizeof(holder), "%s", BIG);
  i = snprintf(holder, 1, "%s", BIG);

  if ((i < 0) || (*holder != '\0'))
    {
      fprintf(stderr, "FAILED termination only\n");
      exit(EXIT_FAILURE);
    }

  /* Test maximum buffer size */
  i = snprintf(holder, ((size_t) INT_MAX) + 1, "%s", BIG);

  if (i >= 0)
    {
      fprintf(stderr, "FAILED too large buffer\n");
      exit(EXIT_FAILURE);
    }

  /* Test padding a field to larger than buffer size. */
  {
    size_t s = sizeof(holder) * 16;

    i = snprintf(holder, sizeof(holder), "%*s", (int)s, BIG);

    if ((i != s) || ((strlen(holder) + 1) != sizeof(holder)))
      {
	fprintf(stderr,
		"FAILED with padding larger than buffer: %zd output, "
		"%lu written to buffer\n",
		i, strlen(holder));
	exit(EXIT_FAILURE);
      }
  }

  /* Test precision to larger than buffer size. */
  {
    size_t s = sizeof(holder) * 4;

    i = snprintf(holder, sizeof(holder), "%*.*e", (int)s, (int)s, 1e0);

    if ((i <= s) || ((strlen(holder) + 1) != sizeof(holder)))
      {
	fprintf(stderr,
		"FAILED with precision larger than buffer: %zd output, "
		"%lu written to buffer\n",
		i, strlen(holder));
	exit(EXIT_FAILURE);
      }
  }

  /* signal success */
  printf("SUCCESS\n");
  return(EXIT_SUCCESS);
}
