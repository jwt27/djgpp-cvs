#include <stdio.h>
#include <sys/stat.h>

int
main(int argc, char **argv)
{
  int i;
  for (i=1; i<argc; i++)
    printf("%s -> %s\n", argv[i], _truename(argv[i], 0));
  return 0;
}
