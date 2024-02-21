#include <float.h>
#include <stdio.h>

double x=0.0, y;

int
main(void)
{
  printf("fpu status = %x\n", _status87());

  y = 1/x;

  printf("fpu status = %x\n", _status87());

  _clear87();

  printf("fpu status = %x\n", _status87());

  return 0;
}
