#include <stdio.h>
#include <string.h>

#define STRING  "this is a test string"



int main(void)
{
  char result[sizeof(STRING)];
  char *buffer;
  size_t old_len, len = sizeof(STRING);
  int failed = 0;


  buffer = asnprintf(result, &len, "%s", STRING);
  if (buffer != result)
    failed++;

  old_len = len;
  len = 123;
  buffer = asnprintf(NULL, &len, "%s", STRING);
  if (!buffer)
    failed++;
  if (strcmp(buffer, STRING))
    failed++;
  if (old_len != len)
    failed++;

  printf("asnprintf/vasnprintf test %s.\n", failed ? "failed" : "succeded");
  return failed;
}
