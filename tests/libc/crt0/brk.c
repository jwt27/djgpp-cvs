#include <stdio.h>
#include <unistd.h>

int
main(void)
{
  char *old = sbrk(0);
  int new = brk(old+16);
  printf("old was %p, new is %x, now at %p\n", old, new, sbrk(0));
  return 0;
}
