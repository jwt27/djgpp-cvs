#include <pc.h>

char str[] = "This is a test";

int
main(void)
{
  int i;
  ScreenClear();
  for (i=0; i<20; i++)
    ScreenPutString(str, i, i, i);
  return 0;
}
