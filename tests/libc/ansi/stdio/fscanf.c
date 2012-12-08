#include <stdio.h>

int
main(void)
{
  FILE *f = fopen("fscanf.in", "r");
  char buf[1000];

  printf("buf = %p\n", buf);

  fscanf(f, "%s", buf);

  printf("buf = `%s'\n", buf);

  fclose(f);
  return 0;
}
