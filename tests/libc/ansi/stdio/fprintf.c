#include <stdio.h>

int
main(void)
{
  FILE *f = fopen("fprintf.out", "w");
  fprintf(f,"hello");
  fprintf(f," there\n");
  fprintf(f,"line1\nline2\ncr -->\r<--cr\n\n");
  fclose(f);
  return 0;
}
