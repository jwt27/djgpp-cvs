#include <pc.h>

char str[] = "This is a test";

int
main(void)
{
  int i,j;
  ScreenClear();
  for (i=0; i<20; i++)
    for (j=0; j<(int)sizeof(str); j++)
      ScreenPutChar(str[j], i, i+j, i);
  return 0;
}
