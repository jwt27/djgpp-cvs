#include <string.h>
#include <libc/unconst.h>

char *
index(const char *s, int c)
{
  return strchr(s, c);
}
