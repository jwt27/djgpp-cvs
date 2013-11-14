#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SOURCE  "A string to duplicate."


int main(void)
{
  char *psrc, src[] = SOURCE;
  char *dst[10];
  int rv = 0;
  register int i;


  dst[0] = strndup(src, sizeof(SOURCE));
  if (!dst[0])
    rv++;
  else
  {
    for (i = 0; dst[0][i] == src[i]; i++)
      ;
    if (i != sizeof(SOURCE))
      rv++;
  }

  dst[1] = strndup(src, sizeof(SOURCE) + 1);
  if (!dst[1])
    rv++;
  else
  {
    for (i = 0; dst[1][i] == src[i]; i++)
      ;
    if (i != sizeof(SOURCE))
      rv++;
  }

  dst[2] = strndup(src, sizeof(SOURCE) - 1);
  if (!dst[2])
    rv++;
  else
  {
    for (i = 0; dst[2][i] == src[i]; i++)
      ;
    if (i != sizeof(SOURCE))
      rv++;
  }

  dst[3] = strndup(src, sizeof(SOURCE) - 2);
  if (!dst[3])
    rv++;
  else
  {
    for (i = 0; dst[3][i] == src[i]; i++)
      ;
    if (i != sizeof(SOURCE) - 2)
      rv++;
  }

  dst[4] = strndup(src, 0);
  if (!dst[4])
    rv++;
  else
  {
    for (i = 0; dst[4][i] == src[i]; i++)
      ;
    if (i != 0)
      rv++;
  }

  src[0] = '\0';
  dst[5] = strndup(src, 1000);
  if (!dst[5])
    rv++;
  else
  {
    for (i = 0; dst[5][i] == src[i]; i++)
      ;
    if (i != 1)
      rv++;
  }

  psrc = NULL;
  dst[6] = strndup(psrc, 1000);
  if (dst[6])
    rv++;

  for (i = 0; i < 7; i++)
    free(dst[i]);


  printf("%d checks of strndup failed.\n", rv);
  return rv;
}
