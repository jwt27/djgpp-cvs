#include <stdio.h>

int d1 = 0x12345678;
int d2 = 0x76543210;
char bss[10000];

extern int end;

int
main(void)
{
  char *c, *e;
  if (d1 != 0x12345678)
  {
    printf ("d1 not 0x12345678\n");
    return 1;
  }
  if (d2 != 0x76543210)
  {
    printf ("d1 not 0x12345678\n");
    return 1;
  }

  c = bss;
  e = c + sizeof(bss);
  printf("bss scan from %p to %p, %lu bytes\n", c, e, e-c);
  while (c < e)
  {
    if (*c)
    {
      printf("non-zero bss at %p\n", c);
      return 1;
    }
    c++;
  }
  return 0;
}
