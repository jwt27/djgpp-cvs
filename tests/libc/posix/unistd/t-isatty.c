#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

int
main (void)
{
  /* Check the standard handles */
  assert(isatty(fileno(stdin)));
  assert(isatty(fileno(stdout)));
  assert(!isatty(fileno(stdprn)));

  /* Check an invalid file descriptor. */
  assert(!isatty(99));

  puts("PASS");
  return(EXIT_SUCCESS);
}
