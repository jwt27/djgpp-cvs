#include <stdio.h>
#include <string.h>

int
check(int thing, int number)
{
  if (!thing)
  {
    printf("stpncpy flunked test %d\n", number);
    return 1;
  }
  return 0;
}

int
main(void)
{
  int errors = 0;
  char d[] = "xxxxxxxxxxxxxxxxxxxx";
  errors += check(stpncpy(d, "abc", 2) == d + 2, 1);
  errors += check(stpncpy(d, "abc", 3) == d + 3, 2);
  errors += check(stpncpy(d, "abc", 4) == d + 3, 3);
  errors += check(d[3] == '\0' && d[4] == 'x', 4);
  errors += check(stpncpy(d, "abcd", 5) == d + 4, 5);
  errors += check(d[4] == '\0' && d[5] == 'x', 6);
  errors += check(stpncpy(d, "abcd", 6) == d + 4, 7);
  errors += check(d[4] == '\0' && d[5] == '\0' && d[6] == 'x', 8);
  if (errors)
    printf("%d errors.\n", errors);
  else
    printf("No errors.\n");
  return 0;
}
