#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

int
main (void)
{
  const intmax_t i = INTMAX_MAX;
  const intmax_t j = -INTMAX_MAX;

  assert(imaxabs(i) == i);
  assert(imaxabs(j) == -j);

  puts("PASS");
  return(EXIT_SUCCESS);
}
