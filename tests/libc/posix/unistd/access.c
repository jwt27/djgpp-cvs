#include <unistd.h>
#include <stdio.h>
#include <string.h>

const char *
q(char *s, int a)
{
  if (access(s, a) == 0)
    return "YES";
  return "NO ";
}

int
main(int argc, char **argv)
{
  int i, max=0;
  for (i=1; i<argc; i++)
    if (max < strlen(argv[i]))
      max = strlen(argv[i])+1;
  for (i=1; i<argc; i++)
  {
    printf("access %s:", argv[i]);
    printf("%*c", (int)(max-strlen(argv[i])), ' ');
    printf("F_OK: %s  ", q(argv[i], F_OK));
    printf("R_OK: %s  ", q(argv[i], R_OK));
    printf("W_OK: %s  ", q(argv[i], W_OK));
    printf("X_OK: %s  ", q(argv[i], X_OK));
    printf("D_OK: %s  ", q(argv[i], D_OK));
    printf("\n");
  }
  return 0;
}
