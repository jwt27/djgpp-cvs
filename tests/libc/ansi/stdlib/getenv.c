#include <stdio.h>
#include <stdlib.h>

extern char **environ;

int
main(int argc, char **argv)
{
  int i;
  if (argc > 1)
    printf("getenv `%s' = `%s'\n", argv[1], getenv(argv[1]));
  else
    for (i=0; environ[i]; i++)
      printf("%s\n", environ[i]);
  return 0;
}
