#include <stdio.h>

int
main(void)
{
  FILE *f = fopen("fputc.out", "w");
  fputc('\n', f);
  fclose(f);
  return 0;
}
