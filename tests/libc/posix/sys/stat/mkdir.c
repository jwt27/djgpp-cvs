#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

int
main(int argc, char **argv)
{
  int i;

  for (i=1; i<argc; i++)
  {
    mkdir(argv[i], 0666);
    printf("mkdir(%s) = %s\n", argv[i], strerror(errno));
  }

  return 0;
}
