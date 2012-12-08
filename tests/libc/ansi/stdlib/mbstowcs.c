#include <stdlib.h>

int main(void)
{
#if 0
  size_t len;
#endif

  char s[] = "A";

/* Pacify compiler: -Werror=unused-but-set-variable  */
#if 0
  len = mbstowcs(NULL, s, 0);
#else
  mbstowcs(NULL, s, 0);
#endif

  return 0;
}
