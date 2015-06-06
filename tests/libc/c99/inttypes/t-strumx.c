#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int
main (void)
{
  const uintmax_t i = UINTMAX_MAX;
  uintmax_t       res;
  char            buf[64];
  char           *p = NULL;

  sprintf(buf, "%" PRIuMAX, i);
  res = strtoumax(buf, &p, 0);
  assert(res == i);

  sprintf(buf, "%" PRIuMAX, i);
  res = strtoumax(buf, &p, 10);
  assert(res == i);

  sprintf(buf, "%" PRIoMAX, i);
  res = strtoumax(buf, &p, 0);
  assert(res == i);

  sprintf(buf, "0x%" PRIxMAX, i);
  res = strtoumax(buf, &p, 0);
  assert(res == i);

  sprintf(buf, "0x%" PRIXMAX, i);
  res = strtoumax(buf, &p, 0);
  assert(res == i);

  puts("PASS");
  return(EXIT_SUCCESS);
}
