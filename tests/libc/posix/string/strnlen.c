#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SOURCE  "A string to check."


int main(void)
{
  char *psrc = NULL, src[] = SOURCE;
  int rv = 0;


  if (strnlen(src, sizeof(SOURCE) + 1000) != sizeof(SOURCE) - 1)
    rv++;

  if (strnlen(src, sizeof(SOURCE)) != sizeof(SOURCE) - 1)
    rv++;

  if (strnlen(src, sizeof(SOURCE) - 1) != sizeof(SOURCE) - 1)
    rv++;

  if (strnlen(src, sizeof(SOURCE) - 2) != sizeof(SOURCE) - 2)
    rv++;

  if (strnlen(src, 0) != 0)
    rv++;

  if (strnlen(psrc, sizeof(SOURCE) + 1000) != 0)
    rv++;

  src[sizeof(SOURCE) - 1] = 'z';
  if (strnlen(src, sizeof(SOURCE) + 10) > sizeof(SOURCE) + 10)
    rv++;

  printf("%d checks of strnlen failed.\n", rv);
  return rv;
}
