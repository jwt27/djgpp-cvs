#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>

int
main(void)
{
  int r = open("nul", O_RDONLY);
  printf("first file descriptor is %d\n", r);
  return 0;
}
