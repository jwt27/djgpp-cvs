#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int ac, char *av[])
{
  double d;
  float f;
  long double ld;
  char buf[256];
  char *end;

  localeconv()->decimal_point[0] = '$';

  strcpy(buf, "12$26");
  d = strtod(buf, &end);
  if((d != 12.26) || (end != &buf[5]))
  {
    printf("strtod() failed: %f\n", d);
    return -1;
  }
  f = strtof(buf, &end);
  if((f != 12.26) || (end != &buf[5]))
  {
    printf("strtof() failed: %f\n", f);
    return -2;
  }
  ld = strtold(buf, &end);
  if((ld != 12.26L) || (end != &buf[5]))
  {
    printf("strtold() failed: %Lf\n", ld);
    return -3;
  }
  sprintf(buf, "%.2f", d);
  if(strcmp(buf, "12$26"))
  {
    printf("sprintf failed\n");
    return -4;
  }
  sscanf(buf, "%Lf", &ld);
  if(ld != 12.26L)
  {
    printf("sscanf failed: %Lf\n", ld);
    return -5;
  }

  printf("all tests passed\n");
  return 0;
}
