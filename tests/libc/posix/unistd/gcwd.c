#include <unistd.h>
#include <stdio.h>

int
main(void)
{
  char buf[100], *bp;
  fprintf(stderr, "hello, there\n");
  fflush(stderr);
  if ((bp = getcwd(buf, 100)) == 0)
  {
    perror("getcwd");
    return 0;
  }
  fflush(stderr);
  printf("`%s'\n", bp);
  return 0;
}
