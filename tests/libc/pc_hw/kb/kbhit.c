#include <stdio.h>
#include <pc.h>

int
main(void)
{
  printf("waiting for key\n");
  while (!kbhit())
    ;
  printf("got key, getting char\n");
  printf("char is 0x%04x\n", getkey());
  return 0;
}
