#include <stdio.h>
#include <unistd.h>

int
main(void)
{
  printf("hello");
  sleep(2);
  printf(" there\n");
  return 0;
}
