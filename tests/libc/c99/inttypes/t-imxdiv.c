#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

int
main (void)
{
  intmax_t  n, d;
  imaxdiv_t res;

  /* See <stdint.h>: INTMAX_MAX is an odd number. */

  n = INTMAX_MAX;
  d = 1;
  res = imaxdiv(n, d);
  assert(res.quot == n);
  assert(res.rem  == 0);

  n = INTMAX_MAX;
  d = 2;
  res = imaxdiv(n, d);
  assert(res.quot == (n / 2));
  assert(res.rem  == 1);

  puts("PASS");
  return(EXIT_SUCCESS);
}
