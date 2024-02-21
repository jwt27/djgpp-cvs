#include <stdio.h>
#include <string.h>

int main(void)
{
  char buf[] = "Hello there, stranger";
  char *tok;
  for (tok = strtok(buf, " ,");
       tok;
       tok=strtok(0, " ,"))
    printf("tok = `%s'\n", tok);

  return 0;
}
