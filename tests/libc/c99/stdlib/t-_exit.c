#include <stdio.h>
#include <stdlib.h>

void
oops (void)
{
  puts("Oops. This shouldn't be happening.");
}

int
main (void)
{
  atexit(oops);

  _Exit(EXIT_SUCCESS);

  /* Shouldn't be reached! */
  puts("Well, what's happened here, I wonder?");
  return(EXIT_FAILURE);
}
