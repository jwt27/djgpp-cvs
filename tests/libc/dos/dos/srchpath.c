#include <stdio.h>
#include <dir.h>

int
main(int argc, char **argv)
{
  int i;
  for (i=1; i<argc; i++)
    printf("%s -> %s\n", argv[i], searchpath(argv[i]));
  return 0;
}
