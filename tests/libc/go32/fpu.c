#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <float.h>

void
sf(int x)
{
  printf("FPU exception trapped!\n");
  exit(1);
}

float x = 0.0;

int
main(void)
{
  double y;

  y = 3.2 + x;
  printf("before the trap, y=%g\n", y);

  _control87(0, 0x3f); /* enable exceptions */
  if (signal(SIGFPE, sf) == SIG_ERR)
    printf("cannot set signal handler\n");

  raise(SIGFPE);
  printf("after the raise\n");

  y = 1.0/x;
  printf("after the trap, y=%g\n", y);

  return 0;
}
