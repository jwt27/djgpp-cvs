#include <stdio.h>

/* This program expects LPT1: to be an HP/LJ4 */

int
main(void)
{
  FILE *lpt = fopen("lpt1", "wb");
  fprintf(lpt, "\033(10U\033&k2G");
  fprintf(lpt, "\033&p14X1\t2\r\n34\r\n56\03278XXXXXXXX");
  fprintf(lpt, "\012");
  fclose(lpt);
  return 0;
}
