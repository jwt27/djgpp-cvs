#include <sys/farptr.h>
int
main(void)
{
  while ((_farnspeekb(0x226)&0x80)!=0);
  return 0;
}
