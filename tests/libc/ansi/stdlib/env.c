#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>

extern char **environ;

int
main(void)
{
  char line[1000];
  char cmd[80], val[1000];
  while (1)
  {
    int count = 1, i;
    fprintf(stderr, "> ");
    strcpy(line, "?");
    if (gets(line) == 0)
      break;
    strcpy(cmd, "");
    strcpy(val, "");
    sscanf(line, "%s %[^\n]", cmd, val);

    if (isdigit(cmd[0]))
    {
      count = atoi(cmd);
      sscanf(val, "%s %[^\n]", cmd, val);
    }

    if (strcmp(cmd, "e") == 0)
      for (i=0; environ[i]; i++)
	puts(environ[i]);
    else if (strcmp(cmd, "g") == 0)
      printf("%s = `%s'\n", val, getenv(val));
    else if (strcmp(cmd, "p") == 0)
      for (i=0; i<count; i++)
	putenv(val);
    else if (strcmp(cmd, "q") == 0)
      break;
    else if (strcmp(cmd, "m") == 0)
      printf("memory break = 0x%p\n", sbrk(0));
    else
      fprintf(stderr, "Valid commands: g e p q m, not `%s' `%s'\n", cmd, val);
  }
  return 0;
}
