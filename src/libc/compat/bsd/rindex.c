#include <string.h>
#include <libc/unconst.h>

char *
rindex(const char *s, int c)
{
  return strrchr(s, c);
}
