#include <stdio.h>
#include <process.h>

void
p(int argc, char **argv)
{
  int i;
  for (i=1; i<argc; i++)
    printf(" %s", argv[i]);
  printf("\n");
}

int
main(int argc, char **argv)
{
  p(argc, argv);
  spawnvp(P_WAIT, argv[1], argv+1);
  p(argc, argv);
  return 0;
}
