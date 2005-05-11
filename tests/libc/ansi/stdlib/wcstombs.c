#include <stdlib.h>

int main(void)
{
  size_t len;

  wchar_t ws[] = L"A";

  len = wcstombs(NULL, ws, 0);

  return 0;
}
