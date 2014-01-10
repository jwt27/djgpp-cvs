/*
 * printf6.c
 * Test cases for field width specifier for different floating point conversions.
 */

#include <stdio.h>
#include <string.h>

#define SIZE  128


int
main(void)
{
  const char *should_be[] = {
    /* Created with linux libc.  */
    "0xe.10624dd2f1a9fbe000000000000000000000000000000000000000000000p-3",
    "1.757999999999999999949693019196672594262054190039634704589844e+00",
    "1.757999999999999999949693019196672594262054190039634704589844",
    "1.757999999999999999949693019196672594262054190039634704589844",
    "0xe.000000000000000000000000000000000000000000000000000000000000p-3",
    "1.750000000000000000000000000000000000000000000000000000000000e+00",
    "1.750000000000000000000000000000000000000000000000000000000000",
    "1.75"
  };
  char result[SIZE];
  int failures = 0;


  snprintf(result, SIZE, "%.60La", 1.758L);
  if (strcmp(result, should_be[0]))
  {
    printf("Check 0  \"%%.60La\":\nresult = '%s'\nshould be = '%s'\n", result, should_be[0]);
    failures++;
  }

  snprintf(result, SIZE, "%.60Le", 1.758L);
  if (strcmp(result, should_be[1]))
  {
    printf("Check 1  \"%%.60Le\":\nresult = '%s'\nshould be = '%s'\n", result, should_be[1]);
    failures++;
  }

  snprintf(result, SIZE, "%.60Lf", 1.758L);
  if (strcmp(result, should_be[2]))
  {
    printf("Check 2  \"%%.60Lf\":\nresult = '%s'\nshould be = '%s'\n", result, should_be[2]);
    failures++;
  }

  snprintf(result, SIZE, "%.60Lg", 1.758L);
  if (strcmp(result, should_be[3]))
  {
    printf("Check 3  \"%%.60Lg\":\nresult = '%s'\nshould be = '%s'\n", result, should_be[3]);
    failures++;
  }

  snprintf(result, SIZE, "%.60La", 1.75L);
  if (strcmp(result, should_be[4]))
  {
    printf("Check 4  \"%%.60La\":\nresult = '%s'\nshould be = '%s'\n", result, should_be[4]);
    failures++;
  }

  snprintf(result, SIZE, "%.60Le", 1.75L);
  if (strcmp(result, should_be[5]))
  {
    printf("Check 5  \"%%.60Le\":\nresult = '%s'\nshould be = '%s'\n", result, should_be[5]);
    failures++;
  }

  snprintf(result, SIZE, "%.60Lf", 1.75L);
  if (strcmp(result, should_be[6]))
  {
    printf("Check 6  \"%%.60Lf\":\nresult = '%s'\nshould be = '%s'\n", result, should_be[6]);
    failures++;
  }

  snprintf(result, SIZE, "%.60Lg", 1.75L);
  if (strcmp(result, should_be[7]))
  {
    printf("Check 7  \"%%.60Lg\":\nresult = '%s'\nshould be = '%s'\n", result, should_be[7]);
    failures++;
  }

  printf("%d checks failed.\n", failures);


  return failures;
}
