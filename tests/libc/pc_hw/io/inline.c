#include <pc.h>

int x, x2, x3;
int y;

void
foo(void)
{
  x = inportb(y);
  x2 = inportw(y);
  x3 = inportl(y);
  outportb(y, x);
  outportw(y, x2);
  outportl(y, x3);
}

int
main(void)
{
  return 0;
}
