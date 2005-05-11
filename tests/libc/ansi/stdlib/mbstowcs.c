#include <stdlib.h>

int main(void)
{
  size_t len;

  char s[] = "A";

  len = mbstowcs(NULL, s, 0);

  return 0;
}
