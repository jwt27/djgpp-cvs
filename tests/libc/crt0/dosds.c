#include <stdio.h>
#include <go32.h>
#include <sys/farptr.h>

int
main(void)
{
  int i;
  for (i=0xfff0; i<0x1000f; i++)
  {
    printf("%08x\n", i);
    fflush(stdout);
    _farpeekb(_dos_ds, i);
  }
  return 0;
}
