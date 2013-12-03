#include <stdio.h>
#include <malloc.h>

int
main(void)
{
  size_t aligment = 0;
  size_t amount = 1024;
  char *p;
  int i, failed = 0;


  p =  memalign(aligment, amount);
  if (!p)
  {
    printf("memalign failed to align on byte boundary.\n");
    failed++;
  }

  p =  memalign(++aligment, amount);
  if ((size_t)p % aligment)
  {
    printf("memalign erroneously allowed to aligned on %zd byte boundary.\n", aligment);
    failed++;
  }

  for (++aligment, i = 0; i < 10; i++, aligment = 2 << i)
  {
    p =  memalign(aligment, amount);
    if (!p || (size_t)p % aligment)
    {
      printf("memalign failed to align on %zd byte boundary.\n", aligment);
      failed++;
    }

    p =  memalign(++aligment, amount);
    if (p)
    {
      printf("memalign erroneously allowed to aligned on %zd byte boundary.\n", aligment);
      failed++;
    }
  }

  if (failed)
    printf("memalign failed for %d checks.\n", failed);
  else
    printf("memalign check passed.\n");

  return 0;
}
