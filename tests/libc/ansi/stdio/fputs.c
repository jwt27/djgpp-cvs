#include <stdio.h>

int
main(void)
{
  FILE *f = fopen("fprintf.out", "w");
  fputs("line1\nline2\ncr -->\r<--cr\n\n", f);
  fputs(":\n", f);
  fclose(f);
  return 0;
}
