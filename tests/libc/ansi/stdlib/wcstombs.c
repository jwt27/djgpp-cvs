#include <stdlib.h>

int main(void)
{
#if 0
  size_t len;
#endif

  wchar_t ws[] = L"A";

/* Pacify compiler: -Werror=unused-but-set-variable  */
#if 0
  len = wcstombs(NULL, ws, 0);
#else
  wcstombs(NULL, ws, 0);
#endif

  return 0;
}
