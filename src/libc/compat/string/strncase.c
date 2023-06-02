#include <libc/stubs.h>
#include <string.h>
#include <ctype.h>

int
strncasecmp(const char *s1, const char *s2, size_t n)
{
  return strnicmp(s1, s2, n);
}
