#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

int
main(int argc, char **argv)
{
  int i;
  for (i=1; i<argc; i++)
  {
    char buf[1000];
    _fixpath(argv[i], buf);
    printf("`%s' -> `%s'\n", argv[i], buf);
  }
  return 0;
}
