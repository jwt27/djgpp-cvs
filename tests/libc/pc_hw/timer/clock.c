#include <stdio.h>
#include <time.h>
#include <pc.h>

int
main(void)
{
  clock_t dt = 0, ot=0;
  while (!kbhit())
  {
    clock_t t = clock();
    printf("clock -> %08x   ", t);

    dt = t - ot;
    ot = t;
    printf(" %08x  ", dt);
    dt = -dt;
    printf(" %08x\n", dt);

    fflush(stdout);
    if (t & 0x80000000UL)
      return 0;
  }
  getkey();
  return 0;
}
