#include <stdio.h>
#include <stdlib.h>
#include <math.h>

double x = 3.0;
double y = 7.0;

int
main(int argc, char **argv)
{
  int i = argc > 1 ? atoi(argv[1]) : 0;
  char buf[100];
  if (i == 4)
    return 0;
  printf("nest %d %g\n", i, x / y);
  sprintf(buf, "%s %d\n", argv[0], i+1);
  system(buf);
  return 0;
}
