#include <stdio.h>

int ffs(int);

void d(int x)
{
  printf("ffs(0x%08x) = %d\n", x, ffs(x));
}

int
main(void)
{
  d(0x00000000);
  d(0x00000001);
  d(0x00000002);
  d(0x00000005);
  d(0x00010000);
  d(0x00010010);
  return 0;
}
