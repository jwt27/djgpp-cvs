#include <stdio.h>
#include <unistd.h>

int
main(int argc, char **argv)
{
  char d[4000];

  getcwd(d, 4000);
  printf("After:   %s\n", d);

  printf("Chdir:   %s\n", argv[1]);
  if (chdir(argv[1]))
    perror(argv[1]);

  getcwd(d, 4000);
  printf("Before:  %s\n", d);

  return 0;
}
