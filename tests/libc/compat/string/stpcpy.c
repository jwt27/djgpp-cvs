#include <stdio.h>
#include <string.h>

int
main(void)
{
  char d[100];
  char *c = stpcpy(d, "hello");
  printf("d=%s %p c=%s %p\n", d, d, c, c);
  strcpy(c, "there");
  printf("d=%s %p c=%s %p\n", d, d, c, c);
  return 0;
}
