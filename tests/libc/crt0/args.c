#include <stdio.h>
#include <crt0.h>

char **
__crt0_glob_function(char *a)
{
  return 0;
}

int
main(int argc, char **argv)
{
  int i;
  for (i=0; i<argc; i++)
    printf("[%d] = `%s'\n", i, argv[i]);
  return 0;
}
