#include <crt0.h>

#ifdef NULLOK
int _crt0_startup_flags = _CRT0_FLAG_NULLOK;
#endif

int main()
{
  char *null;
  null = (char *)0;
  *null = 0;
  return 0;
}
