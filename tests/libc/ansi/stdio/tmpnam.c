#include <stdio.h>

int
main(void)
{
  int i;
  char buf[1000];
  for (i=0; i<1030; i++)
  {
    char *s = tmpnam(buf);
    if (i<30 || i > 970)
      printf("%d: %s\n", i, s);
  }
  return 0;
}
