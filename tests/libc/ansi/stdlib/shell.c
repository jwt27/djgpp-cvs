#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int
main(void)
{
  char line[2000];
  int rv;
  while (1)
  {
    printf("$ ");
    fflush(stdout);
    if (fgets(line, 2000, stdin) == 0)
      break;
    line[strlen(line)-1] = 0;
    printf("line = `%s'\n", line);
    rv = system(line);
/*    rv = spawnlp(P_WAIT, line, line, 0); */
    printf("ออ%c %d\n", 16, rv);
  }
  return 0;
}
