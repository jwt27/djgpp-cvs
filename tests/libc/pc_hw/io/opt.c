#include <pc.h>
int
main(void)
{
  while ((inportb(0x226)&0x80)!=0);
  return 0;
}
