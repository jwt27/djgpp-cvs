#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int
main (void)
{
  const intmax_t  i = INTMAX_MAX;
  const intmax_t  j = INTMAX_MIN;
  intmax_t        res;
  char            buf[64];
  char           *p = NULL;

  /* Positive number */
  sprintf(buf, "%" PRIdMAX, i);
  res = strtoimax(buf, &p, 0);
  assert(res == i);

  sprintf(buf, "%" PRIdMAX, i);
  res = strtoimax(buf, &p, 10);
  assert(res == i);

  sprintf(buf, "%" PRIiMAX, i);
  res = strtoimax(buf, &p, 0);
  assert(res == i);

  sprintf(buf, "%" PRIiMAX, i);
  res = strtoimax(buf, &p, 10);
  assert(res == i);

  /* Negative number */
  sprintf(buf, "%" PRIdMAX, j);
  res = strtoimax(buf, &p, 0);
  assert(res == j);

  sprintf(buf, "%" PRIdMAX, j);
  res = strtoimax(buf, &p, 10);
  assert(res == j);

  sprintf(buf, "%" PRIiMAX, j);
  res = strtoimax(buf, &p, 0);
  assert(res == j);

  sprintf(buf, "%" PRIiMAX, j);
  res = strtoimax(buf, &p, 10);
  assert(res == j);

  puts("PASS");
  return(EXIT_SUCCESS);
}
