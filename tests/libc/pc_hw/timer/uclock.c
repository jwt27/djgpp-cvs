#include <stdio.h>
#include <time.h>
#include <pc.h>

int
main(void)
{
  uclock_t dt = 0, ot=0;
  while (!kbhit())
  {
    uclock_t t = uclock();
    printf("uclock -> %08x %08x   ", (unsigned)(t>>32), (unsigned int)t);

    dt = t - ot;
    ot = t;
    printf(" %08x %08x  ", (unsigned)(dt>>32), (unsigned int)dt);
    dt = -dt;
    printf(" %08x %08x\n", (unsigned)(dt>>32), (unsigned int)dt);

    fflush(stdout);
    if (t & 0x8000000000000000ULL)
      return 0;
  }
  getkey();
  return 0;
}
