#include <stdio.h>
#include <unistd.h>

int
main(void)
{
  int i;
  for (i=0; i<10; i++)
    printf("%10d", getpid());
  printf("\n");
  return 0;
}
