#include <stdio.h>
#include <string.h>

int
main(void)
{
  char l[100];
  int i;
  for (i=0; i<35; i++)
  {
    strcpy(l, "mkXXXXXX");
    mktemp(l);
    puts(l);
  }
  return 0;
}
