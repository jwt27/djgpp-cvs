#include <stdio.h>
#include <signal.h>

void
sf(int x)
{
  printf("FPU exception trapped!\n");
  exit(1);
}

int
main(void)
{

  if (signal(SIGFPE, sf) == SIG_ERR)
    printf("cannot set signal handler\n");

  printf("before the raise\n");
  raise(SIGFPE);
  printf("after the raise\n");

  return 0;
}
