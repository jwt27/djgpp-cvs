#include <stdio.h>


union REGS {
  struct {
      unsigned long eax;
      unsigned long ax;
  } x;
};

int
main(void)
{
  union REGS r;
  printf("r.x.eax = %d\n", (int)&(r.x.eax) - (int)&(r));
  printf("r.x.ax = %d\n", (int)&(r.x.ax) -  (int)&(r));
  return 0;
}
