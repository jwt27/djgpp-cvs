#include <libc/stubs.h>
#include <string.h>
#include <ctype.h>

int
strcasecmp(const char *s1, const char *s2)
{
  return stricmp(s1, s2);
}
